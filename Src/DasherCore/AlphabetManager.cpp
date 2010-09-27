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
#include "EventHandler.h"
#include "NodeCreationManager.h"


#include <vector>
#include <sstream>
#include <iostream>

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

CAlphabetManager::CAlphabetManager(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, const CAlphInfo *pAlphabet, const CAlphabetMap *pAlphabetMap, CLanguageModel *pLanguageModel)
  : m_pLanguageModel(pLanguageModel), m_pNCManager(pNCManager), m_pAlphabet(pAlphabet), m_pAlphabetMap(pAlphabetMap) {
  m_pInterface = pInterface;

  m_iLearnContext = m_pLanguageModel->CreateEmptyContext();
}

const CAlphInfo *CAlphabetManager::GetAlphabet() const {
  return m_pAlphabet;
}

CAlphabetManager::~CAlphabetManager() {
  m_pLanguageModel->ReleaseContext(m_iLearnContext);
  //the alphabet belongs to the AlphIO, and may be reused later;
  delete m_pAlphabetMap; //the map was created for this mgr.
  delete m_pLanguageModel;
}

CAlphabetManager::CAlphNode::CAlphNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, int iColour, const string &strDisplayText, CAlphabetManager *pMgr)
: CDasherNode(pParent, iOffset, iLbnd, iHbnd, iColour, strDisplayText), m_pProbInfo(NULL), m_pMgr(pMgr) {
}

CAlphabetManager::CSymbolNode::CSymbolNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, CAlphabetManager *pMgr, symbol _iSymbol)
: CAlphNode(pParent, iOffset, iLbnd, iHbnd, pMgr->m_pAlphabet->GetColour(_iSymbol, iOffset%2), pMgr->m_pAlphabet->GetDisplayText(_iSymbol), pMgr), iSymbol(_iSymbol) {
}

CAlphabetManager::CSymbolNode::CSymbolNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, int iColour, const string &strDisplayText, CAlphabetManager *pMgr, symbol _iSymbol)
: CAlphNode(pParent, iOffset, iLbnd, iHbnd, iColour, strDisplayText, pMgr), iSymbol(_iSymbol) {
}

CAlphabetManager::CGroupNode::CGroupNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, CAlphabetManager *pMgr, const SGroupInfo *pGroup)
: CAlphNode(pParent, iOffset, iLbnd, iHbnd,
            pGroup ? (pGroup->bVisible ? pGroup->iColour : pParent->getColour())
            : iOffset%2 == 1 ? 140 : 10, //special case - was AlphInfo::GetColour for symbol _0_
            pGroup ? pGroup->strLabel : "", pMgr), m_pGroup(pGroup) {
}

CAlphabetManager::CSymbolNode *CAlphabetManager::makeSymbol(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, symbol iSymbol) {
  return new CSymbolNode(pParent, iOffset, iLbnd, iHbnd, this, iSymbol);
}

CAlphabetManager::CGroupNode *CAlphabetManager::makeGroup(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, const SGroupInfo *pGroup) {
  return new CGroupNode(pParent, iOffset, iLbnd, iHbnd, this, pGroup);
}

CAlphabetManager::CAlphNode *CAlphabetManager::GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, bool bEnteredLast, int iOffset) {

  int iNewOffset(max(-1,iOffset-1));
  
  std::vector<symbol> vContextSymbols;
  // TODO: make the LM get the context, rather than force it to fix max context length as an int
  int iStart = max(0, iNewOffset - m_pLanguageModel->GetContextLength());
  
  if(pParent) {
    pParent->GetContext(m_pInterface, m_pAlphabetMap, vContextSymbols, iStart, iNewOffset+1 - iStart);
  } else {
    std::string strContext = (iNewOffset == -1) 
      ? m_pAlphabet->GetDefaultContext()
      : m_pInterface->GetContext(iStart, iNewOffset+1 - iStart);
    m_pAlphabetMap->GetSymbols(vContextSymbols, strContext);
  }
  
  CAlphNode *pNewNode;
  CLanguageModel::Context iContext = m_pLanguageModel->CreateEmptyContext();
  
  std::vector<symbol>::iterator it = vContextSymbols.end();
  while (it!=vContextSymbols.begin()) {
    if (*(--it) == 0) {
      //found an impossible symbol! start after it
      ++it;
      break;
    }
  }
  if (it == vContextSymbols.end()) {
    //previous character was not in the alphabet!
    //can't construct a node "responsible" for entering it
    bEnteredLast=false;
    //instead, Create a node as if we were starting a new sentence...
    vContextSymbols.clear();
    m_pAlphabetMap->GetSymbols(vContextSymbols, m_pAlphabet->GetDefaultContext());
    it = vContextSymbols.begin();
    //TODO: What it the default context somehow contains symbols not in the alphabet?
  }
  //enter the symbols we could make sense of, into the LM context...
  while (it != vContextSymbols.end()) {
    m_pLanguageModel->EnterSymbol(iContext, *(it++));
  }
  
  if(!bEnteredLast) {
    pNewNode = makeGroup(pParent, iNewOffset, iLower, iUpper,  NULL);
  } else {
    const symbol iSymbol(vContextSymbols[vContextSymbols.size() - 1]);
    pNewNode = makeSymbol(pParent, iNewOffset, iLower, iUpper, iSymbol);
    //if the new node is not child of an existing node, then it
    // represents a symbol that's already happened - so we're either
    // going backwards (rebuildParent) or creating a new root after a language change
    DASHER_ASSERT (!pParent);
    pNewNode->SetFlag(NF_SEEN, true);
  }

  pNewNode->iContext = iContext;
  return pNewNode;
}

