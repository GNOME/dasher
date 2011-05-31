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

CDasherNode *CConvertingAlphMgr::CreateSymbolNode(CAlphNode *pParent, unsigned int iLbnd, unsigned int iHbnd, const string &strGroup, int iBkgCol, symbol iSymbol) {
  int i=m_pAlphabet->GetNumberTextSymbols()+1;
  if (iSymbol == i) {
    vector<unsigned int> *pCProb(pParent->GetProbInfo());
    DASHER_ASSERT(pCProb->size() == m_pAlphabet->GetNumberTextSymbols()+2);//initial 0, final conversion prob
    //ACL setting m_iOffset+1 for consistency with "proper" symbol nodes...
    return m_pConvMgr->GetRoot(pParent, (*pCProb)[i-1], (*pCProb)[i], pParent->offset()+1);
  } else {
    return CAlphabetManager::CreateSymbolNode(pParent, iLbnd, iHbnd, strGroup, iBkgCol, iSymbol);
  }
}
