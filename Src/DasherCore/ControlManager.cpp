#include "ControlManager.h"
#include "DasherModel.h"
#include "DasherNode.h"
#include "Event.h"

#include <vector>
#include <iostream>

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CControlManager::CControlManager( CDasherModel *pModel, CLanguageModel *pLanguageModel  )
  : m_pModel(pModel), m_pLanguageModel(pLanguageModel) {

  // FIXME - Unicode names?

  RegisterNode(CTL_ROOT, "Control", 8);
  RegisterNode(CTL_STOP, "Stop", 242);
  RegisterNode(CTL_PAUSE, "Pause", 241);
  RegisterNode(CTL_MOVE, "Move", -1);
  RegisterNode(CTL_MOVE_FORWARD, "Forward", -1);
  RegisterNode(CTL_MOVE_FORWARD_CHAR, ">", -1);
  RegisterNode(CTL_MOVE_FORWARD_WORD, ">>", -1);
  RegisterNode(CTL_MOVE_FORWARD_LINE, ">>>", -1);
  RegisterNode(CTL_MOVE_FORWARD_FILE, ">>>>", -1);
  RegisterNode(CTL_MOVE_BACKWARD, "Backward", -1);
  RegisterNode(CTL_MOVE_BACKWARD_CHAR, "<", -1);
  RegisterNode(CTL_MOVE_BACKWARD_WORD, "<<", -1);
  RegisterNode(CTL_MOVE_BACKWARD_LINE, "<<<", -1);
  RegisterNode(CTL_MOVE_BACKWARD_FILE, "<<<<", -1);
  RegisterNode(CTL_DELETE, "Delete", -1);
  RegisterNode(CTL_DELETE_FORWARD, "Forward", -1);
  RegisterNode(CTL_DELETE_FORWARD_CHAR, ">", -1);
  RegisterNode(CTL_DELETE_FORWARD_WORD, ">>", -1);
  RegisterNode(CTL_DELETE_FORWARD_LINE, ">>>", -1);
  RegisterNode(CTL_DELETE_FORWARD_FILE, ">>>>", -1);
  RegisterNode(CTL_DELETE_BACKWARD, "Backward", -1);
  RegisterNode(CTL_DELETE_BACKWARD_CHAR, "<", -1);
  RegisterNode(CTL_DELETE_BACKWARD_WORD, "<<", -1);
  RegisterNode(CTL_DELETE_BACKWARD_LINE, "<<<", -1);
  RegisterNode(CTL_DELETE_BACKWARD_FILE, "<<<<", -1);
  
  ConnectNode(-1, CTL_ROOT, -2);
  ConnectNode(CTL_STOP, CTL_ROOT, -2);
  ConnectNode(CTL_PAUSE, CTL_ROOT, -2);
  ConnectNode(CTL_MOVE, CTL_ROOT, -2);
  ConnectNode(CTL_DELETE, CTL_ROOT, -2);

  ConnectNode(-1, CTL_STOP, -2);
  ConnectNode(CTL_ROOT, CTL_STOP, -2);
  
  ConnectNode(-1, CTL_PAUSE, -2);
  ConnectNode(CTL_ROOT, CTL_PAUSE, -2);

  ConnectNode(CTL_MOVE_FORWARD, CTL_MOVE, -2);
  ConnectNode(CTL_MOVE_BACKWARD, CTL_MOVE, -2);

  ConnectNode(CTL_MOVE_FORWARD_CHAR, CTL_MOVE_FORWARD, -2);
  ConnectNode(CTL_MOVE_FORWARD_WORD, CTL_MOVE_FORWARD, -2);
  ConnectNode(CTL_MOVE_FORWARD_LINE, CTL_MOVE_FORWARD, -2);
  ConnectNode(CTL_MOVE_FORWARD_FILE, CTL_MOVE_FORWARD, -2);

  ConnectNode(CTL_ROOT, CTL_MOVE_FORWARD_CHAR, -2);
  ConnectNode(CTL_MOVE_FORWARD, CTL_MOVE_FORWARD_CHAR, -2);
  ConnectNode(CTL_MOVE_BACKWARD, CTL_MOVE_FORWARD_CHAR, -2);

  ConnectNode(CTL_ROOT, CTL_MOVE_FORWARD_WORD, -2);
  ConnectNode(CTL_MOVE_FORWARD, CTL_MOVE_FORWARD_WORD, -2);
  ConnectNode(CTL_MOVE_BACKWARD, CTL_MOVE_FORWARD_WORD, -2);

  ConnectNode(CTL_ROOT, CTL_MOVE_FORWARD_LINE, -2);
  ConnectNode(CTL_MOVE_FORWARD, CTL_MOVE_FORWARD_LINE, -2);
  ConnectNode(CTL_MOVE_BACKWARD, CTL_MOVE_FORWARD_LINE, -2);

  ConnectNode(CTL_ROOT, CTL_MOVE_FORWARD_FILE, -2);
  ConnectNode(CTL_MOVE_FORWARD, CTL_MOVE_FORWARD_FILE, -2);
  ConnectNode(CTL_MOVE_BACKWARD, CTL_MOVE_FORWARD_FILE, -2);

  ConnectNode(CTL_MOVE_BACKWARD_CHAR, CTL_MOVE_BACKWARD, -2);
  ConnectNode(CTL_MOVE_BACKWARD_WORD, CTL_MOVE_BACKWARD, -2);
  ConnectNode(CTL_MOVE_BACKWARD_LINE, CTL_MOVE_BACKWARD, -2);
  ConnectNode(CTL_MOVE_BACKWARD_FILE, CTL_MOVE_BACKWARD, -2);

  ConnectNode(CTL_ROOT, CTL_MOVE_BACKWARD_CHAR, -2);
  ConnectNode(CTL_MOVE_FORWARD, CTL_MOVE_BACKWARD_CHAR, -2);
  ConnectNode(CTL_MOVE_BACKWARD, CTL_MOVE_BACKWARD_CHAR, -2);

  ConnectNode(CTL_ROOT, CTL_MOVE_BACKWARD_WORD, -2);
  ConnectNode(CTL_MOVE_FORWARD, CTL_MOVE_BACKWARD_WORD, -2);
  ConnectNode(CTL_MOVE_BACKWARD, CTL_MOVE_BACKWARD_WORD, -2);

  ConnectNode(CTL_ROOT, CTL_MOVE_BACKWARD_LINE, -2);
  ConnectNode(CTL_MOVE_FORWARD, CTL_MOVE_BACKWARD_LINE, -2);
  ConnectNode(CTL_MOVE_BACKWARD, CTL_MOVE_BACKWARD_LINE, -2);

  ConnectNode(CTL_ROOT, CTL_MOVE_BACKWARD_FILE, -2);
  ConnectNode(CTL_MOVE_FORWARD, CTL_MOVE_BACKWARD_FILE, -2);
  ConnectNode(CTL_MOVE_BACKWARD, CTL_MOVE_BACKWARD_FILE, -2);

  ConnectNode(CTL_DELETE_FORWARD, CTL_DELETE, -2);
  ConnectNode(CTL_DELETE_BACKWARD, CTL_DELETE, -2);

  ConnectNode(CTL_DELETE_FORWARD_CHAR, CTL_DELETE_FORWARD, -2);
  ConnectNode(CTL_DELETE_FORWARD_WORD, CTL_DELETE_FORWARD, -2);
  ConnectNode(CTL_DELETE_FORWARD_LINE, CTL_DELETE_FORWARD, -2);
  ConnectNode(CTL_DELETE_FORWARD_FILE, CTL_DELETE_FORWARD, -2);

  ConnectNode(CTL_ROOT, CTL_DELETE_FORWARD_CHAR, -2);
  ConnectNode(CTL_DELETE_FORWARD, CTL_DELETE_FORWARD_CHAR, -2);
  ConnectNode(CTL_DELETE_BACKWARD, CTL_DELETE_FORWARD_CHAR, -2);

  ConnectNode(CTL_ROOT, CTL_DELETE_FORWARD_WORD, -2);
  ConnectNode(CTL_DELETE_FORWARD, CTL_DELETE_FORWARD_WORD, -2);
  ConnectNode(CTL_DELETE_BACKWARD, CTL_DELETE_FORWARD_WORD, -2);

  ConnectNode(CTL_ROOT, CTL_DELETE_FORWARD_LINE, -2);
  ConnectNode(CTL_DELETE_FORWARD, CTL_DELETE_FORWARD_LINE, -2);
  ConnectNode(CTL_DELETE_BACKWARD, CTL_DELETE_FORWARD_LINE, -2);

  ConnectNode(CTL_ROOT, CTL_DELETE_FORWARD_FILE, -2);
  ConnectNode(CTL_DELETE_FORWARD, CTL_DELETE_FORWARD_FILE, -2);
  ConnectNode(CTL_DELETE_BACKWARD, CTL_DELETE_FORWARD_FILE, -2);

  ConnectNode(CTL_DELETE_BACKWARD_CHAR, CTL_DELETE_BACKWARD, -2);
  ConnectNode(CTL_DELETE_BACKWARD_WORD, CTL_DELETE_BACKWARD, -2);
  ConnectNode(CTL_DELETE_BACKWARD_LINE, CTL_DELETE_BACKWARD, -2);
  ConnectNode(CTL_DELETE_BACKWARD_FILE, CTL_DELETE_BACKWARD, -2);

  ConnectNode(CTL_ROOT, CTL_DELETE_BACKWARD_CHAR, -2);
  ConnectNode(CTL_DELETE_FORWARD, CTL_DELETE_BACKWARD_CHAR, -2);
  ConnectNode(CTL_DELETE_BACKWARD, CTL_DELETE_BACKWARD_CHAR, -2);

  ConnectNode(CTL_ROOT, CTL_DELETE_BACKWARD_WORD, -2);
  ConnectNode(CTL_DELETE_FORWARD, CTL_DELETE_BACKWARD_WORD, -2);
  ConnectNode(CTL_DELETE_BACKWARD, CTL_DELETE_BACKWARD_WORD, -2);

  ConnectNode(CTL_ROOT, CTL_DELETE_BACKWARD_LINE, -2);
  ConnectNode(CTL_DELETE_FORWARD, CTL_DELETE_BACKWARD_LINE, -2);
  ConnectNode(CTL_DELETE_BACKWARD, CTL_DELETE_BACKWARD_LINE, -2);

  ConnectNode(CTL_ROOT, CTL_DELETE_BACKWARD_FILE, -2);
  ConnectNode(CTL_DELETE_FORWARD, CTL_DELETE_BACKWARD_FILE, -2);
  ConnectNode(CTL_DELETE_BACKWARD, CTL_DELETE_BACKWARD_FILE, -2);
}

