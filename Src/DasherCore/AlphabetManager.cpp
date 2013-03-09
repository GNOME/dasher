// AlphabetManager.cpp
//
// Copyright (c) 2007 The Dasher Team
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

#include "AlphabetManager.h"
#include "ConversionManager.h"
#include "DasherInterfaceBase.h"
#include "DasherNode.h"
#include "Event.h"
#include "Observable.h"
#include "NodeCreationManager.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"
#include "LanguageModelling/DictLanguageModel.h"
#include "LanguageModelling/MixtureLanguageModel.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include "LanguageModelling/CTWLanguageModel.h"
#include "FileWordGenerator.h"

#include <vector>
#include <sstream>
#include <iostream>
#include "string.h"

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

CAlphabetManager::CAlphabetManager(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet)
  : CSettingsUser(pCreateFrom), m_pBaseGroup(NULL), m_pInterface(pInterface), m_pNCManager(pNCManager), m_pAlphabet(pAlphabet), m_pLastOutput(NULL) {
}

const string &CAlphabetManager::GetLabelText(symbol i) const {
  return m_pAlphabet->GetDisplayText(i);
}

void CAlphabetManager::Setup() {
  InitMap();

  for (char c=33; (c&0x80)==0; c++) {
    string s(&c,1);
    if (m_map.Get(s)==0) {
      m_sDelim = s;
      break;
    }
  }
  //else, if all single-octet chars are in alphabet - leave m_sDelim==""
  // (and we'll find a delimiter for each context)

  CreateLanguageModel();
}

void CAlphabetManager::InitMap() {
  int iPara = m_pAlphabet->GetParagraphSymbol();
  if (iPara) m_map.AddParagraphSymbol(iPara);
  int i;
  for(i = 1; i < m_pAlphabet->iEnd; i++) // 1-indexed
    if (i!=iPara) m_map.Add(m_pAlphabet->GetText(i), i);
  
  /*ACL I'm really not sure where conversion characters should/shouldn't be included.
   They seemed to be included in the Alphabet Map, i.e. for reading training text via GetSymbols;
   but a TODO comment suggested they should _not_ be included in GetNumberSymbols(),
   and I couldn't find any code which would have called e.g. GetText on them.
   Moreover, if these characters are put into the AlphabetMap, they'll be fed into the
   LanguageModel just as any other "symbol", but with an out-of-bounds symbol number!
   (So maybe the range of allowed symbol numbers is wrong?). Hence, not including them atm.
   If they were needed, we could do something like the following:
   if (StartConvertCharacter)
   map->Add(StartConvertCharacter->Text, ++i);
   if (EndConvertCharacter)
   map->Add(EndConvertCharacter->Text, ++i);
   */
}

void CAlphabetManager::CreateLanguageModel() {
  // FIXME - return to using enum here
  switch (GetLongParameter(LP_LANGUAGE_MODEL_ID)) {
    default:
      // If there is a bogus value for the language model ID, we'll default
      // to our trusty old PPM language model.
    case 0:
      m_pLanguageModel = new CPPMLanguageModel(this, m_pAlphabet->iEnd-1);
      break;
    case 2:
      m_pLanguageModel = new CWordLanguageModel(this, m_pAlphabet, &m_map);
      break;
    case 3:
      m_pLanguageModel = new CMixtureLanguageModel(this, m_pAlphabet, &m_map);
      break;
    case 4:
      m_pLanguageModel = new CCTWLanguageModel(m_pAlphabet->iEnd-1);
      break;
  }
}

CTrainer *CAlphabetManager::GetTrainer() {
  return new CTrainer(m_pInterface, m_pLanguageModel, m_pAlphabet, &m_map);
}

void CAlphabetManager::MakeLabels(CDasherScreen *pScreen) {
  m_pBaseGroup->RecursiveDelete();
  for (vector<CDasherScreen::Label *>::iterator it=m_vLabels.begin(); it!=m_vLabels.end(); it++)
    delete (*it);
  m_vLabels.clear();
  for (map<const SGroupInfo *,CDasherScreen::Label *>::iterator it=m_mGroupLabels.begin(); it!=m_mGroupLabels.end(); it++)
    delete it->second;
  m_mGroupLabels.clear();
  m_pBaseGroup = copyGroups(m_pAlphabet,pScreen);
}

