// MandarinAlphMgr.cpp
//
// Copyright (c) 2009 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "../Common/Common.h"

#include "MandarinAlphMgr.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include "DasherInterfaceBase.h"
#include "DasherNode.h"
#include "Event.h"
#include "Observable.h"
#include "NodeCreationManager.h"


#include <vector>
#include <sstream>
#include <iostream>

using namespace std;
using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CMandarinAlphMgr::CMandarinAlphMgr(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, const CAlphIO *pAlphIO)
  : CAlphabetManager(pCreator, pInterface, pNCManager, pAlphabet),
    m_pConversionsBySymbol(new vector<symbol>[GetAlphabet()->GetNumberTextSymbols()+1]) {
  DASHER_ASSERT(pAlphabet->m_iConversionID==2);
      
  //the CHAlphabet contains a group for each SPY syllable+tone, with symbols being chinese characters.      
  const CAlphInfo *pCHAlphabet = pAlphIO->GetInfo(pAlphabet->m_strConversionTarget);
      
  //Build a map from SPY group label, to set of chinese chars (represented as start & end of group in pCHAlphabet)
  map<string,pair<symbol,symbol> > conversions;
  //Dasher's alphabet format means that space and paragraph can't be put into groups,
  // so put them into their own group, manually, keyed by _symbol_ display text:
  if (symbol sp = pCHAlphabet->GetSpaceSymbol())
    conversions[pCHAlphabet->GetDisplayText(sp)]=pair<symbol,symbol>(sp,sp+1);
  if (symbol para = pCHAlphabet->GetParagraphSymbol())
    conversions[pCHAlphabet->GetDisplayText(para)]=pair<symbol,symbol>(para,para+1);
  //Non-recursive traversal of all the groups in the CHAlphabet (we don't care where they are, just to find them)
  vector<const ::SGroupInfo *> groups;
  groups.push_back(pCHAlphabet->m_pBaseGroup);
  while (!groups.empty()) {
    const ::SGroupInfo *pGroup(groups.back()); groups.pop_back();
    if (pGroup->pNext) groups.push_back(pGroup->pNext);
    if (pGroup->pChild) groups.push_back(pGroup->pChild);
    //process this group. The SPY syll+tone is stored as the label, using a tone mark over the vowel, e.g. &#257; = a1
    // such equivalences are recorded in the xml 'name' attribute of the group, but we don't need that.
    if (pGroup->strLabel.length()) {
      DASHER_ASSERT(conversions.find(pGroup->strLabel)==conversions.end()); //no previous group with same label
      conversions[pGroup->strLabel] = pair<symbol,symbol>(pGroup->iStart, pGroup->iEnd);
    }
  }

  //Now: symbols in the primary (SPY) alphabet are syllable+tone, with the string SPY description
  // (using unicode tone marks, e.g. &#257;) in the display text, matching up with the CHAlphabet groups. 
  // (The SPY symbols are arranged in hierarchical groups according to the numbered-tone version, e.g. "a1";
  // but we don't do anything special with those groups, they are just displayed on screen as any normal alphabet).
  //Punctuation is the same way, i.e. PYAlph symbol w/ displaytext "," maps to the CHAlphabel group w/ label ","

  //When we find a group in pCHAlphabet is needed, we add its symbols to m_CH{text,displayText,AlphabetMap}
  // _only_ if the same unicode character is not already present; thus m_CHtext etc. will be a 1-1 mapping
  // between indices and actual chinese unicode characters.
  m_CHtext.push_back(""); m_CHdisplayText.push_back(""); m_CHcolours.push_back(0); //as usual, element 0 is the "unknown symbol"
  std::vector<symbol> vSyms;
  for (symbol i=1; i<=GetAlphabet()->GetNumberTextSymbols(); i++) {
    DASHER_ASSERT(conversions.find(m_pAlphabet->GetDisplayText(i))!=conversions.end());
    pair<symbol,symbol> convs(conversions[m_pAlphabet->GetDisplayText(i)]);
    //for each chinese unicode character in the group, hash it to ensure same unicode = same index into m_CH{text,displayText,AlphabetMap}
    for (symbol CHsym=convs.first; CHsym<convs.second; CHsym++) {
      const string &text(pCHAlphabet->GetText(CHsym));
      int target=m_CHAlphabetMap.Get(text);
      if (!target) {
        //unicode char not seen already, allocate new symbol number
        target = m_CHtext.size();
        m_CHtext.push_back(text);
        m_CHdisplayText.push_back(pCHAlphabet->GetDisplayText(CHsym));
        m_CHcolours.push_back(pCHAlphabet->GetColour(CHsym));
        m_CHAlphabetMap.Add(text,target);
      }
      DASHER_ASSERT(m_CHtext[m_CHAlphabetMap.Get(text)] == text);
      m_pConversionsBySymbol[i].push_back(target);
      //Also the reverse lookup: (rehashed chinese symbol number) -> (pinyin by which it could be produced)
      m_PinyinByChinese[target].insert(i);
    }
  }
  //that leaves m_pConversionsBySymbol as desired.
}

