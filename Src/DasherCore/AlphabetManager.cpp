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

CAlphabetManager::CAlphabetManager(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel)
  : m_pLanguageModel(pLanguageModel), m_pNCManager(pNCManager) {
  m_pInterface = pInterface;

  m_iLearnContext = m_pLanguageModel->CreateEmptyContext();

}

CAlphabetManager::CAlphNode::CAlphNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDisplayInfo, CAlphabetManager *pMgr)
: CDasherNode(pParent, iLbnd, iHbnd, pDisplayInfo), m_pMgr(pMgr), m_pProbInfo(NULL) {
};

CAlphabetManager::CSymbolNode::CSymbolNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDisplayInfo, CAlphabetManager *pMgr, symbol _iSymbol)
: CAlphNode(pParent, iLbnd, iHbnd, pDisplayInfo, pMgr), iSymbol(_iSymbol) {
};

CAlphabetManager::CGroupNode::CGroupNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDisplayInfo, CAlphabetManager *pMgr, SGroupInfo *pGroup)
: CAlphNode(pParent, iLbnd, iHbnd, pDisplayInfo, pMgr), m_pGroup(pGroup) {
};

CAlphabetManager::CSymbolNode *CAlphabetManager::makeSymbol(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDisplayInfo, symbol iSymbol) {
  return new CSymbolNode(pParent, iLbnd, iHbnd, pDisplayInfo, this, iSymbol);
}

CAlphabetManager::CGroupNode *CAlphabetManager::makeGroup(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDisplayInfo, SGroupInfo *pGroup) {
  return new CGroupNode(pParent, iLbnd, iHbnd, pDisplayInfo, this, pGroup);
}

CAlphabetManager::CAlphNode *CAlphabetManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, bool bEnteredLast, int iOffset) {

  int iNewOffset(max(-1,iOffset-1));
  
  std::vector<symbol> vContextSymbols;
  // TODO: make the LM get the context, rather than force it to fix max context length as an int
  int iStart = max(0, iNewOffset - m_pLanguageModel->GetContextLength());
  
  if(pParent) {
    pParent->GetContext(m_pInterface, vContextSymbols, iStart, iNewOffset+1 - iStart);
  } else {
    std::string strContext = (iNewOffset == -1) 
      ? m_pNCManager->GetAlphabet()->GetDefaultContext()
      : m_pInterface->GetContext(iStart, iNewOffset+1 - iStart);
    m_pNCManager->GetAlphabet()->GetSymbols(vContextSymbols, strContext);
  }

  CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
  pDisplayInfo->bShove = true;
  pDisplayInfo->bVisible = true;  
  
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
    m_pNCManager->GetAlphabet()->GetSymbols(vContextSymbols, m_pNCManager->GetAlphabet()->GetDefaultContext());
    it = vContextSymbols.begin();
    //TODO: What it the default context somehow contains symbols not in the alphabet?
  }
  //enter the symbols we could make sense of, into the LM context...
  while (it != vContextSymbols.end()) {
    m_pLanguageModel->EnterSymbol(iContext, *(it++));
  }
  
  if(!bEnteredLast) {
    pDisplayInfo->strDisplayText = ""; //equivalent to do m_pNCManager->GetAlphabet()->GetDisplayText(0)
    pDisplayInfo->iColour = m_pNCManager->GetAlphabet()->GetColour(0, iNewOffset%2);
    pNewNode = makeGroup(pParent, iLower, iUpper, pDisplayInfo, NULL);
  } else {
    const symbol iSymbol(vContextSymbols[vContextSymbols.size() - 1]);
    pDisplayInfo->strDisplayText = m_pNCManager->GetAlphabet()->GetDisplayText(iSymbol);
    pDisplayInfo->iColour = m_pNCManager->GetAlphabet()->GetColour(iSymbol, iNewOffset%2);
    pNewNode = makeSymbol(pParent, iLower, iUpper, pDisplayInfo, iSymbol);
    //if the new node is not child of an existing node, then it
    // represents a symbol that's already happened - so we're either
    // going backwards (rebuildParent) or creating a new root after a language change
    DASHER_ASSERT (!pParent);
    pNewNode->SetFlag(NF_SEEN, true);
  }

  pNewNode->m_iOffset = iNewOffset;

  pNewNode->iContext = iContext;
  return pNewNode;
}

