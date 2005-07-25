#include "ControlManager.h"
#include "DasherModel.h"
#include "DasherNode.h"

#include <vector>
#include <iostream>

using namespace Dasher;

CControlManager::CControlManager( CDasherModel *pModel, CLanguageModel *pLanguageModel  )
  : m_pModel(pModel), m_pLanguageModel(pLanguageModel) {

  // Set up the control tree

  m_pControlRoot = new CControlNode;

  m_pControlRoot->strLabel = "Control";
  m_pControlRoot->iID = 0;

  m_pControlRoot->vChildren.push_back( m_pControlRoot );
  m_pControlRoot->vChildren.push_back( NULL );

}

CDasherNode *CControlManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper) {
  CDasherNode *pNewNode;

  // FIXME - is the language model pointer used?
  

 pNewNode = new CDasherNode(*m_pModel, pParent, m_pModel->GetControlSymbol(), 0, Opts::Nodes1, iLower, iUpper, m_pLanguageModel, false, 7);
 
  // FIXME - handle context properly

  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());

  pNewNode->m_pNodeManager = this;
  pNewNode->m_pUserData = m_pControlRoot;

  return pNewNode;
}

void CControlManager::PopulateChildren( CDasherNode *pNode ) {
   CDasherNode *pNewNode;

   CControlNode *pControlNode(static_cast<CControlNode *>(pNode->m_pUserData));

   int iNChildren( pControlNode->vChildren.size() );

   int iIdx(0);

   for(std::vector<CControlNode *>::iterator it(pControlNode->vChildren.begin()); it != pControlNode->vChildren.end(); ++it) {

     // FIXME - could do this better

     int iLbnd( iIdx*(m_pModel->GetLongParameter(LP_NORMALIZATION)/iNChildren)); 
     int iHbnd( (iIdx+1)*(m_pModel->GetLongParameter(LP_NORMALIZATION)/iNChildren)); 

     if( *it == NULL ) {
       // Escape back to alphabet
       pNewNode = m_pModel->GetRoot(0, pNode, iLbnd, iHbnd);
     }
     else {
       pNewNode = new CDasherNode(*m_pModel, pNode, m_pModel->GetControlSymbol(), 0, Opts::Nodes1, iLbnd, iHbnd, m_pLanguageModel, false, 7);
       pNewNode->m_pNodeManager = this;
       pNewNode->m_pUserData = *it;
     }
     pNode->Children()[iIdx] = pNewNode;
     ++iIdx;
   }
}

void CControlManager::ClearNode( CDasherNode *pNode ) {
  // Should this be responsible for actually doing the deletion
}

void CControlManager::Output( CDasherNode *pNode ) {
  CControlNode *pControlNode(static_cast<CControlNode *>(pNode->m_pUserData));

  std::cout << "Control mode output (" << pControlNode->iID << ")" << std::endl;
};

void CControlManager::Undo( CDasherNode *pNode ) {
  // Do we ever need this?
  // One other thing we probably want is notification when we leave a node - that way we can eg speed up again if we slowed down
};