void CMandarinAlphMgr::MakeLabels(CDasherScreen *pScreen) {
  CAlphabetManager::MakeLabels(pScreen);
  //a bit of a waste, that fills m_vLabels with labels for all the pinyin symbols - which we don't use.
  for (vector<CDasherScreen::Label *>::iterator it=m_vLabels.begin(); it!=m_vLabels.end(); it++)
    delete *it;
  m_vLabels.clear();
  //instead, keep the screen to create labels lazily...
  m_pScreen = pScreen;
}

CMandarinAlphMgr::~CMandarinAlphMgr() {
  delete[] m_pConversionsBySymbol;
}

void CMandarinAlphMgr::CreateLanguageModel() {
  //std::cout<<"CHALphabet size "<< pCHAlphabet->GetNumberTextSymbols(); [7603]
  //std::cout<<"Setting PPMPY model"<<std::endl;
  m_pLanguageModel = new CPPMPYLanguageModel(this, m_CHtext.size()-1, m_pAlphabet->GetNumberTextSymbols());
}

CTrainer *CMandarinAlphMgr::GetTrainer() {
  //We pass in the pinyin alphabet to define the context-switch escape character, and the default context.
  // Although the default context will be symbolified via the _chinese_ alphabet, this seems reasonable
  // as it is the Pinyin alphabet which defines the conversion mapping (i.e. m_strConversionTarget!)
  return new CMandarinTrainer(m_pInterface, static_cast<CPPMPYLanguageModel*>(m_pLanguageModel), m_pAlphabet, m_pAlphabetMap, &m_CHAlphabetMap, m_pAlphabet->m_strConversionTrainingDelimiter);
}

CAlphabetManager::CAlphNode *CMandarinAlphMgr::GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, bool bEnteredLast, int iOffset) {

  int iNewOffset(max(-1,iOffset-1));  
  // Use chinese alphabet, not pinyin...
  pair<symbol, CLanguageModel::Context> p=GetContextSymbols(pParent, iNewOffset, &m_CHAlphabetMap);

  CAlphNode *pNewNode;
  if (p.first==0 || !bEnteredLast) {
    pNewNode = new CGroupNode(iNewOffset, NULL, 0, this, NULL);
  } else {
    DASHER_ASSERT(p.first>0 && p.first<m_CHtext.size());
    pNewNode = new CMandSym(iNewOffset, this, p.first, 0);
  }
  pNewNode->Reparent(pParent, iLower, iUpper);
  pNewNode->iContext = p.second;
  
  return pNewNode;
}

int CMandarinAlphMgr::GetCHColour(symbol CHsym, int iOffset) const {
  int iColour = m_CHcolours[CHsym];
  if (iColour==-1) {
    //none specified in alphabet
    static int colourStore[2][3] = {
      {66,//light blue
        64,//very light green
        62},//light yellow
      {78,//light purple
        81,//brownish
        60},//red
    };    
    return colourStore[iOffset&1][CHsym % 3];
  }
  if ((iOffset&1)==0 && iColour<130) iColour+=130;
  return iColour;
}

CDasherNode *CMandarinAlphMgr::CreateSymbolNode(CAlphNode *pParent, symbol iSymbol) {
  
  //For every PY symbol (=syllable+tone, or "punctuation"),
  // m_pConversionsBySymbol identifies the possible chinese-alphabet symbols
  // that have that syll+tone (for punctuation, this'll be a singleton: the identical
  // punctuation character in the chinese alphabet). A CConvRoot thus offers a choice between them...
  
  if (m_pConversionsBySymbol[iSymbol].size()>1)
    return CreateConvRoot(pParent, iSymbol);
  
  return CreateCHSymbol(pParent,pParent->iContext, *(m_pConversionsBySymbol[iSymbol].begin()), iSymbol);
}

