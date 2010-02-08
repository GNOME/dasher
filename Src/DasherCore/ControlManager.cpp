// ControlManager.cpp
//
// Copyright (c) 2007 The Dasher Team
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

#include "ControlManager.h"
#include <cstring>

using namespace Dasher;
using namespace std;
// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

int CControlManager::m_iNextID = 0;

CControlManager::CControlManager( CNodeCreationManager *pNCManager )
  : m_pNCManager(pNCManager) {
  string SystemString = m_pNCManager->GetStringParameter(SP_SYSTEM_LOC);
  string UserLocation = m_pNCManager->GetStringParameter(SP_USER_LOC);
  m_iNextID = 0;

  // TODO: Need to fix this on WinCE build
#ifndef _WIN32_WCE
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
#endif
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
  for(std::map<int,SControlItem*>::iterator i = m_mapControlMap.begin(); i != m_mapControlMap.end(); i++) {
    SControlItem* pNewNode = i->second;
    if (pNewNode != NULL) {
      delete pNewNode;
      pNewNode = NULL;
    }
  }
}

void CControlManager::RegisterNode( int iID, std::string strLabel, int iColour ) {
  SControlItem *pNewNode;
  
  pNewNode = new SControlItem; // FIXME - do constructor sanely
  pNewNode->strLabel = strLabel;
  pNewNode->iID = iID;
  pNewNode->iColour = iColour;

  m_mapControlMap[iID] = pNewNode;
}

void CControlManager::ConnectNode(int iChild, int iParent, int iAfter) {

  // FIXME - iAfter currently ignored (eventually -1 = start, -2 = end)

  if( iChild == -1 ) {// Corresponds to escaping back to alphabet
    SControlItem* node = m_mapControlMap[iParent];
    if(node)
      node->vChildren.push_back(NULL);
  }
  else
    m_mapControlMap[iParent]->vChildren.push_back(m_mapControlMap[iChild]); 
}

void CControlManager::DisconnectNode(int iChild, int iParent) {
  SControlItem* pParentNode = m_mapControlMap[iParent];
  SControlItem* pChildNode = m_mapControlMap[iChild];

  for(std::vector<SControlItem *>::iterator itChild(pParentNode->vChildren.begin()); itChild != pParentNode->vChildren.end(); ++itChild)
    if(*itChild == pChildNode)
      pParentNode->vChildren.erase(itChild);
}


CDasherNode *CControlManager::GetRoot(CDasherNode *pParent, int iLower, int iUpper, int iOffset) {

  // TODO: Tie this structure to info contained in control map
  CDasherNode::SDisplayInfo *pDisplayInfo = new CDasherNode::SDisplayInfo;
  pDisplayInfo->iColour = m_mapControlMap[0]->iColour;
  pDisplayInfo->bShove = false;
  pDisplayInfo->bVisible = true;
  pDisplayInfo->strDisplayText = m_mapControlMap[0]->strLabel;
  
  CContNode *pNewNode = new CContNode(pParent, iLower, iUpper, pDisplayInfo, this);
 
  // FIXME - handle context properly

  //  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());

  pNewNode->pControlItem = m_mapControlMap[0];
  pNewNode->m_iOffset = iOffset;

  return pNewNode;
}

CControlManager::CContNode::CContNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDisplayInfo, CControlManager *pMgr)
: CDasherNode(pParent, iLbnd, iHbnd, pDisplayInfo), m_pMgr(pMgr) {
}


void CControlManager::CContNode::PopulateChildren() {
  
  CDasherNode *pNewNode;

   int iNChildren( pControlItem->vChildren.size() );

   int iIdx(0);

   for(std::vector<SControlItem *>::iterator it(pControlItem->vChildren.begin()); it != pControlItem->vChildren.end(); ++it) {

     // FIXME - could do this better

     int iLbnd( iIdx*(m_pMgr->m_pNCManager->GetLongParameter(LP_NORMALIZATION)/iNChildren)); 
     int iHbnd( (iIdx+1)*(m_pMgr->m_pNCManager->GetLongParameter(LP_NORMALIZATION)/iNChildren)); 

     if( *it == NULL ) {
       // Escape back to alphabet

       pNewNode = m_pMgr->m_pNCManager->GetAlphRoot(this, iLbnd, iHbnd, false, m_iOffset);
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
       
       CContNode *pContNode;
       pNewNode = pContNode = new CContNode(this, iLbnd, iHbnd, pDisplayInfo, m_pMgr);

       pContNode->pControlItem = *it;

       pNewNode->m_iOffset = m_iOffset;
     }
     DASHER_ASSERT(GetChildren().back()==pNewNode);
     ++iIdx;
   }
}
int CControlManager::CContNode::ExpectedNumChildren() {
  return pControlItem->vChildren.size();
}
void CControlManager::CContNode::Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization ) {

  CControlEvent oEvent(pControlItem->iID);
  // TODO: Need to reimplement this
  //  m_pNCManager->m_bContextSensitive=false;
  m_pMgr->m_pNCManager->InsertEvent(&oEvent);
}

void CControlManager::CContNode::Undo() {
  // Do we ever need this?
  // One other thing we probably want is notification when we leave a node - that way we can eg speed up again if we slowed down
  m_pMgr->m_pNCManager->SetLongParameter(LP_BOOSTFACTOR, 100);
  //Re-enable auto speed control!
  if (m_pMgr->bDisabledSpeedControl)
  {
    m_pMgr->bDisabledSpeedControl = false;
    m_pMgr->m_pNCManager->SetBoolParameter(BP_AUTO_SPEEDCONTROL, 1);
  }
}

void CControlManager::CContNode::Enter() {
  // Slow down to half the speed we were at
  m_pMgr->m_pNCManager->SetLongParameter(LP_BOOSTFACTOR, 50);
  //Disable auto speed control!
  m_pMgr->bDisabledSpeedControl = m_pMgr->m_pNCManager->GetBoolParameter(BP_AUTO_SPEEDCONTROL); 
  m_pMgr->m_pNCManager->SetBoolParameter(BP_AUTO_SPEEDCONTROL, 0);
}


void CControlManager::CContNode::Leave() {
  // Now speed back up, by doubling the speed we were at in control mode
  m_pMgr->m_pNCManager->SetLongParameter(LP_BOOSTFACTOR, 100);
  //Re-enable auto speed control!
  if (m_pMgr->bDisabledSpeedControl)
  {
    m_pMgr->bDisabledSpeedControl = false;
    m_pMgr->m_pNCManager->SetBoolParameter(BP_AUTO_SPEEDCONTROL, 1);
  }
}


void CControlManager::XmlStartHandler(void *pUserData, const XML_Char *szName, const XML_Char **aszAttr) {
  
  int colour=-1;
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

void CControlManager::CContNode::SetControlOffset(int iOffset) {
  m_iOffset = iOffset;
}