SGroupInfo *CAlphabetManager::copyGroups(const SGroupInfo *pBase, CDasherScreen *pScreen) {
  if (pBase==NULL) return NULL;
  DASHER_ASSERT(pBase->iNumChildNodes); //zero-element groups elided by CAlphIO
  if (m_vLabels.size()<pBase->iEnd) m_vLabels.resize(pBase->iEnd);
  string strGroupPrefix; int iBkgCol(-1);
  SGroupInfo * const next=copyGroups(pBase->pNext, pScreen);
  while (pBase->iNumChildNodes==1) {
    //were about to create a group node, which would have only one child
    // (eventually, if the group node were PopulateChildren'd).
    // Such a child would entirely fill it's parent (the group), and thus,
    // creation/destruction of the child would cause the node's colour to flash
    // between that for parent group and child.
    // Hence, instead we elide the group node and create the child _here_...
    
    //1. however we also have to take account of the appearance of the elided group. Hence:
    strGroupPrefix += pBase->strLabel;
    if (pBase->bVisible) iBkgCol=pBase->iColour;
    //2. group might contain a single subgroup, or a single symbol...
    if (!pBase->pChild) {
      //single symbol. Create its label, taking account of enclosing groups...
      // (symbols are never transparent)
      DASHER_ASSERT(pBase->iEnd == pBase->iStart+1);
      string symLabel = strGroupPrefix + GetLabelText(pBase->iStart);
      m_vLabels[pBase->iStart]=(symLabel.empty() ? NULL : pScreen->MakeLabel(symLabel));
      //then skip this group, return any siblings
      return next;
    }
    //...a subgroup, so go into it
    pBase = pBase->pChild;
    DASHER_ASSERT(pBase->pNext==NULL); //can't have siblings as parent has only one child
                                       //hence, original 'next' pointer is still valid
    //3. loop round...
  }
  //in or reached nontrivial subgroup - so make node for entire group
  //First, make (unpefixed) labels for all children in (original) group
  // (children of subgroups that are later elided, will have labels made at elision time)
  {
    SGroupInfo *pChild=pBase->pChild;
    for (int i=pBase->iStart; i<pBase->iEnd;)
      if (!pChild || i<pChild->iStart) {
        const string &symLabel(GetLabelText(i));
        m_vLabels[i] = (symLabel.empty() ? NULL : pScreen->MakeLabel(symLabel));
        i++;
      } else {
        i=pChild->iEnd;
        pChild = pChild->pNext;
      }
  }
  SGroupInfo *pRes = new SGroupInfo(*pBase);
  //apply properties of enclosing group(s)...
  pRes->strLabel = strGroupPrefix + pRes->strLabel;
  if (!pRes->bVisible)
    if ((pRes->iColour = iBkgCol)!=-1) pRes->bVisible=true;
  if (pRes->strLabel.length())
    m_mGroupLabels[pRes] = pScreen->MakeLabel(pRes->strLabel);
  //siblings (of this group or elided parent) copied already, from original
  // (passed-in) pBase: if pBase unchanged, then still valid, whereas if pBase
  // was changed by the above loop to be a subgroup of the original, then the subgroup
  // has no children, so should be spliced in place of the original pBase.
  pRes->pNext = next;

  //recurse on children
  pRes->pChild = copyGroups(pRes->pChild, pScreen);
  DASHER_ASSERT(pRes->iNumChildNodes>1);
  return pRes;
}

CWordGeneratorBase *CAlphabetManager::GetGameWords() {
  CFileWordGenerator *pGen = new CFileWordGenerator(m_pInterface, m_pAlphabet, &m_map);
  pGen->setAcceptUser(true);
  if (!GetStringParameter(SP_GAME_TEXT_FILE).empty()) {
    const string &gtf(GetStringParameter(SP_GAME_TEXT_FILE));
    if (pGen->ParseFile(gtf,true)) return pGen;
    ///TRANSLATORS: the string "GameTextFile" is the name of a setting in gsettings
    /// (or equivalent), and should not be translated. The %s is the value of that
    /// setting (this message displayed only if the user has provided a value)
    m_pInterface->FormatMessageWithString(_("Note: GameTextFile setting specifies game sentences file '%s' but this does not exist"),gtf.c_str());
  }
  if (!m_pAlphabet->GetGameModeFile().empty()) {
    //TODO, try user dir first / give one or other priority?
    // This will concatenate all - which doesn't seem too bad...?
    m_pInterface->ScanFiles(pGen, m_pAlphabet->GetGameModeFile());
    if (pGen->HasLines()) return pGen;
  }
  pGen->setAcceptUser(false);
  m_pInterface->ScanFiles(pGen, m_pAlphabet->GetTrainingFile());
  if (pGen->HasLines()) return pGen;
  delete pGen;
  return NULL;
}

