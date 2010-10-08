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

CControlBase::CControlBase( CNodeCreationManager *pNCManager)
  : m_pNCManager(pNCManager), m_pRoot(NULL) {
}

CControlBase::NodeTemplate *CControlBase::GetRootTemplate() {
  return m_pRoot;
}

void CControlBase::SetRootTemplate(NodeTemplate *pRoot) {
  if (m_pRoot || !pRoot) throw "SetRoot should only be called once, with a non-null root";
  m_pRoot = pRoot;
}

CDasherNode *CControlBase::GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, int iOffset) {
  if (!m_pRoot) return m_pNCManager->GetAlphabetManager()->GetRoot(pParent, iLower, iUpper, false, iOffset);

  CContNode *pNewNode = new CContNode(pParent, iOffset, iLower, iUpper, m_pRoot, this);
 
  // FIXME - handle context properly

  //  pNewNode->SetContext(m_pLanguageModel->CreateEmptyContext());

  return pNewNode;
}

CControlBase::NodeTemplate::NodeTemplate(const string &strLabel,int iColour)
: m_strLabel(strLabel), m_iColour(iColour) {
}

CControlBase::EventBroadcast::EventBroadcast(int iEvent, const string &strLabel, int iColour)
: NodeTemplate(strLabel, iColour), m_iEvent(iEvent) {
  
}

void CControlBase::EventBroadcast::happen(CContNode *pNode) {
  CControlEvent oEvent(m_iEvent);
  // TODO: Need to reimplement this
  //  m_pNCManager->m_bContextSensitive=false;
  pNode->mgr()->m_pNCManager->InsertEvent(&oEvent);
}

CControlBase::CContNode::CContNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, NodeTemplate *pTemplate, CControlBase *pMgr)
: CDasherNode(pParent, iOffset, iLbnd, iHbnd, (pTemplate->colour() != -1) ? pTemplate->colour() : (pParent->ChildCount()%99)+11, pTemplate->label()), m_pTemplate(pTemplate), m_pMgr(pMgr) {
}

void CControlBase::CContNode::PopulateChildren() {
  
  CDasherNode *pNewNode;
  
  const unsigned int iNChildren( m_pTemplate->successors.size() );
  const unsigned int iNorm(m_pMgr->m_pNCManager->GetLongParameter(LP_NORMALIZATION));
  unsigned int iLbnd(0), iIdx(0);
  
  for (vector<NodeTemplate *>::iterator it = m_pTemplate->successors.begin(); it!=m_pTemplate->successors.end(); it++) {
    
    const unsigned int iHbnd((++iIdx*iNorm)/iNChildren); 
    
    if( *it == NULL ) {
      // Escape back to alphabet
      
      pNewNode = m_pMgr->m_pNCManager->GetAlphabetManager()->GetRoot(this, iLbnd, iHbnd, false, offset()+1);
    }
    else {
      
      pNewNode = new CContNode(this, offset(), iLbnd, iHbnd, *it, m_pMgr);
    }
    iLbnd=iHbnd;
    DASHER_ASSERT(GetChildren().back()==pNewNode);
  }
}
    
int CControlBase::CContNode::ExpectedNumChildren() {
  return m_pTemplate->successors.size();
}

void CControlBase::CContNode::Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization ) {
  m_pTemplate->happen(this);
}

void CControlBase::CContNode::Enter() {
  // Slow down to half the speed we were at
  m_pMgr->m_pNCManager->SetLongParameter(LP_BOOSTFACTOR, 50);
  //Disable auto speed control!
  m_pMgr->bDisabledSpeedControl = m_pMgr->m_pNCManager->GetBoolParameter(BP_AUTO_SPEEDCONTROL); 
  m_pMgr->m_pNCManager->SetBoolParameter(BP_AUTO_SPEEDCONTROL, 0);
}


void CControlBase::CContNode::Leave() {
  // Now speed back up, by doubling the speed we were at in control mode
  m_pMgr->m_pNCManager->SetLongParameter(LP_BOOSTFACTOR, 100);
  //Re-enable auto speed control!
  if (m_pMgr->bDisabledSpeedControl)
  {
    m_pMgr->bDisabledSpeedControl = false;
    m_pMgr->m_pNCManager->SetBoolParameter(BP_AUTO_SPEEDCONTROL, 1);
  }
}

COrigNodes::COrigNodes(CNodeCreationManager *pNCManager, CDasherInterfaceBase *pInterface) : CControlBase(pNCManager), m_pInterface(pInterface) {
  m_iNextID = 0;
  
  // TODO: Need to fix this on WinCE build
#ifndef _WIN32_WCE
  if(!LoadLabelsFromFile(m_pNCManager->GetStringParameter(SP_USER_LOC) + "controllabels.xml")) {
    //  something went wrong
    if (!LoadLabelsFromFile(m_pNCManager->GetStringParameter(SP_SYSTEM_LOC)+"controllabels.xml")) {
      // all else fails do something default
      LoadDefaultLabels();
    }
  }
  ConnectNodes();
  SetRootTemplate(m_perId[CTL_ROOT]);
#endif
}

