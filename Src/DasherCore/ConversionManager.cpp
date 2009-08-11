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

CConversionManager::CConversionManager(CNodeCreationManager *pNCManager, CAlphabet *pAlphabet)
  : CNodeManager(2) {

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

CDasherNode *CConversionManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CDasherNode *pNewNode;

  int iOffset = *(static_cast<int *>(pUserData));

  // TODO: Parameters here are placeholders - need to figure out what's right

  CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
  pDisplayInfo->iColour = 9; // TODO: Hard coded value
  pDisplayInfo->bShove = true;
  pDisplayInfo->bVisible = true;
  pDisplayInfo->strDisplayText = ""; // TODO: Hard coded value, needs i18n

  pNewNode = new CDasherNode(pParent, iLower, iUpper, pDisplayInfo);

  // FIXME - handle context properly
  // TODO: Reimplemnt -----
  //  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
  // -----

  pNewNode->m_pNodeManager = this;
  pNewNode->m_pNodeManager->Ref();


  SConversionData *pNodeUserData = new SConversionData;
  pNewNode->m_pUserData = pNodeUserData;
  pNodeUserData->bisRoot = true;
  pNodeUserData->iOffset = iOffset + 1;

  pNodeUserData->pLanguageModel = NULL;

  pNodeUserData->pSCENode = 0;

  return pNewNode;
}


void CConversionManager::PopulateChildren( CDasherNode *pNode ) {
  DASHER_ASSERT(m_pNCManager);

  SConversionData * pCurrentDataNode (static_cast<SConversionData *>(pNode->m_pUserData));
  CDasherNode *pNewNode;

  // If no helper class is present then just drop straight back to an
  // alphabet root. This should only happen in error cases, and the
  // user should have been warned here.
  //
  int iLbnd(0);
  int iHbnd(m_pNCManager->GetLongParameter(LP_NORMALIZATION));


  CAlphabetManager::SRootData oRootData;
  oRootData.szContext = NULL;
  oRootData.iOffset = pCurrentDataNode->iOffset + 1;

  pNewNode = m_pNCManager->GetRoot(0, pNode, iLbnd, iHbnd, &oRootData);
  pNewNode->SetFlag(NF_SEEN, false);

  pNode->Children().push_back(pNewNode);

  return;
}

void CConversionManager::ClearNode( CDasherNode *pNode ) {
  if(pNode->m_pUserData){
    SConversionData *pUserData(static_cast<SConversionData *>(pNode->m_pUserData));

    pUserData->pLanguageModel->ReleaseContext(pUserData->iContext);
    delete (SConversionData *)(pNode->m_pUserData);
  }
}

void CConversionManager::RecursiveDumpTree(SCENode *pCurrent, unsigned int iDepth) {
  pCurrent = pCurrent->GetChild();

  if(pCurrent){
    while(pCurrent){
        std::cout << " " << pCurrent->pszConversion << " " << pCurrent->IsHeadAndCandNum << " " << pCurrent->CandIndex << " " << pCurrent->IsComplete << " " << pCurrent->AcCharCount << std::endl;
	pCurrent = pCurrent->GetNext();
    }
  }
  /*
  while(pCurrent) {
    for(unsigned int i(0); i < iDepth; ++i)
      std::cout << "-";

    std::cout << " " << pCurrent->pszConversion << " " << pCurrent->IsHeadAndCandNum << " " << pCurrent->CandIndex << " " << pCurrent->IsComplete << " " << pCurrent->AcCharCount << std::endl;

    RecursiveDumpTree(pCurrent->GetChild(), iDepth + 1);
    pCurrent = pCurrent->GetNext();
  }
  */
}

void CConversionManager::Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  // TODO: Reimplement this
  //  m_pNCManager->m_bContextSensitive = true;

  SCENode *pCurrentSCENode((static_cast<SConversionData *>(pNode->m_pUserData))->pSCENode);

  if(pCurrentSCENode){
    Dasher::CEditEvent oEvent(1, pCurrentSCENode->pszConversion, static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
    m_pNCManager->InsertEvent(&oEvent);

    if((pNode->GetChildren())[0]->m_pNodeManager != this) {
      Dasher::CEditEvent oEvent(11, "", 0);
      m_pNCManager->InsertEvent(&oEvent);
    }
  }
  else {
    if(!((static_cast<SConversionData *>(pNode->m_pUserData))->bisRoot)) {
      Dasher::CEditEvent oOPEvent(1, "|", static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oOPEvent);
    }
    else {
      Dasher::CEditEvent oOPEvent(1, ">", static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oOPEvent);
    }

    Dasher::CEditEvent oEvent(10, "", 0);
    m_pNCManager->InsertEvent(&oEvent);
  }
}

void CConversionManager::Undo( CDasherNode *pNode ) {
  SCENode *pCurrentSCENode((static_cast<SConversionData *>(pNode->m_pUserData))->pSCENode);

  if(pCurrentSCENode) {
    if(pCurrentSCENode->pszConversion && (strlen(pCurrentSCENode->pszConversion) > 0)) {
      Dasher::CEditEvent oEvent(2, pCurrentSCENode->pszConversion, static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oEvent);
    }
  }
  else {
    if(!((static_cast<SConversionData *>(pNode->m_pUserData))->bisRoot)) {
      Dasher::CEditEvent oOPEvent(2, "|", static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oOPEvent);
    }
    else {
      Dasher::CEditEvent oOPEvent(2, ">", static_cast<SConversionData *>(pNode->m_pUserData)->iOffset);
      m_pNCManager->InsertEvent(&oOPEvent);
    }
  }
}
