// NodeManagerFactory.h
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

#ifndef __nodemanagerfactory_h__
#define __nodemanagerfactory_h__

namespace Dasher {
  // Forward declaration
  class CDasherNode;

  /// \ingroup Model
  /// @{

  /// Factory class for CNodeManager derived classes. The model here
  /// is that node managers are provided via instances of CDasherNodes
  /// which represent the roots of subtrees. References to the node
  /// managers are then propagated through the child creation process.
  ///
  /// Individual factories may either implement simngleton managers,
  /// which are shared by all nodes in a given class, or separate
  /// manager instances for each subtree, in which case reference
  /// counting is implemented.
  ///
  class CNodeManagerFactory {
  public:
    
    /// Create a root node of the appropriate class, instantiating a
    /// node manager if appropriate.
    ///
    /// @param pParent Parent node for the root, NULL if it is the base of the tree.
    /// @param iLower Lower bound within parent (unused if pParent is NULL).
    /// @param iUpper Upper bound within parent (unused if pParent is NULL).
    /// @param pUserData Subclass specific data used at creation time.
    ///
    /// @return The new root node.
    ///
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) = 0;
  };
  /// @}
}

#endif