CMandarinAlphMgr::CConvRoot *CMandarinAlphMgr::CreateConvRoot(CAlphNode *pParent, symbol iPYsym) {
  
  // the same offset as we've still not entered/selected a symbol (leaf);
  // Colour is always 9 so ignore iBkgCol
  CConvRoot *pConv = new CConvRoot(pParent->offset(), this, iPYsym);
    
  // and use the same context too (pinyin syll+tone is _not_ used as part of the LM context)
  pConv->iContext = m_pLanguageModel->CloneContext(pParent->iContext);
  return pConv;
}

CMandarinAlphMgr::CConvRoot::CConvRoot(int iOffset, CMandarinAlphMgr *pMgr, symbol pySym)
: CAlphBase(iOffset, 9, NULL, pMgr), m_pySym(pySym) {
  DASHER_ASSERT(pMgr->m_pConversionsBySymbol[pySym].size()>1);
  //colour + label from ConversionManager.
}

int CMandarinAlphMgr::CConvRoot::ExpectedNumChildren() {
  return mgr()->m_pConversionsBySymbol[m_pySym].size();
}

void CMandarinAlphMgr::CConvRoot::PopulateChildren() {
  PopulateChildrenWithExisting(NULL);
}

void CMandarinAlphMgr::CConvRoot::PopulateChildrenWithExisting(CMandSym *existing) {
  if (m_vChInfo.empty()) {
    mgr()->GetConversions(m_vChInfo,m_pySym, iContext);
  }
  
  int iCum(0);
  
  // Finally loop through and create the children
  for (vector<pair<symbol, unsigned int> >::const_iterator it = m_vChInfo.begin(); it!=m_vChInfo.end(); it++) {
    //      std::cout << "Current scec: " << pCurrentSCEChild << std::endl;
    const unsigned int iLbnd(iCum), iHbnd(iCum + it->second);
    
    iCum = iHbnd;
    CMandSym *pNewNode = (existing)
      ? existing->RebuildCHSymbol(this, it->first)
      : mgr()->CreateCHSymbol(this, this->iContext, it->first, m_pySym);
    pNewNode->Reparent(this, iLbnd, iHbnd);
  }
}

CMandarinAlphMgr::CMandSym *CMandarinAlphMgr::CreateCHSymbol(CDasherNode *pParent, CLanguageModel::Context iContext, symbol iCHsym, symbol iPYparent) {
  // TODO: Parameters here are placeholders - need to figure out
  // what's right 

  int iNewOffset = pParent->offset()+1;
  if (m_CHtext[iCHsym] == "\r\n") iNewOffset++;
  CMandSym *pNewNode = new CMandSym(iNewOffset, this, iCHsym, iPYparent);
  pNewNode->iContext = m_pLanguageModel->CloneContext(iContext);
  m_pLanguageModel->EnterSymbol(pNewNode->iContext, iCHsym);
  return pNewNode;
}

CDasherNode *CMandarinAlphMgr::CConvRoot::RebuildSymbol(CAlphNode *pParent, symbol iSym) {
  if (iSym == m_pySym) return this;
  return CAlphBase::RebuildSymbol(pParent, iSym);
}

bool CMandarinAlphMgr::CConvRoot::isInGroup(const SGroupInfo *pGroup) {
  return pGroup->iStart <= m_pySym && pGroup->iEnd > m_pySym;
}

void CMandarinAlphMgr::CConvRoot::SetFlag(int iFlag, bool bValue) {
  if (iFlag==NF_COMMITTED && bValue && !GetFlag(NF_COMMITTED)
      && !GetFlag(NF_GAME) && mgr()->GetBoolParameter(BP_LM_ADAPTIVE)) {
    //CConvRoot's context is the same as parent's context (no symbol yet!),
    // i.e. is the context in which the pinyin was predicted.
    static_cast<CPPMPYLanguageModel *>(mgr()->m_pLanguageModel)->LearnPYSymbol(iContext, m_pySym);
  }
  CDasherNode::SetFlag(iFlag,bValue);
}

// For sorting pair<symbol, probability>s into descending order
// (most probable first, hence sense of >)
bool CompSecond(const pair<symbol,unsigned int> &a, const pair<symbol,unsigned int> &b) {
  return a.second>b.second;
}