bool CAlphabetManager::CSymbolNode::GameSearchNode(string strTargetUtf8Char) {
  if (m_pMgr->m_pNCManager->GetAlphabet()->GetText(iSymbol) == strTargetUtf8Char) {
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

void CAlphabetManager::CSymbolNode::GetContext(CDasherInterfaceBase *pInterface, vector<symbol> &vContextSymbols, int iOffset, int iLength) {
  if (!GetFlag(NF_SEEN) && iOffset+iLength-1 == m_iOffset) {
    if (iLength > 1) Parent()->GetContext(pInterface, vContextSymbols, iOffset, iLength-1);
    vContextSymbols.push_back(iSymbol);
  } else {
    CDasherNode::GetContext(pInterface, vContextSymbols, iOffset, iLength);
  }
}

symbol CAlphabetManager::CSymbolNode::GetAlphSymbol() {
  return iSymbol;
}

void CAlphabetManager::CSymbolNode::PopulateChildren() {
  m_pMgr->IterateChildGroups(this, NULL, NULL);
}
int CAlphabetManager::CAlphNode::ExpectedNumChildren() {
  return m_pMgr->m_pNCManager->GetAlphabet()->iNumChildNodes;
}

std::vector<unsigned int> *CAlphabetManager::CAlphNode::GetProbInfo() {
  if (!m_pProbInfo) {
    m_pProbInfo = new std::vector<unsigned int>();
    m_pMgr->m_pNCManager->GetProbs(iContext, *m_pProbInfo, m_pMgr->m_pNCManager->GetLongParameter(LP_NORMALIZATION));
    // work out cumulative probs in place
    for(unsigned int i = 1; i < m_pProbInfo->size(); i++) {
      (*m_pProbInfo)[i] += (*m_pProbInfo)[i - 1];
    }
  }
  return m_pProbInfo;
}

std::vector<unsigned int> *CAlphabetManager::CGroupNode::GetProbInfo() {
  if (m_pGroup && Parent() && Parent()->mgrId() == 0) {
    DASHER_ASSERT(Parent()->m_iOffset == m_iOffset);
    return (static_cast<CAlphNode *>(Parent()))->GetProbInfo();
  }
  //nope, no usable parent. compute here...
  return CAlphNode::GetProbInfo();
}

void CAlphabetManager::CGroupNode::PopulateChildren() {
  m_pMgr->IterateChildGroups(this, m_pGroup, NULL);
  if (GetChildren().size()==1) {
    CDasherNode *pChild = GetChildren()[0];
    //single child, must therefore completely fill this node...
    DASHER_ASSERT(pChild->Lbnd()==0 && pChild->Hbnd()==65536);
    //in earlier versions of Dasher with subnodes, that child would have been created
    // at the same time as this node, so this node would never be seen/rendered (as the
    // child would cover it). However, lazily (as we do now) creating the child, will
    // suddenly obscure this (parent) node, changing it's colour. Hence, avoid this by
    // making the child look like the parent...(note that changing the parent, before the
    // child is created, to look like the child will do, would more closely mirror the old
    // behaviour, but we can't really do that!
    CDasherNode::SDisplayInfo *pInfo = (CDasherNode::SDisplayInfo *)pChild->GetDisplayInfo();
    //ick, note the cast to get rid of 'const'ness. TODO: do something about SDisplayInfo...!!
    pInfo->bVisible=false;
    pInfo->iColour = GetDisplayInfo()->iColour;
  }
}
int CAlphabetManager::CGroupNode::ExpectedNumChildren() {
  return (m_pGroup) ? m_pGroup->iNumChildNodes : CAlphNode::ExpectedNumChildren();
}

CAlphabetManager::CGroupNode *CAlphabetManager::CreateGroupNode(CAlphNode *pParent, SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd) {
  // TODO: More sensible structure in group data to map directly to this
  CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
  pDisplayInfo->iColour = (pInfo->bVisible ? pInfo->iColour : pParent->GetDisplayInfo()->iColour);
  pDisplayInfo->bShove = true;
  pDisplayInfo->bVisible = pInfo->bVisible;
  pDisplayInfo->strDisplayText = pInfo->strLabel;

  CGroupNode *pNewNode = makeGroup(pParent, iLbnd, iHbnd, pDisplayInfo, pInfo);

  // When creating a group node...
  pNewNode->m_iOffset = pParent->m_iOffset; // ...the offset is the same as the parent...
  pNewNode->iContext = m_pLanguageModel->CloneContext(pParent->iContext);

  return pNewNode;
}

CAlphabetManager::CGroupNode *CAlphabetManager::CGroupNode::RebuildGroup(CAlphNode *pParent, SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd) {
  if (pInfo == m_pGroup) {
    SetRange(iLbnd, iHbnd);
    SetParent(pParent);
    //offset doesn't increase for groups...
    DASHER_ASSERT (m_iOffset == pParent->m_iOffset);
    return this;
  }
  CGroupNode *pRet=m_pMgr->CreateGroupNode(pParent, pInfo, iLbnd, iHbnd);
  if (pInfo->iStart <= m_pGroup->iStart && pInfo->iEnd >= m_pGroup->iEnd) {
    //created group node should contain this one
    m_pMgr->IterateChildGroups(pRet,pInfo,this);
  }
  return pRet;
}

CAlphabetManager::CGroupNode *CAlphabetManager::CSymbolNode::RebuildGroup(CAlphNode *pParent, SGroupInfo *pInfo, unsigned int iLbnd, unsigned int iHbnd) {
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

  CDasherNode *pNewNode = NULL;

  //Does not invoke conversion node

  // TODO: Better way of specifying alternate roots

  // TODO: Need to fix fact that this is created even when control mode is switched off
  if(iSymbol == m_pNCManager->GetAlphabet()->GetControlSymbol()) {
      //ACL setting offset as one more than parent for consistency with "proper" symbol nodes...
      pNewNode = m_pNCManager->GetCtrlRoot(pParent, iLbnd, iHbnd, pParent->m_iOffset+1); 

#ifdef _WIN32_WCE
      //no control manager - but (TODO!) we still try to create (0-size!) control node...
      DASHER_ASSERT(!pNewNode);
      // For now, just hack it so we get a normal root node here
      pNewNode = m_pNCManager->GetAlphRoot(pParent, iLbnd, iHbnd, false, pParent->m_iOffset+1);
#else
      DASHER_ASSERT(pNewNode);
#endif
    }
    else if(iSymbol == m_pNCManager->GetAlphabet()->GetStartConversionSymbol()) {
      //  else if(iSymbol == m_pNCManager->GetSpaceSymbol()) {

      //ACL setting m_iOffset+1 for consistency with "proper" symbol nodes...
      pNewNode = m_pNCManager->GetConvRoot(pParent, iLbnd, iHbnd, pParent->m_iOffset+1);
    }
    else {
      //compute phase directly from offset
      int iColour = m_pNCManager->GetAlphabet()->GetColour(iSymbol, (pParent->m_iOffset+1)%2);

      // TODO: Exceptions / error handling in general

      CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
      pDisplayInfo->iColour = iColour;
      pDisplayInfo->bShove = true;
      pDisplayInfo->bVisible = true;
      pDisplayInfo->strDisplayText = m_pNCManager->GetAlphabet()->GetDisplayText(iSymbol);

      CAlphNode *pAlphNode;
      pNewNode = pAlphNode = makeSymbol(pParent, iLbnd, iHbnd, pDisplayInfo,iSymbol);

      //     std::stringstream ssLabel;

      //     ssLabel << m_pNCManager->GetAlphabet()->GetDisplayText(iSymbol) << ": " << pNewNode;

      //    pDisplayInfo->strDisplayText = ssLabel.str();


      pNewNode->m_iNumSymbols = 1;

      pNewNode->m_iOffset = pParent->m_iOffset + 1;

      pAlphNode->iContext = CreateSymbolContext(pParent, iSymbol);
  }

  return pNewNode;
}

CDasherNode *CAlphabetManager::CSymbolNode::RebuildSymbol(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd) {
  if(iSymbol == this->iSymbol) {
    SetRange(iLbnd, iHbnd);
    SetParent(pParent);
    DASHER_ASSERT(m_iOffset == pParent->m_iOffset + 1);
    return this;
  }
  return m_pMgr->CreateSymbolNode(pParent, iSymbol, iLbnd, iHbnd);
}

CDasherNode *CAlphabetManager::CGroupNode::RebuildSymbol(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd) {
  return m_pMgr->CreateSymbolNode(pParent, iSymbol, iLbnd, iHbnd);
}

void CAlphabetManager::IterateChildGroups(CAlphNode *pParent, SGroupInfo *pParentGroup, CAlphNode *buildAround) {
  std::vector<unsigned int> *pCProb(pParent->GetProbInfo());
  const int iMin(pParentGroup ? pParentGroup->iStart : 1);
  const int iMax(pParentGroup ? pParentGroup->iEnd : pCProb->size());
  // TODO: Think through alphabet file formats etc. to make this class easier.
  // TODO: Throw a warning if parent node already has children
  
  // Create child nodes and add them
  
  int i(iMin); //lowest index of child which we haven't yet added
  SGroupInfo *pCurrentNode(pParentGroup ? pParentGroup->pChild : m_pNCManager->GetAlphabet()->m_pBaseGroup);
  // The SGroupInfo structure has something like linked list behaviour
  // Each SGroupInfo contains a pNext, a pointer to a sibling group info
  while (i < iMax) {
    CDasherNode *pNewChild;
    bool bSymbol = !pCurrentNode //gone past last subgroup
                  || i < pCurrentNode->iStart; //not reached next subgroup
    const int iStart=i, iEnd = (bSymbol) ? i+1 : pCurrentNode->iEnd;
#ifdef WIN32
    typedef unsigned __int64 temptype;
#else
    typedef unsigned long long int temptype;
#endif
    unsigned int iLbnd = (((*pCProb)[iStart-1] - (*pCProb)[iMin-1]) *
                          (temptype)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) /
                         ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
    unsigned int iHbnd = (((*pCProb)[iEnd-1] - (*pCProb)[iMin-1]) *
                          (temptype)(m_pNCManager->GetLongParameter(LP_NORMALIZATION))) /
                         ((*pCProb)[iMax-1] - (*pCProb)[iMin-1]);
    
    if (bSymbol) {
      pNewChild = (buildAround) ? buildAround->RebuildSymbol(pParent, i, iLbnd, iHbnd) : CreateSymbolNode(pParent, i, iLbnd, iHbnd);
      i++; //make one symbol at a time - move onto next in next iteration
    } else { //in/reached subgroup - do entire group in one go:
      pNewChild= (buildAround) ? buildAround->RebuildGroup(pParent, pCurrentNode, iLbnd, iHbnd) : CreateGroupNode(pParent, pCurrentNode, iLbnd, iHbnd);
      i = pCurrentNode->iEnd; //make one group at a time - so move past entire group...
      pCurrentNode = pCurrentNode->pNext;
    }
    DASHER_ASSERT(pParent->GetChildren().back()==pNewChild);
  }

  pParent->SetFlag(NF_ALLCHILDREN, true);
}

CAlphabetManager::CAlphNode::~CAlphNode() {
  delete m_pProbInfo;
  m_pMgr->m_pLanguageModel->ReleaseContext(iContext);
}

void CAlphabetManager::CSymbolNode::Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  //std::cout << this << " " << Parent() << ": Output at offset " << m_iOffset << " *" << m_pMgr->m_pNCManager->GetAlphabet()->GetText(t) << "* " << std::endl;

  Dasher::CEditEvent oEvent(1, m_pMgr->m_pNCManager->GetAlphabet()->GetText(iSymbol), m_iOffset);
  m_pMgr->m_pNCManager->InsertEvent(&oEvent);

  // Track this symbol and its probability for logging purposes
  if (pAdded != NULL) {
    Dasher::SymbolProb sItem;
    sItem.sym    = iSymbol;
    sItem.prob   = GetProb(iNormalization);

    pAdded->push_back(sItem);
  }
}

void CAlphabetManager::CSymbolNode::Undo() {
  Dasher::CEditEvent oEvent(2, m_pMgr->m_pNCManager->GetAlphabet()->GetText(iSymbol), m_iOffset);
  m_pMgr->m_pNCManager->InsertEvent(&oEvent);
}

CDasherNode *CAlphabetManager::CGroupNode::RebuildParent() {
  // CAlphNode's always have a parent, they inserted a symbol; CGroupNode's
  // with an m_pGroup have a container i.e. the parent group, unless
  // m_pGroup==NULL => "root" node where Alphabet->m_pBaseGroup is the *first*child*...
  if (m_pGroup == NULL) return NULL;
  //offset of group node is same as parent...
  return CAlphNode::RebuildParent(m_iOffset);
}

CDasherNode *CAlphabetManager::CSymbolNode::RebuildParent() {
  //parent's offset is one less than this.
  return CAlphNode::RebuildParent(m_iOffset-1);
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
