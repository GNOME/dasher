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

  class CDasherInterfaceBase;

  /// \ingroup Model
  /// @{

  /// A node manager which deals with control nodes.
  ///
  class CControlBase : public CNodeManager {
  public:

    class NodeTemplate;

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

      virtual void Output();

      virtual void Enter();
      virtual void Leave();

    private:
      NodeTemplate *m_pTemplate;
      CControlBase *m_pMgr;
    };

    class Action {
    public:
      virtual void happen(CContNode *pNode) {}
    };
    class NodeTemplate : public Action {
    public:
      NodeTemplate(const std::string &strLabel, int iColour);
      virtual ~NodeTemplate();
      const std::string m_strLabel;
      const int m_iColour;
      std::vector<NodeTemplate *> successors;
    private:
      friend class CControlBase;
      CDasherScreen::Label *m_pLabel;
    };

    template <typename T> class MethodTemplate : public NodeTemplate {
    public:
      ///A "Method" is pointer to a function "void X()", that is a member of a T...
      typedef void (T::*Method)();
      MethodTemplate(const std::string &strLabel, int color, T *pRecv, Method f) : NodeTemplate(strLabel,color),m_pRecv(pRecv),m_f(f) {
      }
      virtual void happen(CContNode *pNode) {
        //invoke pointer-to-member-function m_f on object *m_pRecv!
        (m_pRecv->*m_f)();
      }
    private:
      T *m_pRecv;
      Method m_f;
    };

    NodeTemplate *GetRootTemplate();

    CControlBase(CNodeCreationManager *pNCManager);

    ///Make this manager ready to make nodes renderable on the screen by preallocating labels
    virtual void MakeLabels(CDasherScreen *pScreen);
    
    ///
    /// Get a new root node owned by this manager
    ///

    virtual CDasherNode *GetRoot(CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, int iOffset);

  protected:
    ///Sets the root - should be called by subclass constructor to make
    /// superclass ready for use.
    ///Note, may only be called once, and with a non-null pRoot, or will throw an error message.
    void SetRootTemplate(NodeTemplate *pRoot);

    CNodeCreationManager *m_pNCManager;

  private:
    NodeTemplate *m_pRoot;

    ///Whether we'd temporarily disabled Automatic Speed Control
    ///(if _and only if_ so, should re-enable it when leaving a node)
    bool bDisabledSpeedControl;

  };

  ///Class reads node tree definitions from an XML file, linking together the NodeTemplates
  /// according to defined names, nesting of <node/>s, and  <ref/>s. Also handles the
  /// <alph/> tag, meaning one child of the node is to escape back to the alphabet. Subclasses
  /// may override parseAction to provide actions for the nodes to perform, also parseOther
  /// to link with NodeTemplates from other sources.
  class CControlParser {
  protected:
    ///Loads all node definitions from the specified filename, adding them to
    /// any loaded from previous calls. (However, files processed independently:
    /// e.g. names defined in one file will not be seen from another)
    /// \param strFilename name+full-path of xml file to load
    /// \return true if the file was opened successfully; false if not.
    bool LoadFile(const std::string &strFilename);
    /// \return all node definitions that have been loaded by this CControlParser.
    const vector<CControlBase::NodeTemplate*> &parsedNodes();
    ///Subclasses may override to parse other nodes (besides "node", "ref" and "alph").
    ///The default implementation always returns NULL.
    /// \return A node template, if the name was recognised; NULL if not recognised.
    virtual CControlBase::NodeTemplate *parseOther(const XML_Char *name, const XML_Char **atts) {
      return NULL;
    }
    ///Subclasses may override to parse actions within nodes.
    ///The default implementation always returns NULL.
    /// \return A (new) action pointer, if the name+attributes were successfully parsed; NULL if not recognised.
    virtual CControlBase::Action *parseAction(const XML_Char *name, const XML_Char **atts) {
      return NULL;
    };
    //TODO cleanup/deletion
  private:
    ///all top-level parsed nodes
    vector<CControlBase::NodeTemplate *> m_vParsed;
  };

  ///subclass which we actually construct! Parses editing node definitions from a file,
  /// then adds Pause and/or Stop, Speak, and Copy (to clipboard), all as children
  /// of the "root" control node.
  class CControlManager : public CDasherComponent, public CControlBase, public CControlParser {
  public:
    class Pause : public NodeTemplate {
    public:
      Pause(const std::string &strLabel, int iColour);
      void happen(CContNode *pNode);
    };
    CControlManager(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CNodeCreationManager *pNCManager, CDasherInterfaceBase *pInterface);
    void HandleEvent(CEvent *pEvent);

    typedef enum {
      EDIT_CHAR, EDIT_WORD, EDIT_LINE, EDIT_FILE
    } EditDistance;

    ///Recomputes which of pause, stop, speak and copy the root control node should have amongst its children.
    /// Automatically called whenever copy-on-stop/speak-on-stop or input filter changes;
    /// subclasses of CDasherInterfaceBase should also call this if
    ///  (a) they override Stop() and hasStopTriggers() with additional actions, if these are enabled/disabled
    ///      and this causes the value returned by hasStopTriggers() to change;
    ///  (b) the values returned by SupportsSpeech() and/or SupportsClipboard() ever change.
    void updateActions();
    ~CControlManager();

  protected:
    ///Override to allow a <root/> tag to include a fresh control root
    NodeTemplate *parseOther(const XML_Char *name, const XML_Char **atts);
    ///Override to recognise <move/> and <delete/> tags as actions.
    Action *parseAction(const XML_Char *name, const XML_Char **atts);

  private:
    NodeTemplate *m_pPause, *m_pStop;
    CDasherInterfaceBase *m_pInterface;
    ///group headers, with three children each (all/new/repeat)
    NodeTemplate *m_pSpeech, *m_pCopy;
  };
  /// @}
}


#endif