void CMandarinAlphMgr::GetConversions(std::vector<pair<symbol,unsigned int> > &vChildren, symbol pySym, Dasher::CLanguageModel::Context context) {

  const vector<symbol> &convs(m_pConversionsBySymbol[pySym]);

  //Symbols which we are including with the probabilities predicted by the LM.
  // We do this for the most probable symbols first, up to at least BY_PY_PROB_SORT_THRES
  // (a percentage); after this, the remaining probability mass is distributed
  // uniformly between the remaining symbols, which are kept in alphabet order (after the
  // more probable ones).
  //Two degenerate cases: PROB_SORT_THRES=0 => all (legal) ch symbols predicted uniformly
  // PROB_SORT_THRES=100 => all symbols put into probability order
  set<symbol> haveProbs;
  uint64 iRemaining(CDasherModel::NORMALIZATION);
  
  if (long percent=GetLongParameter(LP_PY_PROB_SORT_THRES)) {
    const uint64 iNorm(iRemaining);
    const unsigned int uniform((GetLongParameter(LP_UNIFORM)*iNorm)/1000);
    
    //Set up list of symbols with blank probability entries...
    for(vector<symbol>::const_iterator it = convs.begin(); it != convs.end(); ++it) {
      vChildren.push_back(std::pair<symbol, unsigned int>(*it,0));
    }
    
    //Then call LM to fill in the probs, passing iNorm and uniform directly -
    // GetPartProbs distributes the last param between however elements there are in vChildren...
    static_cast<CPPMPYLanguageModel *>(m_pLanguageModel)->GetPartProbs(context, vChildren, iNorm, uniform);
  
    //std::cout<<"after get probs "<<std::endl;
  
    uint64 sumProb=0;  
    for (std::vector<pair<symbol,unsigned int> >::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
      sumProb += it->second;
    }
    DASHER_ASSERT(sumProb==iNorm);
    
    //Sort all symbols into probability order (highest first)
    stable_sort(vChildren.begin(), vChildren.end(), CompSecond);
    if (percent>=100) return; //and that's what's required.
    
    //ok, some symbols as predicted by LM, others not...
    vector<pair<symbol,unsigned int> > probOrder;
    swap(vChildren, probOrder);
    const unsigned int stop(iNorm - (iNorm*percent)/100);//intermediate values are uint64
    for (vector<pair<symbol, unsigned int> >::iterator it=probOrder.begin(); iRemaining>stop; it++) {
      //assert: the remaining probability mass, divided by the remaining symbols,
      // (i.e. the probability mass each symbol would receive if we uniformed the rest)
      // must be less than the probability predicted by the LM (as we're processing
      // symbols in decreasing order)
      DASHER_ASSERT(iRemaining <= it->second*(convs.size()-vChildren.size()));
      vChildren.push_back(*it);
      haveProbs.insert(it->first);
      iRemaining-=it->second;
    }
  }
  //Now distribute iRemaining uniformly between all remaining symbols,
  // keeping them in alphabet order
  if (iRemaining) {
    unsigned int iEach(iRemaining / (convs.size() - haveProbs.size()));
    for (vector<symbol>::const_iterator it=convs.begin(); it!=convs.end(); it++) {
      if (haveProbs.count(*it)==0)
        vChildren.push_back(pair<symbol,unsigned int>(*it,iEach));
    }
    
    //account for rounding error by topping up
    DASHER_ASSERT(vChildren.size() == convs.size());
    iRemaining -= iEach * (convs.size() - haveProbs.size());
    unsigned int iLeft = vChildren.size();
    for (vector<pair<symbol, unsigned int> >::iterator it=vChildren.end(); iRemaining && it-- != vChildren.begin();) {
      const unsigned int p(iRemaining / iLeft);
      it->second+=p; iRemaining-=p; iLeft--;
    }
  }
}

CDasherScreen::Label *CMandarinAlphMgr::GetLabel(int iCHsym) {
  //TODO: LRU cache, keep down to some sensible #labels allocated?
  if (iCHsym>=m_vLabels.size()) {
    m_vLabels.resize(iCHsym+1);
  } else if (m_vLabels[iCHsym]) {
    return m_vLabels[iCHsym];
  }
  return m_vLabels[iCHsym] = m_pScreen->MakeLabel(m_CHdisplayText[iCHsym]);
}

CMandarinAlphMgr::CMandSym::CMandSym(int iOffset, CMandarinAlphMgr *pMgr, symbol iSymbol, symbol pyParent)
: CSymbolNode(iOffset, pMgr->GetCHColour(iSymbol,iOffset), pMgr->GetLabel(iSymbol), pMgr, iSymbol), m_pyParent(pyParent) {
}

