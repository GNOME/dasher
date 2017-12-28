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

CConvertingAlphMgr::CConvertingAlphMgr(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CConversionManager *pConvMgr, const CAlphInfo *pAlphabet)
 : CAlphabetManager(pCreateFrom, pInterface, pNCManager, pAlphabet), m_pConvMgr(pConvMgr) {
 }

void CConvertingAlphMgr::MakeLabels(CDasherScreen *pScreen) {
  CAlphabetManager::MakeLabels(pScreen);
  m_pConvMgr->ChangeScreen(pScreen);
}

CConvertingAlphMgr::~CConvertingAlphMgr() {
}

CDasherNode *CConvertingAlphMgr::CreateSymbolNode(CAlphNode *pParent, symbol iSymbol) {
  //int i=m_pAlphabet->iEnd;
  if (iSymbol == m_pAlphabet->iEnd) {
    vector<unsigned int> *pCProb(pParent->GetProbInfo());
    DASHER_ASSERT(pCProb->size() == m_pAlphabet->iEnd+1);//initial 0, final conversion prob

    //this used to be the "CloneAlphContext" method. Why it uses the
    // ConversionManager's LM to clone a context from an Alphabet Node,
    // I don't know - not sure how LanguageModelling WRT conversion
    // is supposed to work...
    CLanguageModel::Context iContext = (pParent->iContext)
      ? m_pConvMgr->m_pLanguageModel->CloneContext(pParent->iContext)
      : m_pConvMgr->m_pLanguageModel->CreateEmptyContext(); 

    //ACL setting m_iOffset+1 for consistency with "proper" symbol nodes...
    return m_pConvMgr->GetRoot(pParent->offset()+1, iContext);
    //Note: previous code used (*pCProb)[i-1] and (*pCProb)[i] instead of
    // iLbnd and iHbnd passed in; presumably these must have been the same?
  } else {
    return CAlphabetManager::CreateSymbolNode(pParent, iSymbol);
  }
}
