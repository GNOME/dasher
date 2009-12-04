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
#include "PinYinConversionHelper.h"
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

CMandarinAlphMgr::CMandarinAlphMgr(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CLanguageModel *pLanguageModel)
  : CAlphabetManager(pInterface, pNCManager, pLanguageModel) {
}

CAlphabetManager::CAlphNode *CMandarinAlphMgr::GetRoot(CDasherNode *pParent, int iLower, int iUpper, char *szContext, int iOffset) {

  CAlphNode *pNewNode = CAlphabetManager::GetRoot(pParent, iLower, iUpper, szContext, iOffset);

  //Override context for Mandarin Dasher
  if (pParent){
    CPinYinConversionHelper::CPYConvNode *pPYParent = static_cast<CPinYinConversionHelper::CPYConvNode *>(pParent);
    //ACL think this is how this Mandarin thing works here...
    // but would be nice if I could ASSERT that that cast is ok!
    pNewNode->iContext = m_pLanguageModel->CloneContext(pPYParent->GetConvContext());
  }
  else
	pNewNode->iContext = m_pLanguageModel->CreateEmptyContext();

  return pNewNode;
}

CDasherNode *CMandarinAlphMgr::CreateSymbolNode(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd, symbol iExistingSymbol, CDasherNode *pExistingChild) {

  if (iSymbol <= 1288) {
    //Modified for Mandarin Dasher
    //The following logic switch allows punctuation nodes in Mandarin to be treated in the same way as English (i.e. display and populate next round) instead of invoking a conversion node
	  CDasherNode *pNewNode = m_pNCManager->GetConvRoot(pParent, iLbnd, iHbnd, pParent->m_iOffset);
	  static_cast<CPinYinConversionHelper::CPYConvNode *>(pNewNode)->SetConvSymbol(iSymbol);
	  return pNewNode;
  }
  return CAlphabetManager::CreateSymbolNode(pParent, iSymbol, iLbnd, iHbnd, iExistingSymbol, pExistingChild);
}

CLanguageModel::Context CMandarinAlphMgr::CreateSymbolContext(CAlphNode *pParent, symbol iSymbol)
{
	//Context carry-over. This code may worth looking at debug
	return m_pLanguageModel->CloneContext(pParent->iContext);
}

CMandarinAlphMgr::CMandNode::CMandNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CAlphabetManager *pMgr)
: CAlphNode(pParent, iLbnd, iHbnd, pDispInfo, pMgr) {
}

CAlphabetManager::CAlphNode *CMandarinAlphMgr::makeNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo) {
  return new CMandNode(pParent, iLbnd, iHbnd, pDispInfo, this);
}

void CMandarinAlphMgr::CMandNode::SetFlag(int iFlag, bool bValue) {
  //disable learn-as-you-write for Mandarin Dasher
   if (iFlag==NF_COMMITTED)
     CDasherNode::SetFlag(iFlag, bValue); //bypass CAlphNode setter!
  else
      CAlphNode::SetFlag(iFlag, bValue);
}
