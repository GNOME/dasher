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
#include "DasherInterfaceBase.h"
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

void CControlBase::MakeLabels(CDasherScreen *pScreen) {
  deque<NodeTemplate *> templateQueue(1,m_pRoot);
  set<NodeTemplate *> allTemplates(templateQueue.begin(),templateQueue.end());
  while (!templateQueue.empty()) {
    NodeTemplate *head = templateQueue.front();
    templateQueue.pop_front();
    delete head->m_pLabel;
    head->m_pLabel = pScreen->MakeLabel(head->m_strLabel);
    for (vector<NodeTemplate *>::iterator it = head->successors.begin(); it!=head->successors.end(); it++) {
      if (!(*it)) continue; //an escape back to the alphabet, no label/successors here
      if (allTemplates.find(*it)==allTemplates.end()) {
        allTemplates.insert(*it);
        templateQueue.push_back(*it);
      }
    }
  }
}

CControlBase::NodeTemplate::NodeTemplate(const string &strLabel,int iColour)
: m_strLabel(strLabel), m_iColour(iColour), m_pLabel(NULL) {
}

CControlBase::NodeTemplate::~NodeTemplate() {
  delete m_pLabel;
}

CControlBase::CContNode::CContNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, NodeTemplate *pTemplate, CControlBase *pMgr)
: CDasherNode(pParent, iOffset, iLbnd, iHbnd, (pTemplate->m_iColour != -1) ? pTemplate->m_iColour : (pParent->ChildCount()%99)+11, pTemplate->m_pLabel), m_pTemplate(pTemplate), m_pMgr(pMgr) {
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

const vector<CControlBase::NodeTemplate *> &CControlParser::parsedNodes() {
  return m_vParsed;
}

class XMLNodeTemplate : public CControlBase::NodeTemplate {
public:
  XMLNodeTemplate(const string &label, int color) : NodeTemplate(label, color) {
  }

  void happen(CControlBase::CContNode *pNode) {
    for (vector<Action*>::iterator it=actions.begin(); it!=actions.end(); it++)
      (*it)->happen(pNode);
  }

  ~XMLNodeTemplate() {
    for (vector<Action*>::iterator it=actions.begin(); it!=actions.end(); it++)
      delete *it;
  }

  vector<CControlBase::Action*> actions;
};

bool CControlParser::LoadFile(const string &strFileName) {
  ///Template used for all node defns read in from XML - just
  /// execute a list of Actions.

  class ParseHandler : public AbstractXMLParser {
    typedef CControlBase::NodeTemplate NodeTemplate;
  protected:
    void XmlStartHandler(const XML_Char *name, const XML_Char **atts) {
      vector<NodeTemplate *> &parent(nodeStack.empty() ? m_pMgr->m_vParsed : nodeStack.back()->successors);
      if (strcmp(name,"node")==0) {
        string label,nodeName; int color=-1;
        while (*atts) {
          if (strcmp(*atts,"name")==0) {
            nodeName=*(atts+1);
            DASHER_ASSERT(namedNodes.find(nodeName)==namedNodes.end());
          } else if (strcmp(*atts,"label")==0) {
            label = *(atts+1);
          } else if (strcmp(*atts,"color")==0) {
            color = atoi(*(atts+1));
          }
          atts+=2;
        }
        XMLNodeTemplate *n = new XMLNodeTemplate(label,color);
        parent.push_back(n);
        nodeStack.push_back(n);
        if (nodeName!="")
          namedNodes[nodeName]=n; //all refs resolved at end.
      } else if (strcmp(name,"ref")==0) {
        string target;
        while (*atts) {
          if (strcmp(*atts,"name")==0)
            target=*(atts+1);
          atts+=2;
        }
        map<string,NodeTemplate*>::iterator it=namedNodes.find(target);
        if (it!=namedNodes.end())
          parent.push_back(it->second);
        else {
          parent.push_back(NULL);
          unresolvedRefs.push_back(pair<NodeTemplate**,string>(&(parent.back()),target));
        }
      } else if (strcmp(name,"alph")==0) {
        parent.push_back(NULL);
      } else if (NodeTemplate *n = m_pMgr->parseOther(name, atts)) {
        parent.push_back(n);
      } else if (CControlBase::Action *a=m_pMgr->parseAction(name, atts)) {
        DASHER_ASSERT(!nodeStack.empty());
        nodeStack.back()->actions.push_back(a);
      }
    }

    void XmlEndHandler(const XML_Char *szName) {
      if (strcmp(szName,"node")==0) {
        DASHER_ASSERT(!nodeStack.empty());
        nodeStack.pop_back();
      }
    }

  private:
    ///Following only used in parsing...
    map<string,NodeTemplate*> namedNodes;
    vector<pair<NodeTemplate**,string> > unresolvedRefs;
    vector<XMLNodeTemplate*> nodeStack;
    CControlParser *m_pMgr;
  public:
    ParseHandler(CControlParser *pMgr) : m_pMgr(pMgr) {
    }
    void resolveRefs() {
      //resolve any forward references to nodes declared later
      for (vector<pair<NodeTemplate**,string> >::iterator it=unresolvedRefs.begin(); it!=unresolvedRefs.end(); it++) {
        map<string,NodeTemplate*>::iterator target = namedNodes.find(it->second);
        if (target != namedNodes.end())
          *(it->first) = target->second;
      }
      //somehow, need to clear out any refs that weren't resolved...???
    }
  };

  ParseHandler p(this);
  if (!p.ParseFile(strFileName)) return false;
  p.resolveRefs();
  return true;
}

CControlManager::CControlManager(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CNodeCreationManager *pNCManager, CDasherInterfaceBase *pInterface)
: CDasherComponent(pEventHandler, pSettingsStore), CControlBase(pNCManager), m_pInterface(pInterface), m_pSpeech(NULL), m_pCopy(NULL) {
  //TODO, used to be able to change label+colour of root/pause/stop from controllabels.xml
  // (or, get the root node title "control" from the alphabet!)
  SetRootTemplate(new NodeTemplate("Control",8)); //default NodeTemplate does nothing
  GetRootTemplate()->successors.push_back(NULL);

  m_pPause = new Pause("Pause",241);
  m_pPause->successors.push_back(NULL);
  m_pPause->successors.push_back(GetRootTemplate());
  m_pStop = new MethodTemplate<CDasherInterfaceBase>("Stop", 242, pInterface, &CDasherInterfaceBase::Stop);
  m_pStop->successors.push_back(NULL);
  m_pStop->successors.push_back(GetRootTemplate());

  //TODO, have a parameter to try first, and if that fails:
  if(!LoadFile(m_pNCManager->GetStringParameter(SP_USER_LOC) + "control.xml")) {
    LoadFile(m_pNCManager->GetStringParameter(SP_SYSTEM_LOC)+"control.xml");
    //if that fails, we'll have no editing functions. Fine -
    // doesn't seem vital enough to hardcode a fallback as well!
  }

  updateActions();
}

CControlManager::Pause::Pause(const string &strLabel, int iColour) : NodeTemplate(strLabel,iColour) {
}
void CControlManager::Pause::happen(CContNode *pNode) {
  static_cast<CControlManager *>(pNode->mgr())->m_pNCManager->SetBoolParameter(BP_DASHER_PAUSED,true);
}

CControlBase::NodeTemplate *CControlManager::parseOther(const XML_Char *name, const XML_Char **atts) {
  if (strcmp(name,"root")==0) return GetRootTemplate();
  return CControlParser::parseOther(name, atts);
}

CControlBase::Action *CControlManager::parseAction(const XML_Char *name, const XML_Char **atts) {
  if (strcmp(name,"delete")==0 || strcmp(name,"move")==0) {
    bool bForwards=true; //pick some defaults...
    EditDistance dist=EDIT_WORD; // (?!)
    while (*atts) {
      if (strcmp(*atts,"forward")==0)
        bForwards=(strcmp(*(atts+1),"yes")==0 || strcmp(*(atts+1),"true")==0 || strcmp(*(atts+1),"on")==0);
      else if (strcmp(*atts,"dist")==0) {
        if (strcmp(*(atts+1),"char")==0)
          dist=EDIT_CHAR;
        else if (strcmp(*(atts+1),"word")==0)
          dist=EDIT_WORD;
        else if (strcmp(*(atts+1),"line")==0)
          dist=EDIT_LINE;
        else if (strcmp(*(atts+1),"file")==0)
          dist=EDIT_FILE;
      }
      atts+=2;
    }
    class DirDist {
    protected:
      const bool m_bForwards;
      const EditDistance m_dist;
    public:
      DirDist(bool bForwards,EditDistance dist) : m_bForwards(bForwards), m_dist(dist) {
      }
    };

    if (name[0]=='d') {
      class Delete : private DirDist, public Action {
      public:
        Delete(bool bForwards,EditDistance dist) : DirDist(bForwards,dist) {
        }
        void happen(CContNode *pNode) {
          static_cast<CControlManager*>(pNode->mgr())->m_pInterface->ctrlDelete(m_bForwards,m_dist);
        }
      };
      return new Delete(bForwards,dist);
    } else {
      class Move : private DirDist, public Action {
      public:
        Move(bool bForwards,EditDistance dist) : DirDist(bForwards, dist) {}
        void happen(CContNode *pNode) {
          static_cast<CControlManager*>(pNode->mgr())->m_pInterface->ctrlMove(m_bForwards,m_dist);
        };
      };
      return new Move(bForwards, dist);
    }
  }
  return CControlParser::parseAction(name, atts);
}

CControlManager::~CControlManager() {
  delete m_pSpeech;
  delete m_pCopy;
}

class TextActionHeader : public CDasherInterfaceBase::TextAction, public CControlBase::NodeTemplate {
public:
  TextActionHeader(CDasherInterfaceBase *pIntf, const string &strHdr, NodeTemplate *pRoot) : TextAction(pIntf), NodeTemplate(strHdr,-1),
  m_all("All", -1, this, &CDasherInterfaceBase::TextAction::executeOnAll),
  m_new("New", -1, this, &CDasherInterfaceBase::TextAction::executeOnNew),
  m_again("Repeat", -1, this, &CDasherInterfaceBase::TextAction::executeLast) {
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
    vRootSuccessors.push_back(m_pStop);
  if (it!=vOldRootSuccessors.end() && *it == m_pStop) it++;

  //filter is pauseable, and either 'stop' would do something (so pause is different),
  // or we're told to have a stop node but it would be indistinguishable from pause (=>have pause)
  CInputFilter *pInput(static_cast<CInputFilter *>(m_pInterface->GetModuleByName(m_pInterface->GetStringParameter(SP_INPUT_FILTER))));
  if (pInput->supportsPause() && (m_pInterface->hasStopTriggers() || m_pInterface->GetBoolParameter(BP_CONTROL_MODE_HAS_HALT)))
    vRootSuccessors.push_back(m_pPause);
  if (it!=vOldRootSuccessors.end() && *it == m_pPause) it++;

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
    for (vector<NodeTemplate *>::const_iterator it2=parsedNodes().begin(); it2!=parsedNodes().end(); it2++)
      vRootSuccessors.push_back(*it2);
  }
  for (vector<NodeTemplate *>::const_iterator it2=parsedNodes().begin(); it2!=parsedNodes().end(); it2++)
    if (it!=vOldRootSuccessors.end() && *it == *it2) it++;

  //copy anything else (custom) that might have been added...
  while (it != vOldRootSuccessors.end()) vRootSuccessors.push_back(*it++);
}
