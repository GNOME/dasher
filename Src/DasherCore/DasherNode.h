// DasherNode.h
//
// Copyright (c) 2007 David Ward
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

#ifndef __DasherNode_h__
#define __DasherNode_h__

#include "../Common/Common.h"
#include "../Common/NoClones.h"
#include "DasherTypes.h"
#include "LanguageModelling/LanguageModel.h"
#include "NodeManager.h"
#include "Alphabet/GroupInfo.h"

#include <deque>
#include <iostream>

// Node flag constants
#define NF_COMMITTED 1
#define NF_ACTIVE 2
#define NF_ALIVE 4
#define NF_SEEN 8
#define NF_CONVERTED 16
#define NF_GAME 32
#define NF_ALLCHILDREN 64
#define NF_SUBNODE 128
#define NF_SUPER 256

namespace Dasher {
  class CDasherNode;
  class CDasherModel;
}

/// \ingroup Model
/// @{

/// @brief A node in the Dasher model
///
/// The Dasher node represents a box drawn on the display. This class
/// contains the information required to render the node, as well as
/// navigation within the model (parents and children
/// etc.). Additional information is stored in m_pUserData, which is
/// interpreted by the node manager associated with this class. Any
/// logic specific to a particular node manager should be stored here.
///
/// @todo Encapsulate presentation data in a structure?
/// @todo Check that all methods respect the pseudochild attribute
class Dasher::CDasherNode:private NoClones {
 public:

  /// Display attributes of this node, used for rendering.
  struct SDisplayInfo {
    int iColour;
    bool bShove;
    bool bVisible;
    std::string strDisplayText;
  };

  /// Container type for storing children. Note that it's worth
  /// optimising this as lookup happens a lot
  typedef std::deque<CDasherNode*> ChildMap;

  /// @brief Constructor
  ///
  /// @param pParent Parent of the new node
  /// @param iLbnd Lower bound of node within parent
  /// @param iHbnd Upper bound of node within parent
  /// @param pDisplayInfo Struct containing information on how to display the node
  ///
  inline CDasherNode(CDasherNode *pParent, int iLbnd, int iHbnd, SDisplayInfo *pDisplayInfo);

  /// @brief Destructor
  ///
  ~CDasherNode();

  void Trace() const;           // diagnostic

  /// Return display information for this node
  inline const SDisplayInfo *GetDisplayInfo() const;

  /// @name Routines for manipulating node status
  /// @{

  /// @brief Set a node flag
  ///
  /// Set various flags corresponding to the state of the node. The following flags are defined:
  ///
  /// NF_COMMITTED - Node is 'above' the root, so corresponding symbol
  /// has been added to text box, language model trained etc
  ///
  /// NF_ACTIVE - Node is a decendent of the root node (TODO: Isnt this everything?)
  ///
  /// NF_ALIVE - Node is large enough to be displayed
  ///
  /// NF_SEEN - Node has already been output
  ///
  /// NF_CONVERTED - Node has been converted (eg Japanese mode)
  ///
  /// NF_GAME - Node is on the path in game mode
  ///
  /// NF_ALLCHILDREN - Node has all children (TODO: obsolete?)
  ///
  /// NF_SUBNODE - Node should be considered an integral subnode of parents (eg groups)
  ///
  /// NF_SUPER - Node covers entire visible area
  /// 
  /// @param iFlag The flag to set
  /// @param bValue The new value of the flag
  ///
  void SetFlag(int iFlag, bool bValue);

  /// @brief Get the value of a flag for this node
  ///
  /// @param iFlag The flag to get
  ///
  /// @return The current value of the flag
  ///
  inline bool GetFlag(int iFlag) const;

  /// @brief Recursively mark nodes as converted 
  ///
  /// Ensure that this node is marked as being converted, together with
  /// all of its ancestors (assuming that unconverted nodes are
  /// 'contiguous' at the brances of the tree).
  ///
  /// @todo replace with a generic 'recursive set flag'?
  ///
  void ConvertWithAncestors();

  /// @}

  /// @name Routines relating to the size of the node
  /// @{

  // Lower and higher bounds, and the range

  /// @brief Get the lower bound of a node
  ///
  /// @return The lower bound
  ///
  inline int Lbnd() const;

  /// @brief Get the upper bound of a node
  ///
  /// @return The upper bound
  ///
  inline int Hbnd() const;

  /// @brief Get the range of a node (upper - lower bound)
  ///
  /// @return The range
  ///
  /// @todo Should this be here (trivial arithmethic of existing methods)
  ///
  inline int Range() const;