const CAlphInfo *CAlphabetManager::GetAlphabet() const {
  return m_pAlphabet;
}

CAlphabetManager::~CAlphabetManager() {
  //the alphabet belongs to the AlphIO, and may be reused later
  delete m_pLanguageModel;
}

void CAlphabetManager::WriteTrainFileFull(CDasherInterfaceBase *pInterface) {
  if (strTrainfileBuffer == "") return;
  if (strTrainfileContext != "") {
    //If context begins with the default, skip that - it'll be entered by Trainer 1st anyway
    string defCtx(m_pAlphabet->GetDefaultContext());
    if (strTrainfileContext.substr(0,defCtx.length()) == defCtx)
      strTrainfileContext = strTrainfileContext.substr(defCtx.length());
    string sDelim(m_sDelim);
    if (sDelim == "") {
      //find a character not in the context we want to write out
      char c=33;
      while (strTrainfileContext.find(c)!=strTrainfileContext.length()) c++; //will terminate, context is ~~5 chars
      sDelim = string(&c,1);
    }
    strTrainfileBuffer = m_pAlphabet->GetContextEscapeChar() + sDelim + strTrainfileContext + sDelim + strTrainfileBuffer;
    strTrainfileContext="";
  }
  pInterface->WriteTrainFile(m_pAlphabet->GetTrainingFile(), strTrainfileBuffer);
  strTrainfileBuffer="";
}

int CAlphabetManager::GetColour(symbol sym, int iOffset) const {
  int iColour = m_pAlphabet->GetColour(sym);

  // This is for backwards compatibility with old alphabet files -
  // ideally make this log a warning (unrelated TODO: automate
  // validation of alphabet files, plus maintenance of repository
  // etc.)
  if(iColour == -1)
      iColour = (sym % 3) + 10;

  // Loop on low colours for nodes (TODO: go back to colour namespaces?)
  // letters 10-109, alternate colours 140-239
  if((iOffset&1) == 0 && iColour < 110)
    iColour += 130;

  return iColour;
}


CAlphabetManager::CAlphBase::CAlphBase(int iOffset, int iColour, CDasherScreen::Label *pLabel, CAlphabetManager *pMgr)
: CDasherNode(iOffset, iColour, pLabel), m_pMgr(pMgr) {
}

void CAlphabetManager::CAlphBase::Output() {
  if (m_pMgr->m_pLastOutput && m_pMgr->m_pLastOutput == Parent())
    m_pMgr->m_pLastOutput=this;
  //Case where lastOutput != Parent to subclasses, if they want to.
  //Note if lastOutput==NULL, we leave it - so the first letter written after startup,
  // will register as a context switch and write out an empty/default context.
}

void CAlphabetManager::CAlphBase::Undo() {
  if (m_pMgr->m_pLastOutput==this) m_pMgr->m_pLastOutput = Parent();
}
CAlphabetManager::CAlphNode::CAlphNode(int iOffset, int iColour, CDasherScreen::Label *pLabel, CAlphabetManager *pMgr)
: CAlphBase(iOffset, iColour, pLabel, pMgr), m_pProbInfo(NULL) {
}

CAlphabetManager::CSymbolNode::CSymbolNode(int iOffset, CDasherScreen::Label *pLabel, CAlphabetManager *pMgr, symbol _iSymbol)
: CAlphNode(iOffset, pMgr->GetColour(_iSymbol, iOffset), pLabel, pMgr), iSymbol(_iSymbol) {
}

CAlphabetManager::CSymbolNode::CSymbolNode(int iOffset, int iColour, CDasherScreen::Label *pLabel, CAlphabetManager *pMgr, symbol _iSymbol)
: CAlphNode(iOffset, iColour, pLabel, pMgr), iSymbol(_iSymbol) {
}

CAlphabetManager::CGroupNode::CGroupNode(int iOffset, CDasherScreen::Label *pLabel, int iBkgCol, CAlphabetManager *pMgr, const SGroupInfo *pGroup)
: CAlphNode(iOffset,
            pGroup==pMgr->m_pBaseGroup ? ((iOffset&1) ? 7 : 137) //special case for root nodes
            : (pGroup->bVisible ? pGroup->iColour : iBkgCol),
            pLabel, pMgr), m_pGroup(pGroup) {
  if (!m_pGroup->bVisible) SetFlag(NF_VISIBLE, false);
}

