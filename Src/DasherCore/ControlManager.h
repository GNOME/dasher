// ControlManager.h
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

#ifndef __controlmanager_h__
#define __controlmanager_h__

#include "DasherNode.h"
#include "Event.h"
#include "NodeManager.h"
#include "NodeCreationManager.h"
#include "DasherInterfaceBase.h"

#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#ifndef _WIN32_WCE
#include <sys/stat.h>
#endif

#include <string>

#include <expat.h>

using namespace std;

class CNodeCreationManager;

namespace Dasher {

  class CDasherModel;

  /// \ingroup Model
  /// @{

  /// A node manager which deals with control nodes.
  ///
  class CControlBase : public CNodeManager {
  public:

    enum { CTL_ROOT, CTL_STOP, CTL_PAUSE, CTL_MOVE, CTL_MOVE_FORWARD, 
	   CTL_MOVE_FORWARD_CHAR, CTL_MOVE_FORWARD_WORD, CTL_MOVE_FORWARD_LINE,
	   CTL_MOVE_FORWARD_FILE, CTL_MOVE_BACKWARD, CTL_MOVE_BACKWARD_CHAR, 
	   CTL_MOVE_BACKWARD_WORD, CTL_MOVE_BACKWARD_LINE, CTL_MOVE_BACKWARD_FILE,
	   CTL_DELETE, CTL_DELETE_FORWARD, 
	   CTL_DELETE_FORWARD_CHAR, CTL_DELETE_FORWARD_WORD, CTL_DELETE_FORWARD_LINE,
	   CTL_DELETE_FORWARD_FILE, CTL_DELETE_BACKWARD, CTL_DELETE_BACKWARD_CHAR,
	   CTL_DELETE_BACKWARD_WORD, CTL_DELETE_BACKWARD_LINE, CTL_DELETE_BACKWARD_FILE,
	   CTL_USER
    };

    class NodeTemplate;
    
  protected:
    ///Sets the root - should be called by subclass constructor to make
    /// superclass ready for use.
    ///Note, may only be called once, and with a non-null pRoot, or will throw an error message.
    void SetRootTemplate(NodeTemplate *pRoot);
    
    CNodeCreationManager *m_pNCManager;
    
    class CContNode : public CDasherNode {
    public:
      CControlBase *mgr() {return m_pMgr;}
      CContNode(CDasherNode *pParent, int iOffset, unsigned int iLbnd, unsigned int iHbnd, NodeTemplate *pTemplate, CControlBase *pMgr);
      
      bool bShove() {return false;}
      ///
      /// Provide children for the supplied node
      ///
      
      virtual void PopulateChildren();
      virtual int ExpectedNumChildren();
      
      virtual void Output(Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization );
      
      virtual void Enter();
      virtual void Leave();
      
    private:
      NodeTemplate *m_pTemplate;
      CControlBase *m_pMgr;
    };
    
  public:    
    class NodeTemplate {
    public:
      NodeTemplate(const std::string &strLabel, int iColour);
      virtual ~NodeTemplate() {}
      int colour() {return m_iColour;};
      const std::string &label() {return m_strLabel;};
      std::vector<NodeTemplate *> successors;
      virtual void happen(CContNode *pNode) {}
    private:
      std::string m_strLabel;
      int m_iColour;
    };
    
    template <typename T> class MethodTemplate : public NodeTemplate {
    public:
      ///pointer to a function "void X()", that is a member of a T...
      typedef void (T::*Method)();
      MethodTemplate(T *pRecv, const std::string &strLabel, Method f) : NodeTemplate(strLabel,-1),m_pRecv(pRecv),m_f(f) {
      }
      virtual void happen(CContNode *pNode) {
        //invoke pointer-to-member-function m_f on object *m_pRecv!
        (m_pRecv->*m_f)();
      }
    private:
      T *m_pRecv;
      Method m_f;
    };
    
    class EventBroadcast : public NodeTemplate {
    public:
      EventBroadcast(int iEvent, const std::string &strLabel, int iColour);
      virtual void happen(CContNode *pNode);
    private:
      const int m_iEvent;
    };
    
    NodeTemplate *GetRootTemplate();
    
    CControlBase(CNodeCreationManager *pNCManager);

    ///
    /// Get a new root node owned by this manager
    ///

    virtual CDasherNode *GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, int iOffset);

  private:
    NodeTemplate *m_pRoot;

    ///Whether we'd temporarily disabled Automatic Speed Control
    ///(if _and only if_ so, should re-enable it when leaving a node)
    bool bDisabledSpeedControl;
    
  };
  
  ///subclass attempts to recreate interface of previous control manager...
  class COrigNodes : public CControlBase {
  public:
    COrigNodes(CNodeCreationManager *pNCManager, CDasherInterfaceBase *pInterface);
    ~COrigNodes();
    
    //keep these around for now, as this might let Win32/Gtk2 work?
    void RegisterNode( int iID, std::string strLabel, int iColour );
    void ConnectNode(int iChild, int iParent, int iAfter);
    void DisconnectNode(int iChild, int iParent);
    
    class Pause : public NodeTemplate {
    public:
      Pause(const std::string &strLabel, int iColour);
      void happen(CContNode *pNode);
    };
    class Stop : public NodeTemplate {
    public:
      Stop(const std::string &strLabel, int iColour);
      void happen(CContNode *pNode);
    };

  private:
    //For now, make all the loading routines private:
    // they are called from the constructor in the same fashion as in old ControlManager.
    
    // The possibility of loading labels/layouts from different files/formats
    // remains, but is left to alternative subclasses of ControlBase.
    
    ///Attempts to load control labels from specified file.
    /// Returns true for success, false for failure (e.g. no such file!)
    bool LoadLabelsFromFile(string strFileName);
    
    ///Load a default set of labels. Return true for success, or false
    /// if labels already loaded
    bool LoadDefaultLabels();
    
    void ConnectNodes();

    static void XmlStartHandler(void *pUserData, const XML_Char *szName, const XML_Char **aszAttr);
    static void XmlEndHandler(void *pUserData, const XML_Char *szName);
    static void XmlCDataHandler(void *pUserData, const XML_Char *szData, int iLength);

    int m_iNextID;
  protected:
    std::map<int,NodeTemplate *> m_perId;
    CDasherInterfaceBase *m_pInterface;
  };
  
  ///subclass which we actually construct...
  class CControlManager : public CDasherComponent, public COrigNodes {
  public:
    CControlManager(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CNodeCreationManager *pNCManager, CDasherInterfaceBase *pInterface);
    void HandleEvent(CEvent *pEvent);
    
    ///Recomputes which of pause, stop, speak and copy the root control node should have amongst its children.
    /// Automatically called whenever copy-on-stop/speak-on-stop or input filter changes;
    /// subclasses of CDasherInterfaceBase should also call this if
    ///  (a) they override Stop() and hasStopTriggers() with additional actions, if these are enabled/disabled
    ///      and this causes the value returned by hasStopTriggers() to change;
    ///  (b) the values returned by SupportsSpeech() and/or SupportsClipboard() ever change.
    void updateActions();
    ~CControlManager();
    
  private:
    ///group headers, with three children each (all/new/repeat)
    NodeTemplate *m_pSpeech, *m_pCopy;
  };
  /// @}
}


#endif
