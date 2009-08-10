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

CMandarinAlphMgr::CMandarinAlphMgr(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel, CLanguageModel::Context iLearnContext) 
  : CAlphabetManager(pInterface, pNCManager, pLanguageModel, iLearnContext) {
}

CDasherNode *CMandarinAlphMgr::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {

  CDasherNode *pNewNode = CAlphabetManager::GetRoot(pParent, iLower, iUpper, pUserData);
  SAlphabetData *pNodeUserData = static_cast<SAlphabetData *>(pNewNode->m_pUserData);

  //Override context for Mandarin Dasher
  if (pParent){
    CConversionManager::SConversionData *pParentConversionData = static_cast<CConversionManager::SConversionData *>(pParent->m_pUserData);
    pNodeUserData->iContext = m_pLanguageModel->CloneContext(pParentConversionData->iContext);
  }
  else 
	pNodeUserData->iContext = m_pLanguageModel->CreateEmptyContext();

  return pNewNode;
}

CDasherNode *CMandarinAlphMgr::CreateSymbolNode(CDasherNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd, symbol iExistingSymbol, CDasherNode *pExistingChild) {

  if (iSymbol <= 1288) {
	SAlphabetData *pParentData = static_cast<SAlphabetData *>(pParent->m_pUserData);	
	  
    //Modified for Mandarin Dasher
    //The following logic switch allows punctuation nodes in Mandarin to be treated in the same way as English (i.e. display and populate next round) instead of invoking a conversion node
	  CDasherNode *pNewNode = m_pNCManager->GetRoot(2, pParent, iLbnd, iHbnd, &(pParentData->iOffset));
	  static_cast<SAlphabetData *>(pNewNode->m_pUserData)->iSymbol = iSymbol;
	  return pNewNode;
  }
  return CAlphabetManager::CreateSymbolNode(pParent, iSymbol, iLbnd, iHbnd, iExistingSymbol, pExistingChild);
}

CLanguageModel::Context CMandarinAlphMgr::CreateSymbolContext(SAlphabetData *pParentData, symbol iSymbol)
{
	//Context carry-over. This code may worth looking at debug      
	return m_pLanguageModel->CloneContext(pParentData->iContext);
}

void CMandarinAlphMgr::SetFlag(CDasherNode *pNode, int iFlag, bool bValue) {
  //disable learn-as-you-write for Mandarin Dasher
  if (iFlag!=NF_COMMITTED) CAlphabetManager::SetFlag(pNode, iFlag, bValue);
}