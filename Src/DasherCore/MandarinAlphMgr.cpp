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
#include "EventHandler.h"
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

CMandarinAlphMgr::CMandarinAlphMgr(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, const CAlphabetMap *pAlphMap)
  : CAlphabetManager(pInterface, pNCManager, pAlphabet, pAlphMap),
    m_pCHAlphabet(pInterface->GetInfo("Chinese 简体中文 (simplified chinese, in pin yin groups)")),
    m_pCHAlphabetMap(m_pCHAlphabet->MakeMap()),
    m_pConversionsBySymbol(new set<symbol>[GetAlphabet()->GetNumberTextSymbols()+1]) {
  //the CHAlphabet contains a group for each SPY syllable+tone, with symbols being chinese characters.
  // Build a map from SPY to set of chinese chars (note, the same chinese unicode can occur in multiple places;
  // hence, we represent as unicode not CHAlph symbol number)...
  map<string,set<string> > conversions;
  //Non-recursive traversal of all the groups in the CHAlphabet (we don't care where they are, just to find them)
  vector<const SGroupInfo *> groups;
  groups.push_back(m_pCHAlphabet->m_pBaseGroup);
  while (!groups.empty()) {
    const SGroupInfo *pGroup(groups.back()); groups.pop_back();
    if (pGroup->pNext) groups.push_back(pGroup->pNext);
    if (pGroup->pChild) groups.push_back(pGroup->pChild);
    //process this group. The SPY syll+tone is stored as the label, using a tone mark over the vowel, e.g. &#257; = a1
    // such equivalences are recorded in the xml 'name' attribute of the group, but we don't need that.
    if (pGroup->strLabel.length()) {
      set<string> &chars(conversions[pGroup->strLabel]);
      DASHER_ASSERT(chars.empty()); //no previous group with same label
      for (int ch=pGroup->iStart; ch<pGroup->iEnd; ch++)
        chars.insert(m_pCHAlphabet->GetText(ch));
    }
  }
  //Dasher's alphabet format means that space and paragraph can't be put into groups,
  // so the above will skip them. Hence, add them using the _symbol_ display text:
  if (symbol sp = m_pCHAlphabet->GetSpaceSymbol())
    conversions[m_pCHAlphabet->GetDisplayText(sp)].insert(m_pCHAlphabet->GetText(sp));
  if (symbol para = m_pCHAlphabet->GetParagraphSymbol())
    conversions[m_pCHAlphabet->GetDisplayText(para)].insert(m_pCHAlphabet->GetText(para));

  //Now: symbols in the primary (SPY) alphabet are syllable+tone, with the string SPY description
  // (using unicode tone marks, e.g. &#257;) in the display text, matching up with the CHAlphabet groups. 
  // (The SPY symbols are arranged in hierarchical groups according to the numbered-tone version, e.g. "a1";
  // but we don't do anything special with those groups, they are just displayed on screen as any normal alphabet).
  //Punctuation is the same way, i.e. PYAlph symbol w/ displaytext "," maps to the CHAlphabel group w/ label ","
  std::vector<symbol> vSyms;
  for (symbol i=1; i<=GetAlphabet()->GetNumberTextSymbols(); i++) {
    set<string> &convs(conversions[m_pAlphabet->GetDisplayText(i)]);
    DASHER_ASSERT(!convs.empty());
    //convert each of these chinese unicode characters into a CHAlphabet symbol...
    for (set<string>::const_iterator it=convs.begin(); it!=convs.end(); it++) {
      vSyms.clear();
      m_pCHAlphabetMap->GetSymbols(vSyms, *it);
      DASHER_ASSERT(vSyms.size()==1 && vSyms[0]!=0); //i.e. conversion is exactly one chinese symbol
      DASHER_ASSERT(m_pCHAlphabet->GetText(vSyms[0]) == *it);
      m_pConversionsBySymbol[i].insert(vSyms[0]);
      //Also the reverse lookup: (valid/used chinese symbol number) -> (pinyin by which it could be produced)
      m_PinyinByChinese[vSyms[0]].insert(i);
    }
  }
  //that leaves m_pConversionsBySymbol as desired.
}

CMandarinAlphMgr::~CMandarinAlphMgr() {
  delete[] m_pConversionsBySymbol;
}