bool CAlphabetManager::CSymbolNode::GameSearchNode(string strTargetUtf8Char) {
  if (m_pMgr->m_pAlphabet->GetText(iSymbol) == strTargetUtf8Char) {
    SetFlag(NF_GAME, true);
    return true;
  }
  return false;
}
bool CAlphabetManager::CGroupNode::GameSearchNode(string strTargetUtf8Char) {
  if (GameSearchChildren(strTargetUtf8Char)) {
    SetFlag(NF_GAME, true);
    return true;
  }
return false;
}

CLanguageModel::Context CAlphabetManager::CAlphNode::CloneAlphContext(CLanguageModel *pLanguageModel) {
  if (iContext) return pLanguageModel->CloneContext(iContext);
  return CDasherNode::CloneAlphContext(pLanguageModel);
}

void CAlphabetManager::CSymbolNode::GetContext(CDasherInterfaceBase *pInterface, const CAlphabetMap *pAlphabetMap, vector<symbol> &vContextSymbols, int iOffset, int iLength) {
  if (!GetFlag(NF_SEEN) && iOffset+iLength-1 == offset()) {
    if (iLength > 1) Parent()->GetContext(pInterface, pAlphabetMap, vContextSymbols, iOffset, iLength-1);
    vContextSymbols.push_back(iSymbol);
  } else {
    CDasherNode::GetContext(pInterface, pAlphabetMap, vContextSymbols, iOffset, iLength);
  }
}

symbol CAlphabetManager::CSymbolNode::GetAlphSymbol() {
  return iSymbol;
}

void CAlphabetManager::CSymbolNode::PopulateChildren() {
  m_pMgr->IterateChildGroups(this, NULL, NULL);
}
int CAlphabetManager::CAlphNode::ExpectedNumChildren() {
  int i=m_pMgr->m_pAlphabet->iNumChildNodes;
  return (m_pMgr->m_pNCManager->GetBoolParameter(BP_CONTROL_MODE)) ? i+1 : i;
}

