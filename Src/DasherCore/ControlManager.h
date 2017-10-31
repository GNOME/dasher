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

#include <list>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <string>

#include <expat.h>

using namespace std;

class CNodeCreationManager;

namespace Dasher {

  class CDasherInterfaceBase;
  class SpeechHeader;
  class CopyHeader;

  /// \ingroup Model
  /// @{

  /// A node manager which deals with control nodes.
  ///
  class CControlBase : public CNodeManager, protected CSettingsUser {
  public:

    class NodeTemplate;

    class CContNode : public CDasherNode {
    public:
      CControlBase *mgr() const override {return m_pMgr;}
      CContNode(int iOffset, int iColour, NodeTemplate *pTemplate, CControlBase *pMgr);
      CDasherScreen::Label *getLabel() override { return m_pTemplate->m_pLabel; }

      bool bShove() override {return false;}
      double SpeedMul() override;
      ///
      /// Provide children for the supplied node
      ///

      virtual void PopulateChildren() override;
      virtual int ExpectedNumChildren() override;

      virtual void Output() override;

    private:
      NodeTemplate *m_pTemplate;
      CControlBase *m_pMgr;
    };

    class Action {
    public:
      virtual ~Action() { };
      virtual int calculateNewOffset(CControlBase::CContNode *pNode, int offsetBefore) { return offsetBefore; }
        virtual void happen(CContNode *pNode) {}
    };
    class NodeTemplate : public Action {
    public:
      NodeTemplate(const std::string &strLabel, int iColour);
      virtual ~NodeTemplate();
      const std::string m_strLabel;
      const int m_iColour;
      std::list<NodeTemplate *> successors;

    private:
      friend class CControlBase;
      CDasherScreen::Label *m_pLabel;
    };

    NodeTemplate *GetRootTemplate();

    CControlBase(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager);

    ///Make this manager ready to make nodes renderable on the screen by preallocating labels
    virtual void ChangeScreen(CDasherScreen *pScreen);
    
    ///
    /// Get a new root node owned by this manager
    ///

    virtual CDasherNode *GetRoot(CDasherNode *pContext, int iOffset);
    CDasherInterfaceBase* GetDasherInterface() { return m_pInterface; }
  protected:
    ///Sets the root - should be called by subclass constructor to make
    /// superclass ready for use.
    ///Note, may only be called once, and with a non-null pRoot, or will throw an error message.
    void SetRootTemplate(NodeTemplate *pRoot);

    CDasherInterfaceBase *m_pInterface;
    CNodeCreationManager *m_pNCManager;
    
    int getColour(NodeTemplate *pTemplate, CDasherNode *pParent);
    CDasherScreen *m_pScreen;
    
  private:
    NodeTemplate *m_pRoot;
  };

  ///Class reads node tree definitions from an XML file, linking together the NodeTemplates
  /// according to defined names, nesting of <node/>s, and  <ref/>s. Also handles the
  /// <alph/> tag, meaning one child of the node is to escape back to the alphabet. Subclasses
  /// may override parseAction to provide actions for the nodes to perform, also parseOther
  /// to link with NodeTemplates from other sources.
  class CControlParser : public AbstractXMLParser {
  public:
    CControlParser(CMessageDisplay *pMsgs);
    ///Loads all node definitions from the specified filename. Note that
    /// system files will not be loaded if user files are (and user files will
    /// clear out any nodes from system ones). However, multiple system or multiple
    /// user files, will be concatenated. (However, files are processed separately:
    /// e.g. names defined in one file will not be seen from another)
    /// \param strFilename name+full-path of xml file to load
    /// \param bUser true if from user-specific location (takes priority over system)
    /// \return true if the file was opened successfully; false if not.
    bool ParseFile(const std::string &strFilename, bool bUser);
  protected:
    /// \return all node definitions that have been loaded by this CControlParser.
    const list<CControlBase::NodeTemplate*> &parsedNodes();
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
    void XmlStartHandler(const XML_Char *name, const XML_Char **atts);
    void XmlEndHandler(const XML_Char *szName);
  private:
    ///all top-level parsed nodes
    std::list<CControlBase::NodeTemplate *> m_vParsed;
    ///whether parsed nodes were from user file or not
    bool m_bUser;

    ///Following only used as temporary variables during parsing...
    map<string,CControlBase::NodeTemplate*> namedNodes;
    list<pair<CControlBase::NodeTemplate**,string> > unresolvedRefs;
    list<CControlBase::NodeTemplate*> nodeStack;
  };

  ///subclass which we actually construct! Parses editing node definitions from a file,
  /// then adds Pause and/or Stop, Speak, and Copy (to clipboard), all as children
  /// of the "root" control node.
  class CControlManager : public CSettingsObserver, public CControlBase, public CControlParser {
  public:
    CControlManager(CSettingsUser *pCreateFrom, CNodeCreationManager *pNCManager, CDasherInterfaceBase *pInterface);
    void HandleEvent(int iParameter);

    typedef enum {
      EDIT_CHAR, EDIT_WORD, EDIT_SENTENCE, EDIT_PARAGRAPH, EDIT_FILE, EDIT_LINE, EDIT_PAGE, EDIT_SELECTION,
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
    map<string, CControlBase::Action*> m_actions;
    ///group of statefull actions (all/new/repeat/...)
    SpeechHeader *m_pSpeech;
    CopyHeader *m_pCopy;
  };
  /// @}

  class CControlBoxIO : public AbstractXMLParser {
  public:
    CControlBoxIO(CMessageDisplay *pMsgs);
    void GetControlBoxes(std::vector < std::string > *pList) const;
    CControlManager* CreateControlManager(const std::string& id, CSettingsUser *pCreateFrom, CNodeCreationManager *pNCManager, CDasherInterfaceBase *pInterface) const;
    bool ParseFile(const std::string &strFilename, bool bUser) override;
    void XmlStartHandler(const XML_Char *name, const XML_Char **atts) override;
    void XmlEndHandler(const XML_Char *szName) override {};
  private:
    std::map<std::string, std::string> m_controlFiles;
    std::string m_filename;
  };
}


#endif