bool COrigNodes::LoadLabelsFromFile(string strFileName) {
  int iFileSize;
  {
    struct stat sFileInfo;
    if (stat(strFileName.c_str(), &sFileInfo)==-1) return false; //fail
    iFileSize = sFileInfo.st_size;
  }
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

COrigNodes::Pause::Pause(const string &strLabel, int iColour) : NodeTemplate(strLabel,iColour) {
}
void COrigNodes::Pause::happen(CContNode *pNode) {
  static_cast<COrigNodes *>(pNode->mgr())->m_pNCManager->SetBoolParameter(BP_DASHER_PAUSED,true);
}

COrigNodes::Stop::Stop(const string &strLabel, int iColour) : NodeTemplate(strLabel, iColour) {
}
void COrigNodes::Stop::happen(CContNode *pNode) {
  static_cast<COrigNodes *>(pNode->mgr())->m_pInterface->Stop();
}

bool COrigNodes::LoadDefaultLabels() {
  //hmmm. This is probably not the most flexible policy...
  if (!m_perId.empty()) return false;
  
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
  return true;
}

void COrigNodes::ConnectNodes() {
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

COrigNodes::~COrigNodes() {
  for (std::map<int,NodeTemplate *>::iterator it = m_perId.begin(); it!=m_perId.end(); it++)
    delete it->second;
}

void COrigNodes::RegisterNode( int iID, std::string strLabel, int iColour ) {
  DASHER_ASSERT(m_perId.count(iID)==0);
  if (iID == CTL_STOP) m_perId[iID] = new Stop(strLabel,iColour);
  else if (iID == CTL_PAUSE) m_perId[iID] = new Pause(strLabel, iColour);
  else m_perId[iID] = new EventBroadcast(iID,strLabel,iColour);
}

void COrigNodes::ConnectNode(int iChild, int iParent, int iAfter) {
  //ACL duplicating old functionality here. Idea had been to do
  // something with iAfter "(eventually -1 = start, -2 = end)", but
  // since this wasn't used, and this is all legacy code anyway ;-),
  // I'm leaving as is...
  
  NodeTemplate *pParent(m_perId[iParent]);
  if (pParent) //Note - old code only checked this if iChild==-1...?!
    pParent->successors.push_back(iChild==-1 ? NULL : m_perId[iChild]);
}

void COrigNodes::DisconnectNode(int iChild, int iParent) {
  NodeTemplate *pChild(m_perId[iChild]), *pParent(m_perId[iParent]);
  if (pParent && (pChild || iChild == -1)) {
    for (vector<NodeTemplate *>::iterator it = pParent->successors.begin(); it!=pParent->successors.end(); it++) {
      if (*it == pChild) {
        pParent->successors.erase(it);
      }
    }
  }
}

void COrigNodes::XmlStartHandler(void *pUserData, const XML_Char *szName, const XML_Char **aszAttr) {
  COrigNodes *pMgr(static_cast<COrigNodes *>(pUserData));
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
    pMgr->RegisterNode(pMgr->m_iNextID++, str, colour);
  }
}

void COrigNodes::XmlEndHandler(void *pUserData, const XML_Char *szName) {
}

void COrigNodes::XmlCDataHandler(void *pUserData, const XML_Char *szData, int iLength){
}

CControlManager::CControlManager(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CNodeCreationManager *pNCManager, CDasherInterfaceBase *pInterface)
: CDasherComponent(pEventHandler, pSettingsStore), COrigNodes(pNCManager, pInterface), m_pSpeech(NULL), m_pCopy(NULL) {
  
  updateActions();
}

CControlManager::~CControlManager() {
  delete m_pSpeech;
  delete m_pCopy;
}

class TextActionHeader : public CDasherInterfaceBase::TextAction, public CControlBase::NodeTemplate {
public:
  TextActionHeader(CDasherInterfaceBase *pIntf, const string &strHdr, NodeTemplate *pRoot) : TextAction(pIntf), NodeTemplate(strHdr,-1),
  m_all(this, "All", &CDasherInterfaceBase::TextAction::executeOnAll),
  m_new(this, "New", &CDasherInterfaceBase::TextAction::executeOnNew),
  m_again(this, "Repeat", &CDasherInterfaceBase::TextAction::executeLast) {
    successors.push_back(&m_all); m_all.successors.push_back(NULL); m_all.successors.push_back(pRoot);
    successors.push_back(&m_new); m_new.successors.push_back(NULL); m_new.successors.push_back(pRoot);
    successors.push_back(&m_again); m_again.successors.push_back(NULL); m_again.successors.push_back(pRoot);
  }
private:
  CControlBase::MethodTemplate<CDasherInterfaceBase::TextAction> m_all, m_new, m_again;
};

class SpeechHeader : public TextActionHeader {
public:
  SpeechHeader(CDasherInterfaceBase *pIntf, NodeTemplate *pRoot) : TextActionHeader(pIntf, "Speak", pRoot) {
  }
  void operator()(const std::string &strText) {
    m_pIntf->Speak(strText, true);
  }
};

class CopyHeader  : public TextActionHeader {
public:
  CopyHeader(CDasherInterfaceBase *pIntf, NodeTemplate *pRoot) : TextActionHeader(pIntf, "Copy", pRoot) {
  }
  void operator()(const std::string &strText) {
    m_pIntf->CopyToClipboard(strText);
  }
};

void CControlManager::HandleEvent(CEvent *pEvent) {
  if (pEvent->m_iEventType == EV_PARAM_NOTIFY) {
    switch (static_cast<CParameterNotificationEvent *>(pEvent)->m_iParameter) {
      case BP_CONTROL_MODE_HAS_HALT:
      case BP_CONTROL_MODE_HAS_EDIT:
      case BP_CONTROL_MODE_HAS_SPEECH:
      case BP_CONTROL_MODE_HAS_COPY:
      case BP_COPY_ALL_ON_STOP:
      case BP_SPEAK_ALL_ON_STOP:
      case SP_INPUT_FILTER:
        updateActions();
    }
  }
}

void CControlManager::updateActions() {
  vector<NodeTemplate *> &vRootSuccessors(GetRootTemplate()->successors);
  vector<NodeTemplate *> vOldRootSuccessors;
  vOldRootSuccessors.swap(vRootSuccessors);
  vector<NodeTemplate *>::iterator it=vOldRootSuccessors.begin();
  DASHER_ASSERT(*it == NULL); //escape back to alphabet
  vRootSuccessors.push_back(*it++);
  
  //stop does something, and we're told to add a node for it
  // (either a dynamic filter where the user can't use the normal stop mechanism precisely,
  //  or a static filter but a 'stop' action is easier than using speak->all / copy->all then pause)
  if (m_pInterface->hasStopTriggers() && m_pInterface->GetBoolParameter(BP_CONTROL_MODE_HAS_HALT))
    vRootSuccessors.push_back(m_perId[CTL_STOP]);
  if (it!=vOldRootSuccessors.end() && *it == m_perId[CTL_STOP]) it++;
  
  //filter is pauseable, and either 'stop' would do something (so pause is different),
  // or we're told to have a stop node but it would be indistinguishable from pause (=>have pause)
  CInputFilter *pInput(static_cast<CInputFilter *>(m_pInterface->GetModuleByName(m_pInterface->GetStringParameter(SP_INPUT_FILTER))));
  if (pInput->supportsPause() && (m_pInterface->hasStopTriggers() || m_pInterface->GetBoolParameter(BP_CONTROL_MODE_HAS_HALT)))
    vRootSuccessors.push_back(m_perId[CTL_PAUSE]);
  if (it!=vOldRootSuccessors.end() && *it == m_perId[CTL_PAUSE]) it++;
  
  if (m_pInterface->GetBoolParameter(BP_CONTROL_MODE_HAS_SPEECH) && m_pInterface->SupportsSpeech()) {
    if (!m_pSpeech) m_pSpeech = new SpeechHeader(m_pInterface, GetRootTemplate());
    vRootSuccessors.push_back(m_pSpeech);
  }
  if (it!=vOldRootSuccessors.end() && *it == m_pSpeech) it++;
  
  if (m_pInterface->GetBoolParameter(BP_CONTROL_MODE_HAS_COPY) && m_pInterface->SupportsClipboard()) {
    if (!m_pCopy) m_pCopy = new CopyHeader(m_pInterface, GetRootTemplate());
    vRootSuccessors.push_back(m_pCopy);
  }
  if (it!=vOldRootSuccessors.end() && *it == m_pCopy) it++;

  if (m_pInterface->GetBoolParameter(BP_CONTROL_MODE_HAS_EDIT)) {
    vRootSuccessors.push_back(m_perId[CTL_MOVE]);
    vRootSuccessors.push_back(m_perId[CTL_DELETE]);
  }
  if (it!=vOldRootSuccessors.end() && *it == m_perId[CTL_MOVE]) it++;
  if (it!=vOldRootSuccessors.end() && *it == m_perId[CTL_DELETE]) it++;
  
  //copy anything else (custom) that might have been added...
  while (it != vOldRootSuccessors.end()) vRootSuccessors.push_back(*it++);
}