void CAlphabetManager::GetProbs(vector<unsigned int> *pProbInfo, CLanguageModel::Context context) {
  const unsigned int iSymbols = m_pAlphabet->GetNumberTextSymbols();
  unsigned long iNorm(m_pNCManager->GetLongParameter(LP_NORMALIZATION));
  
  // TODO - sort out size of control node - for the timebeing I'll fix the control node at 5%
  // TODO: New method (see commented code) has been removed as it wasn' working.
  
  const int iControlSpace = m_pNCManager->GetBoolParameter(BP_CONTROL_MODE) ? iNorm / 20 : 0;
  
  iNorm -= iControlSpace;
  
  const unsigned long uniform(m_pNCManager->GetLongParameter(LP_UNIFORM)); 
  //the case for control mode on, generalizes to handle control mode off also,
  // as then iNorm - control_space == iNorm...
  const unsigned int iUniformAdd = ((iNorm * uniform) / 1000) / iSymbols;
  const unsigned long iNonUniformNorm = iNorm - iSymbols * iUniformAdd;
  //  m_pLanguageModel->GetProbs(context, Probs, iNorm, ((iNorm * uniform) / 1000));
  
  //ACL used to test explicitly for MandarinDasher and if so called GetPYProbs instead
  // (by statically casting to PPMPYLanguageModel). However, have renamed PPMPYLanguageModel::GetPYProbs
  // to GetProbs as per ordinary language model, so no need to test....
  m_pLanguageModel->GetProbs(context, *pProbInfo, iNonUniformNorm, 0);
  
  DASHER_ASSERT(pProbInfo->size() == m_pAlphabet->GetNumberTextSymbols()+1);//initial 0
  
  for(unsigned int k(1); k < pProbInfo->size(); ++k)
    (*pProbInfo)[k] += iUniformAdd;
  
#ifdef DEBUG
  {
    int iTotal = 0;
    for(int k = 0; k < pProbInfo->size(); ++k)
      iTotal += (*pProbInfo)[k];
    DASHER_ASSERT(iTotal == m_pNCManager->GetLongParameter(LP_NORMALIZATION) - iControlSpace);
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
  if (m_pGroup && Parent() && Parent()->mgr() == mgr()) {
    DASHER_ASSERT(Parent()->offset() == offset());
    return (static_cast<CAlphNode *>(Parent()))->GetProbInfo();
  }
  //nope, no usable parent. compute here...
  return CAlphNode::GetProbInfo();
}

void CAlphabetManager::CGroupNode::PopulateChildren() {
  m_pMgr->IterateChildGroups(this, m_pGroup, NULL);
}

int CAlphabetManager::CGroupNode::ExpectedNumChildren() {
  return (m_pGroup) ? m_pGroup->iNumChildNodes : CAlphNode::ExpectedNumChildren();
}

CAlphabetManager::CGroupNode *CAlphabetManager::CreateGroupNode(CAlphNode *pParent, const SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd) {

  // When creating a group node...
  // ...the offset is the same as the parent...
  CGroupNode *pNewNode = makeGroup(pParent, pParent->offset(), iLbnd, iHbnd, pInfo);

  //...as is the context!
  pNewNode->iContext = m_pLanguageModel->CloneContext(pParent->iContext);

  return pNewNode;
}

CAlphabetManager::CGroupNode *CAlphabetManager::CGroupNode::RebuildGroup(CAlphNode *pParent, const SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd) {
  if (pInfo == m_pGroup) {
    SetRange(iLbnd, iHbnd);
    SetParent(pParent);
    //offset doesn't increase for groups...
    DASHER_ASSERT (offset() == pParent->offset());
    return this;
  }
  CGroupNode *pRet=m_pMgr->CreateGroupNode(pParent, pInfo, iLbnd, iHbnd);
  if (pInfo->iStart <= m_pGroup->iStart && pInfo->iEnd >= m_pGroup->iEnd) {
    //created group node should contain this one
    m_pMgr->IterateChildGroups(pRet,pInfo,this);
  }
  return pRet;
}

CAlphabetManager::CGroupNode *CAlphabetManager::CSymbolNode::RebuildGroup(CAlphNode *pParent, const SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd) {
  CGroupNode *pRet=m_pMgr->CreateGroupNode(pParent, pInfo, iLbnd, iHbnd);
  if (pInfo->iStart <= iSymbol && pInfo->iEnd > iSymbol) {
    m_pMgr->IterateChildGroups(pRet, pInfo, this);
  }
  return pRet;
}

CLanguageModel::Context CAlphabetManager::CreateSymbolContext(CAlphNode *pParent, symbol iSymbol)
{
  CLanguageModel::Context iContext = m_pLanguageModel->CloneContext(pParent->iContext);
  m_pLanguageModel->EnterSymbol(iContext, iSymbol); // TODO: Don't use symbols?
  return iContext;
}

CDasherNode *CAlphabetManager::CreateSymbolNode(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd) {

    // TODO: Exceptions / error handling in general

    CAlphNode *pAlphNode = makeSymbol(pParent, pParent->offset()+1, iLbnd, iHbnd, iSymbol);

    //     std::stringstream ssLabel;

    //     ssLabel << m_pAlphabet->GetDisplayText(iSymbol) << ": " << pNewNode;

    //    pDisplayInfo->strDisplayText = ssLabel.str();

    pAlphNode->iContext = CreateSymbolContext(pParent, iSymbol);

  return pAlphNode;
}

CDasherNode *CAlphabetManager::CSymbolNode::RebuildSymbol(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd) {
  if(iSymbol == this->iSymbol) {
    SetRange(iLbnd, iHbnd);
    SetParent(pParent);
    DASHER_ASSERT(offset() == pParent->offset() + 1);
    return this;
  }
  return m_pMgr->CreateSymbolNode(pParent, iSymbol, iLbnd, iHbnd);
}

CDasherNode *CAlphabetManager::CGroupNode::RebuildSymbol(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd) {
  return m_pMgr->CreateSymbolNode(pParent, iSymbol, iLbnd, iHbnd);
}

void CAlphabetManager::IterateChildGroups(CAlphNode *pParent, const SGroupInfo *pParentGroup, CAlphNode *buildAround) {
  std::vector<unsigned int> *pCProb(pParent->GetProbInfo());
  DASHER_ASSERT((*pCProb)[0] == 0);
  const int iMin(pParentGroup ? pParentGroup->iStart : 1);
  const int iMax(pParentGroup ? pParentGroup->iEnd : m_pAlphabet->GetNumberTextSymbols()+1);
  unsigned int iRange(pParentGroup ? ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]) : m_pNCManager->GetLongParameter(LP_NORMALIZATION));
  
  // TODO: Think through alphabet file formats etc. to make this class easier.
  // TODO: Throw a warning if parent node already has children
  
  // Create child nodes and add them
  
  int i(iMin); //lowest index of child which we haven't yet added
  const SGroupInfo *pCurrentNode(pParentGroup ? pParentGroup->pChild : m_pAlphabet->m_pBaseGroup);
  // The SGroupInfo structure has something like linked list behaviour
  // Each SGroupInfo contains a pNext, a pointer to a sibling group info
  while (i < iMax) {
    CDasherNode *pNewChild;
    bool bSymbol = !pCurrentNode //gone past last subgroup
                  || i < pCurrentNode->iStart; //not reached next subgroup
    const int iStart=i, iEnd = (bSymbol) ? i+1 : pCurrentNode->iEnd;
    //uint64 is platform-dependently #defined in DasherTypes.h as an (unsigned) 64-bit int ("__int64" or "long long int")
    unsigned int iLbnd = (((*pCProb)[iStart-1] - (*pCProb)[iMin-1]) *
                          (uint64)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) /
                         iRange;
    unsigned int iHbnd = (((*pCProb)[iEnd-1] - (*pCProb)[iMin-1]) *
                          (uint64)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) /
                         iRange;
    //loop for eliding groups with single children (see below).
    // Variables store necessary properties of any elided groups:
    std::string groupPrefix=""; int iOverrideColour=-1;
    const SGroupInfo *pInner=pCurrentNode;
    while (true) {
      if (bSymbol) {
        pNewChild = (buildAround) ? buildAround->RebuildSymbol(pParent, i, iLbnd, iHbnd) : CreateSymbolNode(pParent, i, iLbnd, iHbnd);
        i++; //make one symbol at a time - move onto next symbol in next iteration of (outer) loop
        break; //exit inner (group elision) loop
      } else if (pInner->iNumChildNodes>1) { //in/reached nontrivial subgroup - do make node for entire group:
        pNewChild= (buildAround) ? buildAround->RebuildGroup(pParent, pInner, iLbnd, iHbnd) : CreateGroupNode(pParent, pInner, iLbnd, iHbnd);
        i = pInner->iEnd; //make one group at a time - so move past entire group...
        pCurrentNode = pCurrentNode->pNext; //next sibling of _original_ pCurrentNode (above)
                                     // (maybe not of pCurrentNode now, which might be a subgroup filling the original)
        break; //exit inner (group elision) loop
      }
      //were about to create a group node, which would have only one child
      // (eventually, if the group node were PopulateChildren'd).
      // Such a child would entirely fill it's parent (the group), and thus,
      // creation/destruction of the child would cause the node's colour to flash
      // between that for parent group and child.
      // Hence, instead we elide the group node and create the child _here_...
      
      //1. however we also have to take account of the appearance of the elided group. Hence:
      groupPrefix += pInner->strLabel;
      if (pInner->bVisible) iOverrideColour=pInner->iColour;
      //2. now go into the group...
      pInner = pInner->pChild;
      bSymbol = (pInner==NULL); //which might contain a single subgroup, or a single symbol
      if (bSymbol) pCurrentNode = pCurrentNode->pNext; //if a symbol, we've still moved past the outer (elided) group
      DASHER_ASSERT(iEnd == (bSymbol ? i+1 : pInner->iEnd)); //probability calcs still ok
      //3. loop round inner loop...
    }
    //created a new node - symbol or (group which will have >1 child).
    DASHER_ASSERT(pParent->GetChildren().back()==pNewChild);
    //now adjust the node we've actually created, to take account of any elided group(s)...
    // tho not if we've reused the existing node, assume that's been adjusted already
    if (pNewChild && pNewChild!=buildAround) pNewChild->PrependElidedGroup(iOverrideColour, groupPrefix);
  }

  if (!pParentGroup) AddExtras(pParent,pCProb);
  pParent->SetFlag(NF_ALLCHILDREN, true);
}

