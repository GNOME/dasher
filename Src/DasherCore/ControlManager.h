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

#include "NodeManager.h"
#include "LanguageModelling/LanguageModel.h" // Urgh - we really shouldn't need to know about language models here
#include "DasherModel.h"
#include "DasherNode.h"
#include "Event.h"

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

namespace Dasher {

  class CDasherModel;

  /// \ingroup Model
  /// @{

  /// A node manager which deals with control nodes.
  /// Currently can only have one instance due to use 
  /// of static members for callbacks from expat.
  ///
  class CControlManager : public CNodeManager {
  public:

    class CControlNode {
    public: // Worry about encapsulation later
      std::vector<CControlNode *> vChildren;
      std::string strLabel;
      int iID;
      int iColour;
    };

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

    CControlManager(CNodeCreationManager *pNCManager);
    ~CControlManager();

    ///
    /// Does nothing - control manager isn't reference counted.
    ///

    virtual void Ref() {};
    
    ///
    /// Does nothing - control manager isn't reference counted.
    ///
    
    virtual void Unref() {};

    ///
    /// Get a new root node owned by this manager
    ///

    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, int iOffset);

    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren( CDasherNode *pNode );
    
    ///
    /// Delete any storage alocated for this node
    ///

    virtual void ClearNode( CDasherNode *pNode );
    
    virtual void Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization );
    virtual void Undo( CDasherNode *pNode );

    virtual void Enter(CDasherNode *pNode);
    virtual void Leave(CDasherNode *pNode);

    void RegisterNode( int iID, std::string strLabel, int iColour );
    void ConnectNode(int iChild, int iParent, int iAfter);
    void DisconnectNode(int iChild, int iParent);

    void SetControlOffset(CDasherNode *pNode, int iOffset);

  private:

    static void XmlStartHandler(void *pUserData, const XML_Char *szName, const XML_Char **aszAttr);
    static void XmlEndHandler(void *pUserData, const XML_Char *szName);
    static void XmlCDataHandler(void *pUserData, const XML_Char *szData, int iLength);
    
    int LoadLabelsFromFile(string strFileName, int iFileSize);
    int LoadDefaultLabels();
    int ConnectNodes();

    static int m_iNextID;
    CNodeCreationManager *m_pNCManager;
    CLanguageModel *m_pLanguageModel;
    std::map<int,CControlNode*> m_mapControlMap;

    struct SControlData {
      CControlNode *pControlNode;
    };

    ///Whether we'd temporarily disabled Automatic Speed Control
    ///(if _and only if_ so, should re-enable it when leaving a node)
    bool bDisabledSpeedControl;

  };
  /// @}
}


#endif