CAlphabetManager::CAlphNode *CAlphabetManager::GetRoot(CDasherNode *pParent, bool bEnteredLast, int iOffset) {
  //pParent is not a parent, just for document/context.
  int iNewOffset(max(-1,iOffset-1));

  pair<symbol, CLanguageModel::Context> p = GetContextSymbols(pParent, iNewOffset, &m_map);

  CAlphNode *pNewNode;
  if(p.first==0 || !bEnteredLast) {
    //couldn't extract last symbol (so probably using default context), or shouldn't
    pNewNode = new CGroupNode(iNewOffset, NULL, 0, this, m_pBaseGroup); //default background colour
  } else {
    //new node represents a symbol that's already happened - i.e. user has already steered through it;
    // so either we're rebuilding, or else creating a new root from existing text (in edit box)
    DASHER_ASSERT(!pParent);
    pNewNode = CreateSymbolRoot(iNewOffset, p.second, p.first);
    pNewNode->SetFlag(NF_SEEN, true);
    pNewNode->CDasherNode::SetFlag(NF_COMMITTED, true); //do NOT commit!
  }
  pNewNode->iContext = p.second;
  return pNewNode;
}

CAlphabetManager::CAlphNode *CAlphabetManager::CreateSymbolRoot(int iOffset, CLanguageModel::Context ctx, symbol sym) {
  return new CSymbolNode(iOffset, m_vLabels[sym], this, sym);
}

pair<symbol, CLanguageModel::Context> CAlphabetManager::GetContextSymbols(CDasherNode *pParent, int iRootOffset, const CAlphabetMap *pAlphMap) {
  vector<symbol> vContextSymbols; bool bHaveFinalSymbol = true;
  //no context is ever available at offset -1 (=choice between symbols with offset 0)
  if (iRootOffset!=-1) {
    // TODO: make the LM get the context, rather than force it to fix max context length as an int
    int iStart = max(0, iRootOffset - m_pLanguageModel->GetContextLength());
    if(pParent) {
      pParent->GetContext(m_pInterface, pAlphMap, vContextSymbols, iStart, iRootOffset+1 - iStart);
    } else {
      pAlphMap->GetSymbols(vContextSymbols, m_pInterface->GetContext(iStart, iRootOffset+1 - iStart));
    }

    for (std::vector<symbol>::iterator it = vContextSymbols.end(); it!=vContextSymbols.begin();) {
      if (*(--it) == 0) {
        //found an impossible symbol! erase from beginning up to it (inclusive)
        vContextSymbols.erase(vContextSymbols.begin(), ++it);
        break;
      }
    }
  }
  if (vContextSymbols.empty()) {
    bHaveFinalSymbol = false;
    pAlphMap->GetSymbols(vContextSymbols, m_pAlphabet->GetDefaultContext());
  }

  CLanguageModel::Context iContext = m_pLanguageModel->CreateEmptyContext();

  //enter the symbols we could make sense of, into the LM context...
  for (vector<symbol>::iterator it=vContextSymbols.begin(); it != vContextSymbols.end(); it++) {
    m_pLanguageModel->EnterSymbol(iContext, *it);
  }
  return pair<symbol,CLanguageModel::Context>(bHaveFinalSymbol ? vContextSymbols[vContextSymbols.size()-1] : 0, iContext);
}

bool CAlphabetManager::CSymbolNode::GameSearchNode(symbol sym) {
  if (sym == iSymbol) {
    SetFlag(NF_GAME, true);
    return true;
  }
  return false;
}
bool CAlphabetManager::CGroupNode::GameSearchNode(symbol sym) {
  if (sym >= m_pGroup->iStart && sym < m_pGroup->iEnd) {
    if (GetFlag(NF_ALLCHILDREN)) {
      if (!GameSearchChildren(sym)) //recurse, to mark game child also
        DASHER_ASSERT(false); //sym within this group, should definitely be found!
    }
    SetFlag(NF_GAME, true);
    return true;
  }
  DASHER_ASSERT(!GameSearchChildren(sym));
  return false;
}

