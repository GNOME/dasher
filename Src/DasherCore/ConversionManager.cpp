#ifndef WIN32
#include "config.h"
#endif 

#ifdef JAPANESE

#include "ConversionManager.h"
#include "Event.h"
#include "EventHandler.h"

#include <iostream>
#include <string>
#include <vector>

using namespace Dasher;

CConversionManager::CConversionManager(CDasherModel *pModel, CLanguageModel *pLanguageModel, CConversionHelper *pHelper) 
  : CNodeManager(2) {
  m_pModel = pModel;
  m_pLanguageModel = pLanguageModel;
  m_pHelper = pHelper;

  m_iRefCount = 1;

  m_bTreeBuilt = false;
}

CDasherNode *CConversionManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CDasherNode *pNewNode;

  // TODO: Parameters here are placeholders - need to figure out what's right
  pNewNode = new CDasherNode(pParent, m_pModel->GetStartConversionSymbol(), 0, Opts::Nodes2, iLower, iUpper, m_pLanguageModel, 2);
 
  // FIXME - handle context properly
  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());

  pNewNode->m_pNodeManager = this;
  pNewNode->m_pNodeManager->Ref();

  pNewNode->m_pUserData = 0;
  pNewNode->m_strDisplayText = "Convert";
  pNewNode->m_bShove = false;
  pNewNode->m_pBaseGroup = 0;

  return pNewNode;
}

void CConversionManager::PopulateChildren( CDasherNode *pNode ) {

  if(!m_bTreeBuilt) {
    BuildTree(pNode);
    m_bTreeBuilt = true;
  }

  CDasherNode *pNewNode;

  CConversionManagerNode *pCurrentCMNode(static_cast<CConversionManagerNode *>(pNode->m_pUserData));
  
  if(pCurrentCMNode == 0)
    if(m_pRoot)
      pCurrentCMNode = m_pRoot[0];
    else
      pCurrentCMNode = 0;
  else if((pCurrentCMNode->m_pChild == 0) && (pCurrentCMNode->m_iPhrase < m_iRootCount - 1))
    pCurrentCMNode = m_pRoot[pCurrentCMNode->m_iPhrase + 1];

  CConversionManagerNode *pCurrentCMChild;

  if(pCurrentCMNode)
    pCurrentCMChild = pCurrentCMNode->m_pChild;
  else
    pCurrentCMChild = 0;

  if(pCurrentCMChild) {
    int iIdx(0);

    while(pCurrentCMChild) {
      int iLbnd( iIdx*(m_pModel->GetLongParameter(LP_NORMALIZATION)/pCurrentCMNode->m_iNumChildren)); 
      int iHbnd( (iIdx+1)*(m_pModel->GetLongParameter(LP_NORMALIZATION)/pCurrentCMNode->m_iNumChildren)); 

      // TODO: Parameters here are placeholders - need to figure out what's right
      pNewNode = new CDasherNode(pNode, m_pModel->GetStartConversionSymbol(), 0, Opts::Nodes2, iLbnd, iHbnd, m_pLanguageModel, 1);
      
      // FIXME - handle context properly
      pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());
      
      pNewNode->m_pNodeManager = this;
      pNewNode->m_pNodeManager->Ref();

      pNewNode->m_pUserData = pCurrentCMChild;
      pNewNode->m_strDisplayText = pCurrentCMChild->m_strSymbol;
      pNewNode->m_bShove = true;
      pNewNode->m_pBaseGroup = 0;
      
      pNode->Children().push_back(pNewNode);

      pCurrentCMChild = pCurrentCMChild->m_pNext;
      ++iIdx;
    }
  }
  else {
    // TODO: Placeholder algorithm here
    // TODO: Add an 'end of conversion' node?
    int iLbnd(0);
    int iHbnd(m_pModel->GetLongParameter(LP_NORMALIZATION)); 
      
    pNewNode = m_pModel->GetRoot(0, pNode, iLbnd, iHbnd, NULL);
    pNewNode->Seen(false);
      
    pNode->Children().push_back(pNewNode);
  }
}

void CConversionManager::ClearNode( CDasherNode *pNode ) {
  // TODO: Need to implement this
  
  pNode->m_pNodeManager->Unref();
}

