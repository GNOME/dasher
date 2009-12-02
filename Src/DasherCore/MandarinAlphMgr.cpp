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

CAlphabetManager::CAlphNode *CMandarinAlphMgr::GetRoot(CDasherNode *pParent, int iLower, int iUpper, bool bEnteredLast, int iOffset) {
  CAlphNode *pNewNode;
  if (!bEnteredLast) {
    //(probably) escaping back to alphabet after conversion. At any rate, we'll duplicate a lot of
    // superclass node-construction code, to avoid building a context - as we're going to throw that away
    // (and the context-building code often doesn't work with conversion!)...
    CDasherNode::SDisplayInfo *pDispInfo = new CDasherNode::SDisplayInfo;
    pDispInfo->bShove = true;
    pDispInfo->bVisible = true;
    pDispInfo->strDisplayText = "";
    pDispInfo->iColour = m_pNCManager->GetAlphabet()->GetColour(0, 0);
    pNewNode = makeGroup(pParent, iLower, iUpper, pDispInfo, NULL);
    pNewNode->m_iOffset=max(0,iOffset)-1;
  } else {
    //probably rebuilding parent; call standard GetRoot, which'll extract the most recent symbol
    // (entered by the node (equivalent to that) which we are rebuilding)
    pNewNode = CAlphabetManager::GetRoot(pParent, iLower, iUpper, bEnteredLast, iOffset);
  }

  //Override context for Mandarin Dasher
  if (pParent && pParent->mgrId() == 2) {
    //ACL MandarinAlphMgr should always be used with a PinYinConversionHelper,
    //so this cast really should be ok :-)....but I still wish for a guarantee!
    CPinYinConversionHelper::CPYConvNode *pPYParent = static_cast<CPinYinConversionHelper::CPYConvNode *>(pParent);
    pNewNode->iContext = m_pLanguageModel->CloneContext(pPYParent->GetConvContext());
  } else
    pNewNode->iContext = m_pLanguageModel->CreateEmptyContext();

  return pNewNode;
}

CDasherNode *CMandarinAlphMgr::CreateSymbolNode(CAlphNode *pParent, symbol iSymbol, unsigned int iLbnd, unsigned int iHbnd) {

  if (iSymbol <= 1288) {
    //Will wrote:
    //Modified for Mandarin Dasher
    //The following logic switch allows punctuation nodes in Mandarin to be treated in the same way as English (i.e. display and populate next round) instead of invoking a conversion node
    //ACL I think by "the following logic switch" he meant that symbols <= 1288 are "normal" nodes, NOT punctuation nodes,
    // whereas punctuation is handled by the fallthrough case (standard AlphabetManager CreateSymbolNode)
	  CDasherNode *pNewNode = m_pNCManager->GetConvRoot(pParent, iLbnd, iHbnd, pParent->m_iOffset+1);
	  static_cast<CPinYinConversionHelper::CPYConvNode *>(pNewNode)->SetConvSymbol(iSymbol);
	  return pNewNode;
  }
  return CAlphabetManager::CreateSymbolNode(pParent, iSymbol, iLbnd, iHbnd);
}

CLanguageModel::Context CMandarinAlphMgr::CreateSymbolContext(CAlphNode *pParent, symbol iSymbol)
{
	//Context carry-over. This code may worth looking at debug
	return m_pLanguageModel->CloneContext(pParent->iContext);
}

CMandarinAlphMgr::CMandNode::CMandNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CAlphabetManager *pMgr, symbol iSymbol)
: CSymbolNode(pParent, iLbnd, iHbnd, pDispInfo, pMgr, iSymbol) {
}

CAlphabetManager::CSymbolNode *CMandarinAlphMgr::makeSymbol(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, symbol iSymbol) {
  return new CMandNode(pParent, iLbnd, iHbnd, pDispInfo, this, iSymbol);
}

void CMandarinAlphMgr::CMandNode::SetFlag(int iFlag, bool bValue) {
  //disable learn-as-you-write for Mandarin Dasher
   if (iFlag==NF_COMMITTED)
     CDasherNode::SetFlag(iFlag, bValue); //bypass CAlphNode setter!
  else
      CAlphNode::SetFlag(iFlag, bValue);
}
