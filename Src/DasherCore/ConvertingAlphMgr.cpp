/*
 *  ConvertingAlphMgr.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 06/08/2010.
 *  Copyright 2010 Cavendish Laboratory. All rights reserved.
 *
 */

#include "ConvertingAlphMgr.h"
#include "NodeCreationManager.h"

using namespace Dasher;

CConvertingAlphMgr::CConvertingAlphMgr(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CConversionManager *pConvMgr, const CAlphInfo *pAlphabet, const CAlphabetMap *pAlphabetMap)
 : CAlphabetManager(pInterface, pNCManager, pAlphabet, pAlphabetMap), m_pConvMgr(pConvMgr) {
 }

CConvertingAlphMgr::~CConvertingAlphMgr() {
  m_pConvMgr->Unref();
}

void CConvertingAlphMgr::AddExtras(CAlphNode *pParent, std::vector<unsigned int> *pCProb) {
    //should have another probability....
  const unsigned int i(m_pNCManager->GetAlphabet()->GetNumberTextSymbols()+1);
  DASHER_ASSERT(pCProb->size() == i+1);
  //ACL setting m_iOffset+1 for consistency with "proper" symbol nodes...
  m_pConvMgr->GetRoot(pParent, (*pCProb)[i-1], (*pCProb)[i], pParent->offset()+1);
  CAlphabetManager::AddExtras(pParent, pCProb);
}
