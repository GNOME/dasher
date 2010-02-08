// ConversionManager.cpp
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ConversionManager.h"
#include "Event.h"
#include "EventHandler.h"
#include "NodeCreationManager.h"
#include "DasherModel.h"

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <stdlib.h>

//Note the new implementation in Mandarin Dasher may not be compatible with the previous implementation of Japanese Dasher
//Need to reconcile (a small project)

using namespace Dasher;

CConversionManager::CConversionManager(CNodeCreationManager *pNCManager, CAlphabet *pAlphabet) {

  m_pNCManager = pNCManager;
  m_pAlphabet = pAlphabet;

  m_iRefCount = 1;

  //Testing for alphabet details, delete if needed:
  /*
  int alphSize = pNCManager->GetAlphabet()->GetNumberSymbols();
  std::cout<<"Alphabet size: "<<alphSize<<std::endl;
  for(int i =0; i<alphSize; i++)
    std::cout<<"symbol: "<<i<<"    display text:"<<pNCManager->GetAlphabet()->GetDisplayText(i)<<std::endl;
  */
}

CConversionManager::CConvNode *CConversionManager::makeNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo) {
  return new CConvNode(pParent, iLbnd, iHbnd, pDispInfo, this);
}

CConversionManager::CConvNode *CConversionManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, int iOffset) {

  // TODO: Parameters here are placeholders - need to figure out what's right

  CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
  pDisplayInfo->iColour = 9; // TODO: Hard coded value
  pDisplayInfo->bShove = true;
  pDisplayInfo->bVisible = true;
  pDisplayInfo->strDisplayText = ""; // TODO: Hard coded value, needs i18n

  CConvNode *pNewNode = makeNode(pParent, iLower, iUpper, pDisplayInfo);

  // FIXME - handle context properly
  // TODO: Reimplemnt -----
  //  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
  // -----


  pNewNode->bisRoot = true;
  pNewNode->m_iOffset = iOffset;

  pNewNode->pLanguageModel = NULL;

  pNewNode->pSCENode = 0;

  return pNewNode;
}

CConversionManager::CConvNode::CConvNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CConversionManager *pMgr)
 : CDasherNode(pParent, iLbnd, iHbnd, pDispInfo), m_pMgr(pMgr) {
  pMgr->m_iRefCount++;
}

void CConversionManager::CConvNode::PopulateChildren() {
  DASHER_ASSERT(m_pMgr->m_pNCManager);

  // If no helper class is present then just drop straight back to an
  // alphabet root. This should only happen in error cases, and the
  // user should have been warned here.
  //
  int iLbnd(0);
  int iHbnd(m_pMgr->m_pNCManager->GetLongParameter(LP_NORMALIZATION));

  CDasherNode *pNewNode = m_pMgr->m_pNCManager->GetAlphRoot(this, iLbnd, iHbnd, false, m_iOffset + 1);

  DASHER_ASSERT(GetChildren().back()==pNewNode);
}
int CConversionManager::CConvNode::ExpectedNumChildren() {
  return 1; //the alphabet root
}

CConversionManager::CConvNode::~CConvNode() {
  pLanguageModel->ReleaseContext(iContext);
  m_pMgr->Unref();
}

void CConversionManager::RecursiveDumpTree(SCENode *pCurrent, unsigned int iDepth) {
  const std::vector<SCENode *> &children = pCurrent->GetChildren();
  for (std::vector<SCENode *>::const_iterator it = children.begin(); it!=children.end(); it++) {
    SCENode *pCurrent(*it);
    for(unsigned int i(0); i < iDepth; ++i)
      std::cout << "-";
    std::cout << " " << pCurrent->pszConversion << " " << pCurrent->IsHeadAndCandNum << " " << pCurrent->CandIndex << " " << pCurrent->IsComplete << " " << pCurrent->AcCharCount << std::endl;
    RecursiveDumpTree(pCurrent, iDepth + 1);
  }
}

void CConversionManager::CConvNode::GetContext(CDasherInterfaceBase *pInterface, std::vector<symbol> &vContextSymbols, int iOffset, int iLength) {
  if (!GetFlag(NF_SEEN) && iOffset+iLength-1 == m_iOffset) {
    //ACL I'm extrapolating from PinYinConversionHelper (in which root nodes have their
    // Symbol set by SetConvSymbol, and child nodes are created in PopulateChildren
    // from SCENode's with Symbols having been set in in AssignSizes); not really sure
    // whether this is applicable in the general case(! - but although I think it's right
    // for PinYin, it wouldn't actually be used there, as MandarinDasher overrides contexts
    // everywhere!)
    DASHER_ASSERT(bisRoot || pSCENode);
    if (bisRoot || pSCENode->Symbol!=-1) {
      if (iLength>1) Parent()->GetContext(pInterface, vContextSymbols, iOffset, iLength-1);
      vContextSymbols.push_back(bisRoot ? iSymbol : pSCENode->Symbol);
      return;
    } //else, non-root with pSCENode->Symbol==-1 => fallthrough back to superclass code
  }
  CDasherNode::GetContext(pInterface, vContextSymbols, iOffset, iLength);
}

void CConversionManager::CConvNode::Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  // TODO: Reimplement this
  //  m_pNCManager->m_bContextSensitive = true;

  SCENode *pCurrentSCENode(pSCENode);

  if(pCurrentSCENode){
    Dasher::CEditEvent oEvent(1, pCurrentSCENode->pszConversion, m_iOffset);
    m_pMgr->m_pNCManager->InsertEvent(&oEvent);

    if((GetChildren())[0]->mgrId() == 2) {
      if (static_cast<CConvNode *>(GetChildren()[0])->m_pMgr == m_pMgr) {
        Dasher::CEditEvent oEvent(11, "", 0);
        m_pMgr->m_pNCManager->InsertEvent(&oEvent);
      }
    }
  }
  else {
    if(!bisRoot) {
      Dasher::CEditEvent oOPEvent(1, "|", m_iOffset);
      m_pMgr->m_pNCManager->InsertEvent(&oOPEvent);
    }
    else {
      Dasher::CEditEvent oOPEvent(1, ">", m_iOffset);
      m_pMgr->m_pNCManager->InsertEvent(&oOPEvent);
    }

    Dasher::CEditEvent oEvent(10, "", 0);
    m_pMgr->m_pNCManager->InsertEvent(&oEvent);
  }
}

void CConversionManager::CConvNode::Undo() {  
  SCENode *pCurrentSCENode(pSCENode);

  if(pCurrentSCENode) {
    if(pCurrentSCENode->pszConversion && (strlen(pCurrentSCENode->pszConversion) > 0)) {
      Dasher::CEditEvent oEvent(2, pCurrentSCENode->pszConversion, m_iOffset);
      m_pMgr->m_pNCManager->InsertEvent(&oEvent);
    }
  }
  else {
    if(!bisRoot) {
      Dasher::CEditEvent oOPEvent(2, "|", m_iOffset);
      m_pMgr->m_pNCManager->InsertEvent(&oOPEvent);
    }
    else {
      Dasher::CEditEvent oOPEvent(2, ">", m_iOffset);
      m_pMgr->m_pNCManager->InsertEvent(&oOPEvent);
    }
  }
}
