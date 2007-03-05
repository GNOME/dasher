
#include "../Common/Common.h"

#include "ControlManager.h"
using namespace Dasher;
using namespace std;
// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

int CControlManager::m_iNextID = 0;

CControlManager::CControlManager( CNodeCreationManager *pNCManager )
  : CNodeManager(1), m_pNCManager(pNCManager), m_pLanguageModel(NULL) {
  string SystemString = m_pNCManager->GetStringParameter(SP_SYSTEM_LOC);
  string UserLocation = m_pNCManager->GetStringParameter(SP_USER_LOC);
  m_iNextID = 0;
  struct stat sFileInfo;
  string strFileName = UserLocation + "controllabels.xml";  //  check first location for file
  if(stat(strFileName.c_str(), &sFileInfo) == -1) {
    //  something went wrong
    strFileName = SystemString + "controllabels.xml"; //  check second location for file
    if(stat(strFileName.c_str(), &sFileInfo) == -1) {
      // all else fails do something default
      LoadDefaultLabels();
    }
      else
	LoadLabelsFromFile(strFileName, sFileInfo.st_size);
  }
  else
    LoadLabelsFromFile(strFileName, sFileInfo.st_size);
  
  ConnectNodes();
}

int CControlManager::LoadLabelsFromFile(string strFileName, int iFileSize) {

  // Implement Unicode names via xml from file:
  char* szFileBuffer = new char[iFileSize];
  ifstream oFile(strFileName.c_str());
  oFile.read(szFileBuffer, iFileSize);
  XML_Parser Parser = XML_ParserCreate(NULL);

  // Members passed as callbacks must be static, so don't have a "this" pointer.
  // We give them one through horrible casting so they can effect changes.
  XML_SetUserData(Parser, this);

  XML_SetElementHandler(Parser, XmlStartHandler, XmlEndHandler);
  XML_SetCharacterDataHandler(Parser, XmlCDataHandler);
  XML_Parse(Parser, szFileBuffer, iFileSize, false);
  //  deallocate resources
  XML_ParserFree(Parser);
  oFile.close();
  delete [] szFileBuffer;
  return 0;
}

int CControlManager::LoadDefaultLabels() {
  // TODO: Need to figure out how to handle offset changes here

  RegisterNode(CTL_ROOT, "Control", 8);
  RegisterNode(CTL_STOP, "Stop", 242);
  RegisterNode(CTL_PAUSE, "Pause", 241);
  RegisterNode(CTL_MOVE, "Move", -1);
  RegisterNode(CTL_MOVE_FORWARD, "->", -1);
  RegisterNode(CTL_MOVE_FORWARD_CHAR, ">", -1);
  RegisterNode(CTL_MOVE_FORWARD_WORD, ">>", -1);
  RegisterNode(CTL_MOVE_FORWARD_LINE, ">>>", -1);
  RegisterNode(CTL_MOVE_FORWARD_FILE, ">>>>", -1);
  RegisterNode(CTL_MOVE_BACKWARD, "<-", -1);
  RegisterNode(CTL_MOVE_BACKWARD_CHAR, "<", -1);
  RegisterNode(CTL_MOVE_BACKWARD_WORD, "<<", -1);
  RegisterNode(CTL_MOVE_BACKWARD_LINE, "<<<", -1);
  RegisterNode(CTL_MOVE_BACKWARD_FILE, "<<<<", -1);
  RegisterNode(CTL_DELETE, "Delete", -1);
  RegisterNode(CTL_DELETE_FORWARD, "->", -1);
  RegisterNode(CTL_DELETE_FORWARD_CHAR, ">", -1);
  RegisterNode(CTL_DELETE_FORWARD_WORD, ">>", -1);
  RegisterNode(CTL_DELETE_FORWARD_LINE, ">>>", -1);
  RegisterNode(CTL_DELETE_FORWARD_FILE, ">>>>", -1);
  RegisterNode(CTL_DELETE_BACKWARD, "<-", -1);
  RegisterNode(CTL_DELETE_BACKWARD_CHAR, "<", -1);
  RegisterNode(CTL_DELETE_BACKWARD_WORD, "<<", -1);
  RegisterNode(CTL_DELETE_BACKWARD_LINE, "<<<", -1);
  RegisterNode(CTL_DELETE_BACKWARD_FILE, "<<<<", -1);
  return 0;
}

int CControlManager::ConnectNodes() {
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
  return 0;
}

CControlManager::~CControlManager()
{
  for(std::map<int,CControlNode*>::iterator i = m_mapControlMap.begin(); i != m_mapControlMap.end(); i++) {
    CControlNode* pNewNode = i->second;
    if (pNewNode != NULL) {
      delete pNewNode;
      pNewNode = NULL;
    }
  }
}

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

  if( iChild == -1 ) {// Corresponds to escaping back to alphabet
    CControlNode* node = m_mapControlMap[iParent];
    if(node)
      node->vChildren.push_back(NULL);
  }
  else
    m_mapControlMap[iParent]->vChildren.push_back(m_mapControlMap[iChild]); 
}

void CControlManager::DisconnectNode(int iChild, int iParent) {
  CControlNode* pParentNode = m_mapControlMap[iParent];
  CControlNode* pChildNode = m_mapControlMap[iChild];

  for(std::vector<CControlNode *>::iterator itChild(pParentNode->vChildren.begin()); itChild != pParentNode->vChildren.end(); ++itChild)
    if(*itChild == pChildNode)
      pParentNode->vChildren.erase(itChild);
}