void CAlphabetManager::CSymbolNode::GetContext(CDasherInterfaceBase *pInterface, const CAlphabetMap *pAlphabetMap, vector<symbol> &vContextSymbols, int iOffset, int iLength) {
  if (!GetFlag(NF_SEEN) && iOffset+iLength-1 == offset()) {
    if (iLength > 1) Parent()->GetContext(pInterface, pAlphabetMap, vContextSymbols, iOffset, iLength-numChars());
    vContextSymbols.push_back(iSymbol);
  } else {
    CDasherNode::GetContext(pInterface, pAlphabetMap, vContextSymbols, iOffset, iLength);
  }
}

symbol CAlphabetManager::CSymbolNode::GetAlphSymbol() {
  return iSymbol;
}

void CAlphabetManager::CSymbolNode::PopulateChildren() {
  m_pMgr->IterateChildGroups(this, m_pMgr->m_pBaseGroup, NULL);
}
int CAlphabetManager::CAlphNode::ExpectedNumChildren() {
  int i=m_pMgr->m_pBaseGroup->iNumChildNodes;
  return (m_pMgr->GetBoolParameter(BP_CONTROL_MODE)) ? i+1 : i;
}

void CAlphabetManager::GetProbs(vector<unsigned int> *pProbInfo, CLanguageModel::Context context) {
  const unsigned int iSymbols = m_pBaseGroup->iEnd-1;
  
  // TODO - sort out size of control node - for the timebeing I'll fix the control node at 5%
  // TODO: New method (see commented code) has been removed as it wasn' working.

  const unsigned long iNorm(m_pNCManager->GetAlphNodeNormalization());
  //the case for control mode on, generalizes to handle control mode off also,
  // as then iNorm - control_space == iNorm...
  const unsigned int iUniformAdd = max(1ul, ((iNorm * GetLongParameter(LP_UNIFORM)) / 1000) / iSymbols);
  const unsigned long iNonUniformNorm = iNorm - iSymbols * iUniformAdd;
  //  m_pLanguageModel->GetProbs(context, Probs, iNorm, ((iNorm * uniform) / 1000));

  //ACL used to test explicitly for MandarinDasher and if so called GetPYProbs instead
  // (by statically casting to PPMPYLanguageModel). However, have renamed PPMPYLanguageModel::GetPYProbs
  // to GetProbs as per ordinary language model, so no need to test....
  m_pLanguageModel->GetProbs(context, *pProbInfo, iNonUniformNorm, 0);

  DASHER_ASSERT(pProbInfo->size() == iSymbols+1);//initial 0

  for(unsigned int k(1); k < pProbInfo->size(); ++k)
    (*pProbInfo)[k] += iUniformAdd;

#ifdef DEBUG
  {
    unsigned long iTotal = 0;
    for(unsigned int k = 0; k < pProbInfo->size(); ++k)
      iTotal += (*pProbInfo)[k];
    DASHER_ASSERT(iTotal == iNorm);
  }
#endif
}

std::vector<unsigned int> *CAlphabetManager::CAlphNode::GetProbInfo() {
  if (!m_pProbInfo) {
    m_pProbInfo = new std::vector<unsigned int>();
    m_pMgr->GetProbs(m_pProbInfo, iContext);

    // work out cumulative probs in place
    for(unsigned int i = 1; i < m_pProbInfo->size(); i++) {
      (*m_pProbInfo)[i] += (*m_pProbInfo)[i - 1];
    }
  }
  return m_pProbInfo;
}

std::vector<unsigned int> *CAlphabetManager::CGroupNode::GetProbInfo() {
  if (Parent() && Parent()->mgr() == mgr() && Parent()->offset()==offset()) {
    return (static_cast<CAlphNode *>(Parent()))->GetProbInfo();
  }
  //nope, no usable parent. compute here...
  return CAlphNode::GetProbInfo();
}

void CAlphabetManager::CGroupNode::PopulateChildren() {
  m_pMgr->IterateChildGroups(this, m_pGroup, NULL);
}

int CAlphabetManager::CGroupNode::ExpectedNumChildren() {
  return m_pGroup->iNumChildNodes;
}

CAlphabetManager::CGroupNode *CAlphabetManager::CreateGroupNode(CAlphNode *pParent, int iBkgCol, const SGroupInfo *pInfo) {

  // When creating a group node...
  // ...the offset is the same as the parent...

  CGroupNode *pNewNode = new CGroupNode(pParent->offset(), m_mGroupLabels[pInfo], iBkgCol, this, pInfo);

  //...as is the context!
  pNewNode->iContext = m_pLanguageModel->CloneContext(pParent->iContext);

  return pNewNode;
}

