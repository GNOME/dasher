// DasherNode.h
//
// Copyright (c) 2001-2004 David Ward

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

// CDasherNode represents a rectangle and character 

namespace Dasher {
  class CDasherNode;
  class CDasherModel;
}

// TODO: Move rendering code into node?

class Dasher::CDasherNode:private NoClones {
 public:
  CDasherNode(CDasherNode * parent, symbol Symbol, int iphase, Opts::ColorSchemes ColorScheme, int ilbnd, int ihbnd, CLanguageModel * lm, int Colour);
  ~CDasherNode();

  typedef std::deque<CDasherNode*> ChildMap;

  ChildMap & Children();
  const ChildMap & GetChildren() const;
  
  unsigned int ChildCount() const {
    return m_mChildren.size();
  }

  CDasherNode *Parent() const {
    return m_pParent;
  }
  
  void SetParent(CDasherNode *pNewParent) {
    m_pParent = pNewParent;
  }

  bool NodeIsParent(CDasherNode * oldnode) const;

  // Orphan Child
  void OrphanChild(CDasherNode * pChild);
  void DeleteNephews(CDasherNode *pChild);
  void Delete_children();
  void Trace() const;           // diagnostic
  CDasherNode *const Get_node_under(int, myint y1, myint y2, myint smousex, myint smousey);   // find node under given co-ords

  // Lower and higher bounds, and the range
  inline int Lbnd() const;
  inline int Hbnd() const;

  void SetRange(int iLower, int iUpper) {
    m_iLbnd = iLower;
    m_iHbnd = iUpper;
  };

  inline int Range() const;

  int Colour() const {
    return m_iColour;
  } 

  void SetColour(int iColour) {
    m_iColour = iColour;
  }


  // Get the probability of this node
  double GetProb(int iNormalization);

  /// Ensure that this node is marked as being converted, together with
  /// all of its ancestors (assuming that unconverted nodes are
  /// 'contiguous' at the brances of the tree).
  /// TODO: replace with a generic 'recursive set flag'?
  
  void ConvertWithAncestors();

  CNodeManager *m_pNodeManager;

  /// Pointer for the node manager to do with as it sees fit :-)
  /// Remember to make sure that the node manager deletes anything it
  /// puts here before the node is destroyed.
  ///
  /// Please put everything in here which isn't required to simply
  /// render the node - basically the only exceptions should be the
  /// node size, colour and display text.
  
  void *m_pUserData;

  ///
  /// Text to display with the node (UTF-8)
  ///

  std::string m_strDisplayText;


  int MostProbableChild() {

    int iMax(0);
    int iCurrent;

    for(ChildMap::iterator it(m_mChildren.begin()); it != m_mChildren.end(); ++it) {
      iCurrent = (*it)->Range();

      if(iCurrent > iMax)
	iMax = iCurrent;
    }
    
    return iMax;
  }

  SGroupInfo *m_pBaseGroup;

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

  void SetFlag(int iFlag, bool bValue) {
    if(bValue)
      m_iFlags = m_iFlags | iFlag;
    else
      m_iFlags = m_iFlags & (~iFlag);

    m_pNodeManager->SetFlag(this, iFlag, bValue);
  }

  ///
  /// Get the value of a flag for this node
  ///

  bool GetFlag(int iFlag) const {
    return (m_iFlags & iFlag);
  }

  // Temporary placeholders:

  void SetShove(bool bShove) {
    m_bShove = bShove;
  }

  bool GetShove() {
    return m_bShove;
  }

 private:
  int m_iColour;                // for the advanced colour mode
  int m_iLbnd;
  int m_iHbnd;   // the cumulative lower and upper bound prob relative to parent

  int m_iRefCount;              // reference count if ancestor of (or equal to) root node

  ChildMap m_mChildren;         // pointer to array of children
  CDasherNode *m_pParent;       // pointer to parent

  // Binary flags representing the state of the node
  int m_iFlags;

  ///
  /// Whether this node shoves or not
  ///

  bool m_bShove;
};

/////////////////////////////////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////////////////////////////////

using namespace Dasher;
using namespace Opts;
#include "DasherModel.h"

inline CDasherNode::CDasherNode(CDasherNode *pParent, symbol Symbol, int iphase, ColorSchemes ColorScheme, int ilbnd, int ihbnd, CLanguageModel *lm, int Colour =-1) {
  
  m_iLbnd = ilbnd;
  m_iHbnd = ihbnd;
  m_iColour = Colour;
  m_pParent = pParent;

  // Default flags
  m_iFlags = NF_ACTIVE | NF_ALIVE;

  m_iRefCount = 0;
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

#endif /* #ifndef __DasherNode_h__ */