void CAlphabetManager::AddExtras(CAlphNode *pParent, vector<unsigned int> *pCProb) {
  //control mode:
  DASHER_ASSERT((pParent->GetChildren().back()->Hbnd() == m_pNCManager->GetLongParameter(LP_NORMALIZATION)) ^ m_pNCManager->GetBoolParameter(BP_CONTROL_MODE));
  if (m_pNCManager->GetBoolParameter(BP_CONTROL_MODE)) {
#ifdef _WIN32_WCE
    DASHER_ASSERT(false);
#endif
    //ACL leave offset as is - like its groupnode parent, but unlike its alphnode siblings,
    //the control node does not enter a symbol....
    m_pNCManager->GetControlManager()->GetRoot(pParent, pParent->GetChildren().back()->Hbnd(), m_pNCManager->GetLongParameter(LP_NORMALIZATION), pParent->offset());
  }
}


CAlphabetManager::CAlphNode::~CAlphNode() {
  delete m_pProbInfo;
  m_pMgr->m_pLanguageModel->ReleaseContext(iContext);
}

const std::string &CAlphabetManager::CSymbolNode::outputText() {
  return mgr()->m_pAlphabet->GetText(iSymbol);
}

void CAlphabetManager::CSymbolNode::Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  //std::cout << this << " " << Parent() << ": Output at offset " << m_iOffset << " *" << m_pMgr->m_pAlphabet->GetText(t) << "* " << std::endl;

  Dasher::CEditEvent oEvent(1, outputText(), offset());
  m_pMgr->m_pNCManager->InsertEvent(&oEvent);

  // Track this symbol and its probability for logging purposes
  if (pAdded != NULL) {
    pAdded->push_back(Dasher::SymbolProb(iSymbol, outputText(), Range() / (double)iNormalization));
  }
}