CDasherNode *CAlphabetManager::CAlphBase::RebuildGroup(CAlphNode *pParent, int iBkgCol, const SGroupInfo *pInfo) {
  CGroupNode *pRet=m_pMgr->CreateGroupNode(pParent, iBkgCol, pInfo);
  if (isInGroup(pInfo)) {
    //created group node should contain this one
    m_pMgr->IterateChildGroups(pRet,pInfo,this);
  }
  return pRet;
}

CDasherNode *CAlphabetManager::CGroupNode::RebuildGroup(CAlphNode *pParent, int iBkgCol, const SGroupInfo *pInfo) {
  if (pInfo == m_pGroup) {
    //offset doesn't increase for groups...
    DASHER_ASSERT (offset() == pParent->offset());
    return this;
  }
  return CAlphBase::RebuildGroup(pParent, iBkgCol, pInfo);
}

bool CAlphabetManager::CGroupNode::isInGroup(const SGroupInfo *pInfo) {
  return pInfo->iStart <= m_pGroup->iStart && pInfo->iEnd >= m_pGroup->iEnd;
}

bool CAlphabetManager::CSymbolNode::isInGroup(const SGroupInfo *pInfo) {
  return (pInfo->iStart <= iSymbol && pInfo->iEnd > iSymbol);
}

CDasherNode *CAlphabetManager::CreateSymbolNode(CAlphNode *pParent, symbol iSymbol) {

    // TODO: Exceptions / error handling in general

    // Uniquely, a paragraph symbol can be two characters
    // (and we can't call numChars() on the symbol before we've constructed it!)
    int iNewOffset = pParent->offset()+1;
    if (m_pAlphabet->GetText(iSymbol)=="\r\n") iNewOffset++;
    CSymbolNode *pAlphNode = new CSymbolNode(iNewOffset, m_vLabels[iSymbol], this, iSymbol);
    //     std::stringstream ssLabel;

    //     ssLabel << GetLabelText(iSymbol) << ": " << pNewNode;

    //    pDisplayInfo->strDisplayText = ssLabel.str();

    pAlphNode->iContext = m_pLanguageModel->CloneContext(pParent->iContext);
    m_pLanguageModel->EnterSymbol(pAlphNode->iContext, iSymbol); // TODO: Don't use symbols?

  return pAlphNode;
}

CDasherNode *CAlphabetManager::CAlphBase::RebuildSymbol(CAlphNode *pParent, symbol iSymbol) {
  return m_pMgr->CreateSymbolNode(pParent, iSymbol);
}

CDasherNode *CAlphabetManager::CSymbolNode::RebuildSymbol(CAlphNode *pParent, symbol iSymbol) {
  if(iSymbol == this->iSymbol) {
    DASHER_ASSERT(offset() == pParent->offset() + numChars());
    return this;
  }
  return CAlphBase::RebuildSymbol(pParent, iSymbol);
}

