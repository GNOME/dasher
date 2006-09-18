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

// CDasherNode represents a rectangle and character 

namespace Dasher {
  class CDasherNode;
  class CDasherModel;
}

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
  void Get_string_under(const int, const myint y1, const myint y2, const myint smousex, const myint smousey, std::vector<symbol> &) const;   // get string under given co-ords

  // Lower and higher bounds, and the range
  inline int Lbnd() const;
  inline int Hbnd() const;

  void SetRange(int iLower, int iUpper) {
    m_iLbnd = iLower;
    m_iHbnd = iUpper;
  };

  inline int Range() const;

  // 'Alive' - this could do with an overhaul
  bool Alive() const {
    return m_bAlive;
  } 

  void Alive(bool b) {
    m_bAlive = b;
  }

  void Kill() {
    m_bAlive = 0;
  }

  // 'Seen' - this could do with an overhaul
  bool isSeen() const {
    return m_bSeen;
  } 

  void Seen(bool seen) {
    m_bSeen = seen;
  }

  symbol Symbol() const {  
    return m_Symbol;  
  } 

  int Colour() const {
    return m_iColour;
  } 

  void SetColour(int iColour) {
    m_iColour = iColour;
  }

  void SetGame(bool bInGame) {
    m_bInGame = bInGame;
  }

  bool GetGame() {
    return m_bInGame;
  }

  // Set/replace the context
  void SetContext(CLanguageModel::Context Context);
  CLanguageModel::Context Context()const;

  bool HasAllChildren() const {
    return m_bHasAllChildren;
  };
  void SetHasAllChildren(bool val) {
    m_bHasAllChildren = val;
  };

  // Get the probability of this node
  double GetProb(int iNormalization);

  bool GetConverted() {
    return m_bConverted;
  };

  // Ensure that this node is marked as being converted, together with
  // all of its ancestors (assuming that unconverted nodes are
  // 'contiguous' at the brances of the tree).
  void ConvertWithAncestors();

  // New stuff
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

  ///
  /// Whether this node shoves or not
  ///

  bool m_bShove;

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

  // Members only useful for debugging purposes
  bool m_bWatchDelete; // Notify when this node is deleted

 private:
  int m_iColour;                // for the advanced colour mode
  int m_iLbnd;
  int m_iHbnd;   // the cumulative lower and upper bound prob relative to parent

  // Information concerning the behaviour of the node
  bool m_bIsActive;             // true if descendent of a root node
  int m_iRefCount;              // reference count if ancestor of (or equal to) root node
  bool m_bAlive;                // if true, then display node, else dont bother
  bool m_bSeen;                 // if true, node has been output already

  // Whether this node has been converted (and can therefore be safely deleted)
  bool m_bConverted;            

  // Whether this is on the game mode path
  bool m_bInGame;
  
  // Information internal to the data structure
  ChildMap m_mChildren;         // pointer to array of children
  bool m_bHasAllChildren;       // true if we haven't deleted any children after instantiating them
  CDasherNode *m_pParent;       // pointer to parent

  // TODO: The following should be included in m_pUserData, as they only apply to nodes managed by CAlphabetManager
  CLanguageModel *m_pLanguageModel;     // pointer to the language model - in future, could be different for each node      
  CLanguageModel::Context m_Context;
  const symbol m_Symbol;        // the character to display


};

/////////////////////////////////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////////////////////////////////

using namespace Dasher;
using namespace Opts;
#include "DasherModel.h"

inline CDasherNode::CDasherNode(CDasherNode *pParent, symbol Symbol, int iphase, ColorSchemes ColorScheme, int ilbnd, int ihbnd, CLanguageModel *lm, int Colour =-1)
  : m_mChildren(), m_Symbol(Symbol) {

  m_iLbnd = ilbnd;
  m_iHbnd = ihbnd;
  m_bIsActive = true;
  m_bHasAllChildren = false;
  m_iRefCount = 0;
  m_bAlive = true;
  m_iColour = Colour;
  m_bSeen = false;
  m_pLanguageModel = lm;
  m_pParent = pParent;
  m_Context = CLanguageModel::nullContext;

  m_bConverted = false;
  m_bInGame = false;
  m_bWatchDelete = false;
}

inline void CDasherNode::SetContext(CLanguageModel::Context Context) {
  if(m_Context)
    m_pLanguageModel->ReleaseContext(m_Context);
  m_Context = Context;
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
  /*    // DJW - please make sure DASHER_ASSERT is implemented on your platform
     DASHER_ASSERT(m_mChildren.size()==0);
     m_mChildren = mChildren; */
  return m_mChildren;
}

inline const CDasherNode::ChildMap &CDasherNode::GetChildren() const {
  return m_mChildren;
}

inline CLanguageModel::Context CDasherNode::Context() const {
  return m_Context;
}

#endif /* #ifndef __DasherNode_h__ */
