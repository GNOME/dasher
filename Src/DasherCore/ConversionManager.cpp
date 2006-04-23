#include "ConversionManager.h"

#include <string>
#include <vector>

using namespace Dasher;

CConversionManager::CConversionManager(CDasherModel *pModel, CLanguageModel *pLanguageModel) {
  m_pModel = pModel;
  m_pLanguageModel = pLanguageModel;

  // Assume that this is sorted:
  std::vector<std::string> vCandidateList;

  vCandidateList.push_back("hello");
  vCandidateList.push_back("foo");
  vCandidateList.push_back("fish");

  // TODO: Delete list when we're done
  
  m_pRoot = new CConversionManagerNode;

  m_pRoot->m_strSymbol = "Convert";
  m_pRoot->m_pChild = 0;
  m_pRoot->m_pNext = 0;
  m_pRoot->m_iNumChildren = 0;

  for(std::vector<std::string>::iterator it(vCandidateList.begin()); it != vCandidateList.end(); ++it) {
    CConversionManagerNode *pCurrentNode(m_pRoot);

    // TODO: Need to do unicode here!


    for(int j(0); j < it->size(); ++j) {
      CConversionManagerNode *pNextNode;

      pNextNode = pCurrentNode->FindChild(it->substr(j,1));

      if(!pNextNode) {
	pNextNode = new CConversionManagerNode;

	pNextNode->m_strSymbol = it->substr(j,1);
	pNextNode->m_pChild = 0;
	pNextNode->m_pNext = pCurrentNode->m_pChild;
	pNextNode->m_iNumChildren = 0;
	pCurrentNode->m_pChild = pNextNode;
	++pCurrentNode->m_iNumChildren;
      }

      pCurrentNode = pNextNode;
    }
  }
}

// TODO: Actually need ref counting this time around - and need to check that it's properly implemented elsewhere
void CConversionManager::Ref() {
}

void CConversionManager::Unref() {
}

CDasherNode *CConversionManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CDasherNode *pNewNode;

  // TODO: Parameters here are placeholders - need to figure out what's right
  pNewNode = new CDasherNode(pParent, m_pModel->GetStartConversionSymbol(), 0, Opts::Nodes2, iLower, iUpper, m_pLanguageModel, 2);
 
  // FIXME - handle context properly
  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());

  pNewNode->m_pNodeManager = this;
  pNewNode->m_pUserData = m_pRoot;
  pNewNode->m_strDisplayText = m_pRoot->m_strSymbol;
  pNewNode->m_bShove = false;
  pNewNode->m_pBaseGroup = 0;

  return pNewNode;
}

void CConversionManager::PopulateChildren( CDasherNode *pNode ) {

  CDasherNode *pNewNode;

  CConversionManagerNode *pCurrentCMNode(static_cast<CConversionManagerNode *>(pNode->m_pUserData));
  CConversionManagerNode *pCurrentCMChild(pCurrentCMNode->m_pChild);


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
      pNewNode->m_pUserData = m_pRoot;
      pNewNode->m_strDisplayText = pCurrentCMChild->m_strSymbol;
      pNewNode->m_bShove = false;
      pNewNode->m_pBaseGroup = 0;
      
      pNode->Children().push_back(pNewNode);

      pCurrentCMChild = pCurrentCMChild->m_pNext;
      ++iIdx;
    }
  }
  else {
    // TODO: Placeholder algorithm here
    for(int i(0); i < 2; ++i) {
      int iLbnd( i*(m_pModel->GetLongParameter(LP_NORMALIZATION)/2)); 
      int iHbnd( (i+1)*(m_pModel->GetLongParameter(LP_NORMALIZATION)/2)); 
      
      pNewNode = m_pModel->GetRoot(0, pNode, iLbnd, iHbnd, NULL);
      pNewNode->Seen(false);
      
      pNode->Children().push_back(pNewNode);
    }
  }
}

void CConversionManager::ClearNode( CDasherNode *pNode ) {
}