void CAlphabetManager::CSymbolNode::Undo(int *pNumDeleted) {
  Dasher::CEditEvent oEvent(2, outputText(), offset());
  m_pMgr->m_pNCManager->InsertEvent(&oEvent);
  if (pNumDeleted) (*pNumDeleted)++;
}

CDasherNode *CAlphabetManager::CGroupNode::RebuildParent() {
  // CAlphNode's always have a parent, they inserted a symbol; CGroupNode's
  // with an m_pGroup have a container i.e. the parent group, unless
  // m_pGroup==NULL => "root" node where Alphabet->m_pBaseGroup is the *first*child*...
  if (m_pGroup == NULL) return NULL;
  //offset of group node is same as parent...
  return CAlphNode::RebuildParent(offset());
}

CDasherNode *CAlphabetManager::CSymbolNode::RebuildParent() {
  //parent's offset is one less than this.
  return CAlphNode::RebuildParent(offset()-1);
}

CDasherNode *CAlphabetManager::CAlphNode::RebuildParent(int iNewOffset) {
  //possible that we have a parent, as RebuildParent() rebuilds back to closest AlphNode.
  if (Parent()) return Parent();
  
  CAlphNode *pNewNode = m_pMgr->GetRoot(NULL, 0, 0, iNewOffset!=-1, iNewOffset+1);
  
  //now fill in the new node - recursively - until it reaches us
  m_pMgr->IterateChildGroups(pNewNode, NULL, this);

  //finally return our immediate parent (pNewNode may be an ancestor rather than immediate parent!)
  DASHER_ASSERT(Parent() != NULL);

  //although not required, we believe only NF_SEEN nodes are ever requested to rebuild their parents...
  DASHER_ASSERT(GetFlag(NF_SEEN));
  //so set NF_SEEN on all created ancestors (of which pNewNode is the last)
  CDasherNode *pNode = this;
  do {
    pNode = pNode->Parent();
    pNode->SetFlag(NF_SEEN, true);
  } while (pNode != pNewNode);
  
  return Parent();
}

// TODO: Shouldn't there be an option whether or not to learn as we write?
// For want of a better solution, game mode exemption explicit in this function
void CAlphabetManager::CSymbolNode::SetFlag(int iFlag, bool bValue) {
  CDasherNode::SetFlag(iFlag, bValue);
  switch(iFlag) {
  case NF_COMMITTED:
    if(bValue && !GetFlag(NF_GAME) && m_pMgr->m_pInterface->GetBoolParameter(BP_LM_ADAPTIVE))
      m_pMgr->m_pLanguageModel->LearnSymbol(m_pMgr->m_iLearnContext, iSymbol);
    break;
  }
}