void CConversionManager::BuildTree(CDasherNode *pRoot) {
  CDasherNode *pCurrentNode(pRoot->Parent());

  std::string strCurrentString;
  
  while(pCurrentNode) {
    if(pCurrentNode->m_pNodeManager->GetID() == 2)
      break;

    // TODO: Need to make this the edit text rather than the display text
    strCurrentString = pCurrentNode->m_strDisplayText + strCurrentString;
    pCurrentNode = pCurrentNode->Parent();
  }

  // Assume that this is sorted:
  std::vector<std::vector<std::string> > vCandidateList;
  m_pHelper->Convert(strCurrentString, vCandidateList);

  m_iRootCount = vCandidateList.size();

  if(m_iRootCount == 0)
    m_pRoot = 0;
  else {

  m_pRoot = new CConversionManagerNode *[m_iRootCount];

  for(int i(0); i < m_iRootCount; ++i) {
    m_pRoot[i] = new CConversionManagerNode;
    
    m_pRoot[i]->m_strSymbol = "Convert";
    m_pRoot[i]->m_pChild = 0;
    m_pRoot[i]->m_pNext = 0;
    m_pRoot[i]->m_iNumChildren = 0;
    m_pRoot[i]->m_iPhrase = i;


    for(std::vector<std::string>::iterator it(vCandidateList[i].begin()); it != vCandidateList[i].end(); ++it) {
      CConversionManagerNode *pCurrentNode(m_pRoot[i]);
      
      int iIdx(0);
      
      // TODO: Need phrase-based conversion
      while(iIdx < it->size()) {
	
	int iLength;
	
	// TODO: Really dodgy UTF-8 parser - find a library routine to do this
	if((static_cast<int>((*it)[iIdx]) & 0x80) == 0)
	  iLength = 1;
	else if((static_cast<int>((*it)[iIdx]) & 0xE0) == 0xC0) 
	  iLength = 2;
	else if((static_cast<int>((*it)[iIdx]) & 0xF0) == 0xE0)
	  iLength = 3;
	else if((static_cast<int>((*it)[iIdx]) & 0xF8) == 0xF0)
	  iLength = 4;
	else if((static_cast<int>((*it)[iIdx]) & 0xFC) == 0xF8)
	  iLength = 5;
	else
	  iLength = 6;
	
	CConversionManagerNode *pNextNode;
	
	pNextNode = pCurrentNode->FindChild(it->substr(iIdx, iLength));
	
	if(!pNextNode) {
	  pNextNode = new CConversionManagerNode;
	  
	  pNextNode->m_strSymbol = it->substr(iIdx, iLength);
	  pNextNode->m_pChild = 0;
	  pNextNode->m_pNext = pCurrentNode->m_pChild;
	  pNextNode->m_iNumChildren = 0;
	  pNextNode->m_iPhrase = pCurrentNode->m_iPhrase;
	  pCurrentNode->m_pChild = pNextNode;
	  ++pCurrentNode->m_iNumChildren;
	}
	
	pCurrentNode = pNextNode;
	iIdx += iLength;
      }
    }
  }
  }
}

void CConversionManager::Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {
  m_pModel->m_bContextSensitive = true; 

  CConversionManagerNode *pCurrentCMNode(static_cast<CConversionManagerNode *>(pNode->m_pUserData));

  if(pCurrentCMNode) {
    Dasher::CEditEvent oEvent(1, pCurrentCMNode->m_strSymbol);
    m_pModel->InsertEvent(&oEvent);

    if((pNode->GetChildren())[0]->m_pNodeManager != this) {
      Dasher::CEditEvent oEvent(11, "");
      m_pModel->InsertEvent(&oEvent);
    }
  }
  else {
    Dasher::CEditEvent oEvent(10, "");
    m_pModel->InsertEvent(&oEvent);
  }
}

void CConversionManager::Undo( CDasherNode *pNode ) {
  CConversionManagerNode *pCurrentCMNode(static_cast<CConversionManagerNode *>(pNode->m_pUserData));

  if(pCurrentCMNode) {
    if(pCurrentCMNode->m_strSymbol.size() > 0) {
      Dasher::CEditEvent oEvent(2, pCurrentCMNode->m_strSymbol);
      m_pModel->InsertEvent(&oEvent);
    }
  }
}

#endif