void CMandarinAlphMgr::CreateLanguageModel(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore) {
  //std::cout<<"CHALphabet size "<< pCHAlphabet->GetNumberTextSymbols(); [7603]
  std::cout<<"Setting PPMPY model"<<std::endl;
  m_pLanguageModel = new CPPMPYLanguageModel(pEventHandler, pSettingsStore, m_pCHAlphabet->GetNumberTextSymbols(), m_pAlphabet->GetNumberTextSymbols());
  //our superclass destructor will call ReleaseContext on the iLearnContext when we are destroyed,
  // so we need to put _something_ in there (even tho we don't use it atm!)...
  m_iLearnContext = m_pLanguageModel->CreateEmptyContext();
}

CTrainer *CMandarinAlphMgr::GetTrainer() {
  return new CMandarinTrainer(m_pLanguageModel, m_pAlphabetMap, m_pCHAlphabetMap);
}

CAlphabetManager::CAlphNode *CMandarinAlphMgr::GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, bool bEnteredLast, int iOffset) {

  int iNewOffset(max(-1,iOffset-1));  
  // Use chinese alphabet, not pinyin...
  pair<symbol, CLanguageModel::Context> p=GetContextSymbols(pParent, iNewOffset, m_pCHAlphabetMap);

  CAlphNode *pNewNode;
  if (p.first==0 || !bEnteredLast) {
    pNewNode = new CGroupNode(pParent, iNewOffset, iLower, iUpper, "", 0, this, NULL);
  } else {
    DASHER_ASSERT(p.first>0 && p.first<=m_pCHAlphabet->GetNumberTextSymbols());
    pNewNode = new CMandSym(pParent, iNewOffset, iLower, iUpper,  "", this, p.first, 0);
  }
  pNewNode->iContext = p.second;
  
  return pNewNode;
}

int CMandarinAlphMgr::GetCHColour(symbol CHsym, int iOffset) const {
  int iColour = m_pCHAlphabet->GetColour(CHsym);
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

CDasherNode *CMandarinAlphMgr::CreateSymbolNode(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol) {
  
  //For every PY symbol (=syllable+tone, or "punctuation"),
  // m_pConversionsBySymbol identifies the possible chinese-alphabet symbols
  // that have that syll+tone (for punctuation, this'll be a singleton: the identical
  // punctuation character in the chinese alphabet). A CConvRoot thus offers a choice between them...
  
  if (m_pConversionsBySymbol[iSymbol].size()>1)
    return CreateConvRoot(pParent, iLbnd, iHbnd, strGroup, iSymbol);
  
  return CreateCHSymbol(pParent,pParent->iContext, iLbnd, iHbnd, strGroup, *(m_pConversionsBySymbol[iSymbol].begin()), iSymbol);
}

CMandarinAlphMgr::CConvRoot *CMandarinAlphMgr::CreateConvRoot(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, symbol iPYsym) {
  
  // the same offset as we've still not entered/selected a symbol (leaf);
  // Colour is always 9 so ignore iBkgCol
  CConvRoot *pConv = new CConvRoot(pParent, pParent->offset(), iLbnd, iHbnd, strGroup, this, iPYsym);
    
  // and use the same context too (pinyin syll+tone is _not_ used as part of the LM context)
  pConv->iContext = m_pLanguageModel->CloneContext(pParent->iContext);
  return pConv;
}

CMandarinAlphMgr::CConvRoot::CConvRoot(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, CMandarinAlphMgr *pMgr, symbol pySym)
: CAlphBase(pParent, iOffset, iLbnd, iHbnd, 9, strGroup, pMgr), m_pySym(pySym) {
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
      ? existing->RebuildCHSymbol(this, iLbnd, iHbnd, it->first)
      : mgr()->CreateCHSymbol(this, this->iContext, iLbnd, iHbnd, "", it->first, m_pySym);
    
    DASHER_ASSERT(GetChildren().back()==pNewNode);
  }
}