CDasherNode *CMandarinAlphMgr::CMandSym::RebuildSymbol(CAlphNode *pParent, symbol iSymbol) {
  DASHER_ASSERT(m_pyParent!=0); //should have been computed in RebuildForwardsFromAncestor()
  if (iSymbol==m_pyParent) {
    //create the PY node that lead to this chinese
    if (mgr()->m_pConversionsBySymbol[m_pyParent].size()==1) {
      DASHER_ASSERT( *(mgr()->m_pConversionsBySymbol[m_pyParent].begin()) == this->iSymbol);
      return this;
    }
    //ok, will be a PY-to-Chinese conversion choice
    CConvRoot *pConv = mgr()->CreateConvRoot(pParent, iSymbol);
    pConv->PopulateChildrenWithExisting(this);
    return pConv;
  }
  return CAlphBase::RebuildSymbol(pParent, iSymbol);
}

bool CMandarinAlphMgr::CMandSym::isInGroup(const SGroupInfo *pGroup) {
  DASHER_ASSERT(m_pyParent!=0); //should have been computed in RebuildForwardsFromAncestor()
  //pinyin group contains the pinyin-"symbol"=CConvRoot which we want to be our parent...
  return pGroup->iStart <= m_pyParent && pGroup->iEnd > m_pyParent;
}

CMandarinAlphMgr::CMandSym *CMandarinAlphMgr::CMandSym::RebuildCHSymbol(CConvRoot *pParent, symbol iNewSym) {
  if (iNewSym == this->iSymbol) return this; //reuse existing node
  return mgr()->CreateCHSymbol(pParent, pParent->iContext, iNewSym, pParent->m_pySym);
}

void CMandarinAlphMgr::CMandSym::RebuildForwardsFromAncestor(CAlphNode *pNewNode) {
  if (m_pyParent==0) {
    set<symbol> &possiblePinyin(mgr()->m_PinyinByChinese[iSymbol]);
    if (possiblePinyin.size() > 1) {
      //need to compare pinyin symbols; so compute probability of this (chinese) sym, for each:
      // i.e. P(pinyin) * P(this chinese | pinyin)
      const vector<unsigned int> &vPinyinProbs(*(pNewNode->GetProbInfo()));
      long bestProb=0; //of this chinese, over NORMALIZATION _squared_
      for (set<symbol>::iterator p_it = possiblePinyin.begin(); p_it!=possiblePinyin.end(); p_it++) {
        //compute probability of each chinese symbol for that pinyin (=by filtering)
        // context is the same as the ancestor = previous chinese, as pinyin not part of context
        vector<pair<symbol, unsigned int> > vChineseProbs;
        mgr()->GetConversions(vChineseProbs, *p_it, pNewNode->iContext);
        //now find us in that list
        long thisProb; //i.e. P(this pinyin) * P(this chinese | this pinyin)
        for (vector<pair<symbol,unsigned int> >::iterator c_it = vChineseProbs.begin(); ;) {
          if (c_it->first == iSymbol) {
            //found P(this chinese sym | pinyin). Compute overall...
            thisProb = c_it->second * vPinyinProbs[*p_it];
            break;
          }
          c_it++;
          DASHER_ASSERT(c_it!=vChineseProbs.end()); //gotta find this chinese sym somewhere...
        }
        //see if that works out better than for the other possible pinyin...
        if (thisProb > bestProb) {
          bestProb = thisProb;
          m_pyParent = *p_it;
        }
      }
    } else m_pyParent = *(possiblePinyin.begin());
  }
  CSymbolNode::RebuildForwardsFromAncestor(pNewNode);
}

const std::string &CMandarinAlphMgr::CMandSym::outputText() {
  return mgr()->m_CHtext[iSymbol];
}

string CMandarinAlphMgr::CMandSym::trainText() {
  //NF_COMMITTED should be in process of being set...
  DASHER_ASSERT(!GetFlag(NF_COMMITTED));
  //in which case, we should have a parent (if not, we would have to
  // have been built from string context, i.e. going backwards,
  // in which case we would be committed already)
  DASHER_ASSERT(Parent());
  //so the parent should have set our m_pyParent field...
  DASHER_ASSERT(m_pyParent);
  int iPY = m_pyParent;
  if (iPY==0) {
    std::set<symbol> &py(mgr()->m_PinyinByChinese[iSymbol]);
    DASHER_ASSERT(py.size()==1);
    if (py.size()==1) iPY = *(py.begin());
    else return ""; //output nothing! TODO could reset context for what follows - but don't think this should ever happen?
  }
  return mgr()->m_pAlphabet->m_strConversionTrainingDelimiter + mgr()->m_pAlphabet->GetText(iPY) + outputText();
}