void CAlphabetManager::IterateChildGroups(CAlphNode *pParent, const SGroupInfo *pParentGroup, CAlphBase *buildAround) {
  std::vector<unsigned int> *pCProb(pParent->GetProbInfo());
  DASHER_ASSERT((*pCProb)[0] == 0);
  const int iMin(pParentGroup->iStart);
  const int iMax(pParentGroup->iEnd);
  unsigned int iRange(pParentGroup == m_pBaseGroup ? CDasherModel::NORMALIZATION : ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]));

  // TODO: Think through alphabet file formats etc. to make this class easier.
  // TODO: Throw a warning if parent node already has children

  // Create child nodes and add them

  int i(iMin); //lowest index of child which we haven't yet added
  const SGroupInfo *pCurrentNode(pParentGroup->pChild);
  // The SGroupInfo structure has something like linked list behaviour
  // Each SGroupInfo contains a pNext, a pointer to a sibling group info
  while (i < iMax) {
    CDasherNode *pNewChild;
    bool bSymbol = !pCurrentNode //gone past last subgroup
                  || i < pCurrentNode->iStart; //not reached next subgroup
    const int iStart=i, iEnd = (bSymbol) ? i+1 : pCurrentNode->iEnd;
    //uint64 is platform-dependently #defined in DasherTypes.h as an (unsigned) 64-bit int ("__int64" or "long long int")
    unsigned int iLbnd = (((*pCProb)[iStart-1] - (*pCProb)[iMin-1]) *
                          static_cast<uint64>(CDasherModel::NORMALIZATION)) /
                         iRange;
    unsigned int iHbnd = (((*pCProb)[iEnd-1] - (*pCProb)[iMin-1]) *
                          static_cast<uint64>(CDasherModel::NORMALIZATION)) /
                         iRange;
    if (bSymbol) {
      pNewChild = (buildAround) ? buildAround->RebuildSymbol(pParent, i) : CreateSymbolNode(pParent, i);
      i++; //make one symbol at a time - move onto next symbol in next iteration of (outer) loop
    } else {
      DASHER_ASSERT(pCurrentNode->iNumChildNodes > 1);
      pNewChild= (buildAround) ? buildAround->RebuildGroup(pParent, pParent->getColour(), pCurrentNode) : CreateGroupNode(pParent, pParent->getColour(), pCurrentNode);
      i = pCurrentNode->iEnd; //make one group at a time - so move past entire group...
      pCurrentNode = pCurrentNode->pNext; //next sibling of _original_ pCurrentNode (above)
      // (maybe not of pCurrentNode now, which might be a subgroup filling the original)
    }
    //created a new node - symbol or (group which will have >1 child).
    pNewChild->Reparent(pParent, iLbnd, iHbnd);
  }

  if (pParentGroup == m_pBaseGroup) m_pNCManager->AddExtras(pParent);
  pParent->SetFlag(NF_ALLCHILDREN, true);
}

CAlphabetManager::CAlphNode::~CAlphNode() {
  delete m_pProbInfo;
  m_pMgr->m_pLanguageModel->ReleaseContext(iContext);
}

const std::string &CAlphabetManager::CSymbolNode::outputText() const {
  if (iSymbol == m_pMgr->m_pAlphabet->GetParagraphSymbol() && GetFlag(NF_SEEN)) {
    //Regardless of this particular platform's definition of a newline,
    // which is what we'd _output_, when reversing back over text
    // which may have been produced elsewhere, we represent occurrences
    // of _either_ \n or \r\n by a single paragraph symbol.
    //If the alphabet has a paragraph symbol, \r is not a symbol on its own
    // (and \n isn't a symbol other than paragraph). So look for a
    // \r before the \n.
    DASHER_ASSERT(m_pMgr->m_pInterface->GetContext(offset(),1)=="\n");
    static std::string rn("\r\n"),n("\n"); //must store strings somewhere to return by reference!
    return (m_pMgr->m_pInterface->GetContext(offset()-1,2)=="\r\n") ? rn : n;
  }
  return mgr()->m_pAlphabet->GetText(iSymbol);
}

string CAlphabetManager::CSymbolNode::trainText() {
  return m_pMgr->m_pAlphabet->escape(outputText());
}

int CAlphabetManager::CSymbolNode::numChars() {
  return (outputText()=="\r\n") ? 2 : 1;
}

void CAlphabetManager::CSymbolNode::Output() {
  if (m_pMgr->GetBoolParameter(BP_LM_ADAPTIVE)) {
    if (m_pMgr->m_pLastOutput != Parent()) {
      //Context changed. Flush to disk the old context + text written in it...
      m_pMgr->WriteTrainFileFull(m_pMgr->m_pInterface);

      ///Now extract the context in which this node was written.
      /// Since this node is being output now, its parent must already have been,
      /// so the simplest thing is to read from the edit buffer!
      int iStart = max(0, offset() - m_pMgr->m_pLanguageModel->GetContextLength());
      m_pMgr->strTrainfileContext = m_pMgr->m_pInterface->GetContext(iStart, offset()-iStart);
      if (m_pMgr->strTrainfileContext=="") //Even the empty context (as for a new document)
        m_pMgr->strTrainfileContext = m_pMgr->m_pAlphabet->GetDefaultContext(); //is a new ctx!
    }
    //Now handle outputting of this node
    m_pMgr->m_pLastOutput = this;
    string tr(trainText());
    m_pMgr->strTrainfileBuffer += tr;
    //an actual occurrence of the escape character, must be doubled (like \\)
    if (tr == m_pMgr->m_pAlphabet->GetContextEscapeChar()) m_pMgr->strTrainfileBuffer+=tr;
  }
  //std::cout << this << " " << Parent() << ": Output at offset " << m_iOffset << " *" << m_pMgr->m_pAlphabet->GetText(t) << "* " << std::endl;

  m_pMgr->m_pInterface->editOutput(outputText(), this);
}