CMandarinAlphMgr::CMandSym *CMandarinAlphMgr::CreateCHSymbol(CDasherNode *pParent, CLanguageModel::Context iContext, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, symbol iCHsym, symbol iPYparent) {
  // TODO: Parameters here are placeholders - need to figure out
  // what's right 

  int iNewOffset = pParent->offset()+1;
  if (m_pCHAlphabet->GetText(iCHsym) == "\r\n") iNewOffset++;
  CMandSym *pNewNode = new CMandSym(pParent, iNewOffset, iLbnd, iHbnd, strGroup, this, iCHsym, iPYparent);
  pNewNode->iContext = m_pLanguageModel->CloneContext(iContext);
  m_pLanguageModel->EnterSymbol(pNewNode->iContext, iCHsym);
  return pNewNode;
}

CDasherNode *CMandarinAlphMgr::CConvRoot::RebuildSymbol(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSym) {
  if (iSym == m_pySym) {
    SetParent(pParent);
    SetRange(iLbnd,iHbnd);
    return this;
  }
  return CAlphBase::RebuildSymbol(pParent, iLbnd, iHbnd, strGroup, iBkgCol, iSym);
}

bool CMandarinAlphMgr::CConvRoot::isInGroup(const SGroupInfo *pGroup) {
  return pGroup->iStart <= m_pySym && pGroup->iEnd > m_pySym;
}

void CMandarinAlphMgr::CConvRoot::SetFlag(int iFlag, bool bValue) {
  if (iFlag==NF_COMMITTED && bValue && !GetFlag(NF_COMMITTED)
      && !GetFlag(NF_GAME) && mgr()->m_pNCManager->GetBoolParameter(BP_LM_ADAPTIVE)) {
    static_cast<CPPMPYLanguageModel *>(mgr()->m_pLanguageModel)->LearnPYSymbol(mgr()->m_iLearnContext, m_pySym);
  }
  CDasherNode::SetFlag(iFlag,bValue);
}

void CMandarinAlphMgr::GetConversions(std::vector<pair<symbol,unsigned int> > &vChildren, symbol pySym, Dasher::CLanguageModel::Context context) {

  const set<symbol> &convs(m_pConversionsBySymbol[pySym]);
  for(set<symbol>::const_iterator it = convs.begin(); it != convs.end(); ++it) {
    vChildren.push_back(std::pair<symbol, unsigned int>(*it,0));
  }
  //ACL I think it's a good idea to keep those in a consistent order - symbol order will do nicely
  sort(vChildren.begin(),vChildren.end());

  const uint64 iNorm(m_pNCManager->GetLongParameter(LP_NORMALIZATION));
  const unsigned int uniform((m_pNCManager->GetLongParameter(LP_UNIFORM)*iNorm)/1000);
    
  //ACL pass in iNorm and uniform directly - GetPartProbs distributes the last param between
  // however elements there are in vChildren...
  static_cast<CPPMPYLanguageModel *>(m_pLanguageModel)->GetPartProbs(context, vChildren, iNorm, uniform);
  
  //std::cout<<"after get probs "<<std::endl;
  
  uint64 sumProb=0;  
  for (std::vector<pair<symbol,unsigned int> >::const_iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    sumProb += it->second;
  }
  DASHER_ASSERT(sumProb==iNorm);
  //  std::cout<<"Sum Prob "<<sumProb<<std::endl;
  //  std::cout<<"norm "<<nonuniform_norm<<std::endl;
  
  //Match, sumProbs = nonuniform_norm  
  //but fix one element 'Da4'
  // Finally, iterate through the nodes and actually assign the sizes.
  
 // std::cout<<"sumProb "<<sumProb<<std::endl;
  
  int iRemaining(iNorm);
  for (std::vector<pair<symbol,unsigned int> >::iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    DASHER_ASSERT(it->first>-1); //ACL Will's code tested for both these conditions explicitly, and if so 
    DASHER_ASSERT(sumProb>0);   //then used a probability of 0. I don't think either
                                //should ever happen if the alphabet files are right (there'd have to
                                //be either no conversions of the syllable+tone, or else the LM'd have
                                //to assign zero probability to each), so I'm removing these tests for now...
    iRemaining -= it->second = (it->second*iNorm)/sumProb;
    if (it->second==0) {
#ifdef DEBUG
      std::cout << "WARNING: Erasing zero-probability conversion with symbol " << it->first << std::endl;
#endif
      vChildren.erase(it--);
    }
    //  std::cout<<pNode->pszConversion<<std::endl;
    // std::cout<<pNode->Symbol<<std::endl;
    //    std::cout<<"Probs i "<<pNode<<std::endl;
    // std::cout<<"Symbol i"<<SymbolStore[iIdx]<<std::endl;
    // std::cout<<"symbols size "<<SymbolStore.size()<<std::endl;
    // std::cout<<"Symbols address "<<&SymbolStore<<std::endl;
  }
  DASHER_ASSERT(iRemaining==0);
  
  //std::cout<<"iRemaining "<<iRemaining<<std::endl;
  
  
  // Last of all, allocate anything left over due to rounding error
  
  int iLeft(vChildren.size());
  
  for (std::vector<pair<symbol,unsigned int> >::iterator it = vChildren.begin(); it!=vChildren.end(); it++) {
    int iDiff(iRemaining / iLeft);
    
    it->second += iDiff;
    
    iRemaining -= iDiff;
    --iLeft;
    
    //    std::cout<<"Node size for "<<pNode->pszConversion<<std::endl;
    //std::cout<<"is "<<pNode->NodeSize<<std::endl;
  }
  
  DASHER_ASSERT(iRemaining == 0);
  
}

