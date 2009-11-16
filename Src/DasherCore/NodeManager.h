// NodeManager.h
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

#ifndef __nodemanager_h__
#define __nodemanager_h__
#include "LanguageModelling/LanguageModel.h"
#include "DasherTypes.h"

namespace Dasher {
  class CDasherNode; // Forward declaration

  /// \ingroup Model 
  /// @{

  /// The node management process assigns an instance of a subclass of
  /// CNodeManager to each Dasher node in the tree. Typically a
  /// contiguous subtree will be represented by a single CNodeManager,
  /// but multiple subtrees will often share the same
  /// CNodeManager. CNodeManagers implement reference counting to
  /// ensure that they are deleted when no longer required.
  ///
  /// This mechanism allows broad classes of nodes to be defined with
  /// their own behaviour, for example, regular 'alphabet' nodes are
  /// managed by an instance of CAlphabetManager. CNodeManagers also
  /// currently exist for control nodes, and subtrees representing the
  /// conversion process in certain languages.
  ///
  /// The node manager is responsible for the following aspects of the
  /// lifespan of a node:
  ///
  /// 1. Construction of the root node for the subtree.
  ///
  /// 2. Population of the children of a node, and regeneration of the
  /// parent node where necessary
  ///
  /// 3. Behaviour when the node enters or leaves the crosshair.
  ///
  /// 4. Behaviour when the node's flag status changes.
  ///
  /// 5. Deletion of manager-specific data when the node is deleted.
  ///
  /// The CDasherNode class provides a 'user data' pointer which is
  /// intended for the node manager to use as a store for any
  /// manager-specific status information.
  ///
  class CNodeManager {
  public:
    CNodeManager(int iID) {
      m_iID = iID;
    };

    virtual int GetID() {
      return m_iID;
    };
    
    /// Increment reference count
    /// ACL 12/8/09 Never called polymorphically, so removing.
    //virtual void Ref() = 0;
    
    /// Decrement reference count
    ///
    virtual void Unref() = 0;

    ///
    /// Get a new root node owned by this manager
    /// ACL 12/8/09 this is never called polymorphically;
    /// hence deleting it so subclasses can each use different types
    //virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) = 0;

    ///
    /// Provide children for the supplied node
    ///

    virtual void PopulateChildren( CDasherNode *pNode ) = 0;
    
    ///
    /// Delete any storage alocated for this node
    ///

    virtual void ClearNode( CDasherNode *pNode ) = 0;

    ///
    /// Called whenever a node belonging to this manager first 
    /// moves under the crosshair
    ///

    virtual void Output( CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int iNormalization) {};
    virtual void Undo( CDasherNode *pNode ) {};

    virtual void Enter(CDasherNode *pNode) {};
    virtual void Leave(CDasherNode *pNode) {};

    virtual CDasherNode *RebuildParent(CDasherNode *pNode) {
      return 0;
    }

    virtual void SetFlag(CDasherNode *pNode, int iFlag, bool bValue) {};

    virtual void SetControlOffset(CDasherNode *pNode, int iOffset) {};
    
    ///
    /// See if this node, or *if an NF_SUBNODE* a descendant (recursively),
    /// represents the specified alphanumeric character; if so, set it's NF_GAME flag and
    /// return true; otherwise, return false.
    ///
    virtual bool GameSearchNode(CDasherNode *pNode, std::string strTargetUtf8Char) {return false;}

    /// Clone the context of the specified node, if it's an alphabet node;
    /// else return an empty context. (Used by ConversionManager)
    virtual CLanguageModel::Context CloneAlphContext(CDasherNode *pNode, CLanguageModel *pLanguageModel) {
      return pLanguageModel->CreateEmptyContext();
    };
    
    virtual symbol GetAlphSymbol(CDasherNode *pNode) {
      throw "Hack for pre-MandarinDasher ConversionManager::BuildTree method, needs to access CAlphabetManager-private struct";
    }

  private:
    int m_iID;
  };
  /// @}
}

#endif