CDasherNode *CControlManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CDasherNode *pNewNode;

  // TODO: Tie this structure to info contained in control map
  CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
  pDisplayInfo->iColour = m_mapControlMap[0]->iColour;
  pDisplayInfo->bShove = false;
  pDisplayInfo->bVisible = true;
  pDisplayInfo->strDisplayText = m_mapControlMap[0]->strLabel;
  
  pNewNode = new CDasherNode(pParent, iLower, iUpper, pDisplayInfo);

  int iOffset = *((int *)pUserData);
 
  // FIXME - handle context properly

  //  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());

  pNewNode->m_pNodeManager = this;

  SControlData *pNodeUserData = new SControlData;

  pNodeUserData->pControlNode = m_mapControlMap[0];
  pNodeUserData->iOffset = iOffset;

  pNewNode->m_pUserData = pNodeUserData;

  return pNewNode;
}

void CControlManager::PopulateChildren( CDasherNode *pNode ) {
   CDasherNode *pNewNode;

   CControlNode *pControlNode((static_cast<SControlData *>(pNode->m_pUserData))->pControlNode);

   int iNChildren( pControlNode->vChildren.size() );

   int iIdx(0);

   for(std::vector<CControlNode *>::iterator it(pControlNode->vChildren.begin()); it != pControlNode->vChildren.end(); ++it) {

     // FIXME - could do this better

     int iLbnd( iIdx*(m_pNCManager->GetLongParameter(LP_NORMALIZATION)/iNChildren)); 
     int iHbnd( (iIdx+1)*(m_pNCManager->GetLongParameter(LP_NORMALIZATION)/iNChildren)); 

     if( *it == NULL ) {
       // Escape back to alphabet
       CAlphabetManager::SRootData *pRootData = new CAlphabetManager::SRootData;

       // TODO: Check that these are eventually getting deleted

       pRootData->iOffset = (static_cast<SControlData *>(pNode->m_pUserData))->iOffset;
       pRootData->szContext = NULL; // TODO: Fix this

       pNewNode = m_pNCManager->GetRoot(0, pNode, iLbnd, iHbnd, pRootData);
       pNewNode->SetFlag(NF_SEEN, false);
     }
     else {

       int iColour((*it)->iColour);

       if( iColour == -1 ) {
	 iColour = (iIdx%99)+11;
       }

       CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
       pDisplayInfo->iColour = iColour;
       pDisplayInfo->bShove = false;
       pDisplayInfo->bVisible = true;
       pDisplayInfo->strDisplayText = (*it)->strLabel;
       
       pNewNode = new CDasherNode(pNode, iLbnd, iHbnd, pDisplayInfo);

       pNewNode->m_pNodeManager = this;
       pNewNode->m_pUserData = *it;

       SControlData *pNodeUserData = new SControlData;

       pNodeUserData->pControlNode = *it;
       pNodeUserData->iOffset = (static_cast<SControlData *>(pNode->m_pUserData))->iOffset;

       pNewNode->m_pUserData = pNodeUserData;

     }
     pNode->Children().push_back(pNewNode);
     ++iIdx;
   }
}

void CControlManager::ClearNode( CDasherNode *pNode ) {
  delete (static_cast<SControlData *>(pNode->m_pUserData));
}

void CControlManager::Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization ) {

  CControlNode *pControlNode((static_cast<SControlData *>(pNode->m_pUserData))->pControlNode);

  CControlEvent oEvent(pControlNode->iID);
  // TODO: Need to reimplement this
  //  m_pNCManager->m_bContextSensitive=false;
  m_pNCManager->InsertEvent(&oEvent);
}

void CControlManager::Undo( CDasherNode *pNode ) {
  // Do we ever need this?
  // One other thing we probably want is notification when we leave a node - that way we can eg speed up again if we slowed down
  m_pNCManager->SetLongParameter(LP_SPEED_DIVISOR, 100);
  //Re-enable auto speed control!
  m_pNCManager->SetBoolParameter(BP_AUTO_SPEEDCONTROL, 1);
  
}

void CControlManager::Enter(CDasherNode *pNode) {
  // Slow down to half the speed we were at
  m_pNCManager->SetLongParameter(LP_SPEED_DIVISOR, 200);
  //Disable auto speed control!
  m_pNCManager->SetBoolParameter(BP_AUTO_SPEEDCONTROL, 0);
}


void CControlManager::Leave(CDasherNode *pNode) {
  // Now speed back up, by doubling the speed we were at in control mode
  m_pNCManager->SetLongParameter(LP_SPEED_DIVISOR, 100);
  //Re-enable auto speed control!
  m_pNCManager->SetBoolParameter(BP_AUTO_SPEEDCONTROL, 1);
}


void CControlManager::XmlStartHandler(void *pUserData, const XML_Char *szName, const XML_Char **aszAttr) {
  
  int colour;
  string str;
  if(0==strcmp(szName, "label"))
  {
    for(int i = 0; aszAttr[i]; i += 2)
    {
      if(0==strcmp(aszAttr[i],"value"))
      {
        str = string(aszAttr[i+1]);
      }
      if(0==strcmp(aszAttr[i],"color"))
      {
        colour = atoi(aszAttr[i+1]);
      }  
    }
	((CControlManager*)pUserData)->RegisterNode(CControlManager::m_iNextID++, str, colour);
    
  }
}

void CControlManager::XmlEndHandler(void *pUserData, const XML_Char *szName) {
  return;
}

void CControlManager::XmlCDataHandler(void *pUserData, const XML_Char *szData, int iLength){
  return;
}

void CControlManager::SetControlOffset(CDasherNode *pNode, int iOffset) {
  (static_cast<SControlData *>(pNode->m_pUserData))->iOffset = iOffset;
}