SymbolProb CAlphabetManager::CSymbolNode::GetSymbolProb() const {
  //TODO probability here not right - Range() is relative to parent, not prev symbol
  return Dasher::SymbolProb(iSymbol, outputText(), Range() / (double)CDasherModel::NORMALIZATION);
}

void CAlphabetManager::CSymbolNode::Undo() {
  DASHER_ASSERT(GetFlag(NF_SEEN));
  if (m_pMgr->GetBoolParameter(BP_LM_ADAPTIVE)) {
    if (m_pMgr->m_pLastOutput == this) {
      //Erase from training buffer, and move lastOutput backwards,
      // iff this node was actually written (i.e. not rebuilt _from_ context!)
      std::string &buf(m_pMgr->strTrainfileBuffer);
      std::string tr(trainText());
      if (tr.length()<=buf.length()
          && buf.substr(buf.length()-tr.length(),tr.length())==tr) {
        buf=buf.substr(0,buf.length()-tr.length());
        m_pMgr->m_pLastOutput = Parent();
      }
    }
  } else CAlphBase::Undo();
  m_pMgr->m_pInterface->editDelete(outputText(), this);
}

CDasherNode *CAlphabetManager::CGroupNode::RebuildParent() {

  if (Parent()) return Parent();

  if (m_pGroup == m_pMgr->m_pBaseGroup) {
    //top level root node.
    //if (offset()>0), there was _something_ before us, like
    // a control node; but we no longer know what!
    return NULL;
  }
  
  //All other CGroupNode's have a container i.e. the parent group
  return CAlphBase::RebuildParent();
}

CDasherNode *CAlphabetManager::CAlphBase::RebuildParent() {
  if (!Parent()) {
    //Parent's offset usually one less than this, but can be two for the paragraph symbol.
    int iNewOffset = offset()-numChars();

    CAlphNode *pNewNode = m_pMgr->GetRoot(NULL, iNewOffset!=-1, iNewOffset+1);

    RebuildForwardsFromAncestor(pNewNode);

    if (int flags=(GetFlag(NF_SEEN) ? NF_SEEN : 0) | (GetFlag(NF_COMMITTED) ? NF_COMMITTED : 0)) {
      for (CDasherNode *pNode=this; (pNode=pNode->Parent()); pNode->SetFlag(flags, true));
    }
  }
  return Parent();
}

void CAlphabetManager::CAlphBase::RebuildForwardsFromAncestor(CAlphNode *pNewNode) {
  //now fill in the new node - recursively - until it reaches us
  m_pMgr->IterateChildGroups(pNewNode, m_pMgr->m_pBaseGroup, this);
}

// TODO: Shouldn't there be an option whether or not to learn as we write?
// For want of a better solution, game mode exemption explicit in this function
void CAlphabetManager::CSymbolNode::SetFlag(int iFlag, bool bValue) {
  if ((iFlag & NF_COMMITTED) && bValue && !GetFlag(NF_COMMITTED | NF_GAME)
      && m_pMgr->GetBoolParameter(BP_LM_ADAPTIVE)) {
    //try to commit...if we have parent (else rebuilding (backwards) => don't)
    if (Parent()) {
      if (Parent()->mgr() != mgr()) return; //do not set flag
      CLanguageModel *pLM(m_pMgr->m_pLanguageModel);
      // (Note: for first symbol after startup: parent is (root) group node, which'll have the alphabet default context)
      CLanguageModel::Context ctx = pLM->CloneContext(static_cast<CAlphabetManager::CAlphNode *>(Parent())->iContext);
      pLM->LearnSymbol(ctx, iSymbol);
      //could: pLM->ReleaseContext(ctx);
      //however, seems better to replace this node's context (i.e. which it uses to create its own children)
      // with the new (learned) context: the former was obtained by EnterSymbol rather than LearnSymbol, so
      // will be different iff this node was the first time its symbol was entered into its parent context.
      // (Yes, this node's context is unlikely to be used again, but not impossible...)
      pLM->ReleaseContext(iContext);
      iContext = ctx;
    }
  }
  CDasherNode::SetFlag(iFlag, bValue);
}