CMandarinAlphMgr::CMandSym::CMandSym(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, CMandarinAlphMgr *pMgr, symbol iSymbol, symbol pyParent)
: CSymbolNode(pParent, iOffset, iLbnd, iHbnd, pMgr->GetCHColour(iSymbol,iOffset), strGroup+pMgr->m_pCHAlphabet->GetDisplayText(iSymbol), pMgr, iSymbol), m_pyParent(pyParent) {
}

CDasherNode *CMandarinAlphMgr::CMandSym::RebuildSymbol(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const std::string &strGroup, int iBkgCol, symbol iSymbol) {
  DASHER_ASSERT(m_pyParent!=0); //should have been computed in RebuildForwardsFromAncestor()
  if (iSymbol==m_pyParent) {
    //create the PY node that lead to this chinese
    if (mgr()->m_pConversionsBySymbol[m_pyParent].size()==1) {
      DASHER_ASSERT( *(mgr()->m_pConversionsBySymbol[m_pyParent].begin()) == this->iSymbol);
      SetRange(iLbnd, iHbnd);
      SetParent(pParent);
      return this;
    }
    //ok, will be a PY-to-Chinese conversion choice
    CConvRoot *pConv = mgr()->CreateConvRoot(pParent, iLbnd, iHbnd, strGroup, iSymbol);
    pConv->PopulateChildrenWithExisting(this);
    return pConv;
  }
  return CAlphBase::RebuildSymbol(pParent, iLbnd, iHbnd, strGroup, iBkgCol, iSymbol);
}

bool CMandarinAlphMgr::CMandSym::isInGroup(const SGroupInfo *pGroup) {
  DASHER_ASSERT(m_pyParent!=0); //should have been computed in RebuildForwardsFromAncestor()
  //pinyin group contains the pinyin-"symbol"=CConvRoot which we want to be our parent...
  return pGroup->iStart <= m_pyParent && pGroup->iEnd > m_pyParent;
}

CMandarinAlphMgr::CMandSym *CMandarinAlphMgr::CMandSym::RebuildCHSymbol(CConvRoot *pParent, unsigned int iLbnd, unsigned int iHbnd, symbol iNewSym) {
  if (iNewSym == this->iSymbol) {
    //reuse existing node
    SetParent(pParent);
    SetRange(iLbnd, iHbnd);
    return this;
  }
  return mgr()->CreateCHSymbol(pParent, pParent->iContext, iLbnd, iHbnd, "", iNewSym, pParent->m_pySym);
}

void CMandarinAlphMgr::CMandSym::RebuildForwardsFromAncestor(CAlphNode *pNewNode) {
  if (m_pyParent==0) {
    set<symbol> &possiblePinyin(mgr()->m_PinyinByChinese[iSymbol]);
    if (possiblePinyin.size() > 1) {
      //need to compare pinyin symbols; so compute probability of this (chinese) sym, for each:
      // i.e. P(pinyin) * P(this chinese | pinyin)
      const vector<unsigned int> &vPinyinProbs(*(pNewNode->GetProbInfo()));
      long bestProb=0; //of this chinese, over LP_NORMALIZATION _squared_
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
  //use chinese, not pinyin, alphabet...
  return mgr()->m_pCHAlphabet->GetText(iSymbol);
}