  /// @brief Reset the range of a node
  ///
  /// @param iLower New lower bound
  /// @param iUpper New upper bound
  ///
  inline void SetRange(int iLower, int iUpper);

  /// @brief Get the probability of a node
  ///
  /// @param iNormalization Normalisation constant
  ///
  /// @return The probability
  ///
  /// @todo Possibly a bit simplistic to make a full member
  /// @todo Inline
  ///
  double GetProb(int iNormalization);

  /// @brief Get the size of the most probable child
  ///
  /// @return The size
  ///
  int MostProbableChild();
  /// @}

  /// @name Routines for manipulating relatives
  /// @{

  inline ChildMap & Children();
  inline const ChildMap & GetChildren() const;
  inline unsigned int ChildCount() const;
  inline CDasherNode *Parent() const;
  void SetParent(CDasherNode *pNewParent); 
  bool NodeIsParent(CDasherNode * oldnode) const;
  // TODO: Should this be here?
  CDasherNode *const Get_node_under(int, myint y1, myint y2, myint smousex, myint smousey);   // find node under given co-ords
  
  /// @brief Orphan a child of this node 
  ///
  /// Deletes all other children, and the node itself
  ///
  /// @param pChild The child to keep
  ///
  void OrphanChild(CDasherNode * pChild);

  /// @brief Delete the nephews of a given child
  ///
  /// @param pChild The child to keep
  ///
  void DeleteNephews(CDasherNode *pChild);

  /// @brief Delete the children of this node
  ///
  ///
  void Delete_children();
  /// @}

  /// \todo Make private, read only access?
  CNodeManager *m_pNodeManager;

  /// Pointer for the node manager to do with as it sees fit.
  /// Remember to make sure that the node manager deletes anything it
  /// puts here before the node is destroyed.
  ///
  /// Please put everything in here which isn't required to simply
  /// render the node - basically the only exceptions should be the
  /// node size, colour and display text.
  /// \todo Make private, read only access?
  void *m_pUserData;

  // A hack, to allow this node to be tied to a particular number of symbols;
  int m_iNumSymbols;
  
 private:
  SDisplayInfo *m_pDisplayInfo;

  int m_iLbnd;
  int m_iHbnd;   // the cumulative lower and upper bound prob relative to parent

  int m_iRefCount;              // reference count if ancestor of (or equal to) root node

  ChildMap m_mChildren;         // pointer to array of children
  CDasherNode *m_pParent;       // pointer to parent

  // Binary flags representing the state of the node
  int m_iFlags;
};
/// @}

/////////////////////////////////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////////////////////////////////

using namespace Dasher;
using namespace Opts;
#include "DasherModel.h"

inline CDasherNode::CDasherNode(CDasherNode *pParent, int iLbnd, int iHbnd, SDisplayInfo *pDisplayInfo) {
  // TODO: Check that these are disabled for debug builds, and that we're not shipping such a build
  DASHER_ASSERT(iHbnd >= iLbnd);
  DASHER_ASSERT(pDisplayInfo != NULL);

  m_pParent = pParent;  
  m_iLbnd = iLbnd;
  m_iHbnd = iHbnd;
  m_pDisplayInfo = pDisplayInfo;

  // Default flags (make a definition somewhere, pass flags to constructor)
  m_iFlags = NF_ACTIVE | NF_ALIVE;

  m_iRefCount = 0;
  m_iNumSymbols = 0;
}

inline const CDasherNode::SDisplayInfo *CDasherNode::GetDisplayInfo() const {
  return m_pDisplayInfo;
}

inline int CDasherNode::Lbnd() const {
  return m_iLbnd;
}

inline int CDasherNode::Hbnd() const {
  return m_iHbnd;
}

inline int CDasherNode::Range() const {
  return m_iHbnd - m_iLbnd;
}

inline CDasherNode::ChildMap &CDasherNode::Children() {
  return m_mChildren;
}

inline const CDasherNode::ChildMap &CDasherNode::GetChildren() const {
  return m_mChildren;
}

inline unsigned int CDasherNode::ChildCount() const {
  return m_mChildren.size();
}

inline bool CDasherNode::GetFlag(int iFlag) const {
  return (m_iFlags & iFlag);
}

inline CDasherNode *CDasherNode::Parent() const {
  return m_pParent;
}

inline void CDasherNode::SetRange(int iLower, int iUpper) {
  m_iLbnd = iLower;
  m_iHbnd = iUpper;
}
  

#endif /* #ifndef __DasherNode_h__ */