// FIXME - need to take all this down (free control tree);

void CControlManager::RegisterNode( int iID, std::string strLabel, int iColour ) {
  CControlNode *pNewNode;
  
  pNewNode = new CControlNode; // FIXME - do constructor sanely
  pNewNode->strLabel = strLabel;
  pNewNode->iID = iID;
  pNewNode->iColour = iColour;

  m_mapControlMap[iID] = pNewNode;
}

void CControlManager::ConnectNode(int iChild, int iParent, int iAfter) {

  // FIXME - iAfter currently ignored (eventually -1 = start, -2 = end)

  if( iChild == -1 ) // Corresponds to escaping back to alphabet
    m_mapControlMap[iParent]->vChildren.push_back(NULL);
  else
    m_mapControlMap[iParent]->vChildren.push_back(m_mapControlMap[iChild]); 
}


CDasherNode *CControlManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper) {
  CDasherNode *pNewNode;

  // FIXME - is the language model pointer used?
  

  pNewNode = new CDasherNode(*m_pModel, pParent, m_pModel->GetControlSymbol(),0, Opts::Nodes1, iLower, iUpper, m_pLanguageModel, false,  m_mapControlMap[0]->iColour);
 
  // FIXME - handle context properly

  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());

  pNewNode->m_pNodeManager = this;
  pNewNode->m_pUserData = m_mapControlMap[0];
  pNewNode->m_strDisplayText = static_cast<CControlNode*>(pNewNode->m_pUserData)->strLabel;

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

       int iColour((*it)->iColour);

       if( iColour == -1 ) {
	 iColour = (iIdx%99)+11;
       }

       pNewNode = new CDasherNode(*m_pModel, pNode, m_pModel->GetControlSymbol(), 0, Opts::Nodes1, iLbnd, iHbnd, m_pLanguageModel, false, iColour);
       pNewNode->m_pNodeManager = this;
       pNewNode->m_pUserData = *it;
       pNewNode->m_strDisplayText = (*it)->strLabel;
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
  CControlEvent oEvent(pControlNode->iID);
  m_pModel->InsertEvent(&oEvent);
};

void CControlManager::Undo( CDasherNode *pNode ) {
  // Do we ever need this?
  // One other thing we probably want is notification when we leave a node - that way we can eg speed up again if we slowed down
};
