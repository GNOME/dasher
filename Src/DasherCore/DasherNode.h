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
#include "LanguageModelling/LanguageModel.h"


// CDasherNode represents a rectangle and character 

namespace Dasher {class CDasherNode;class CDasherModel;}
class Dasher::CDasherNode : private NoClones
{
public:
	
	CDasherNode(const CDasherModel& dashermodel, CDasherNode *parent,symbol Symbol, 
		int iphase, Opts::ColorSchemes ColorScheme,
		int ilbnd,int ihbnd,
		CLanguageModel *lm, bool ControlChild, int Colour, ControlTree *controltree);

	~CDasherNode();
	
	// Node relationships
	CDasherNode ** const Children() const;
	unsigned int ChildCount() const {return m_iChildCount;}
	CDasherNode* Parent() const {return m_pParent;}
	bool NodeIsParent(CDasherNode *oldnode) const;
	void SetChildren(CDasherNode** ppChildren, int iChildCount);
	
	// Orphan Child
	void OrphanChild(CDasherNode* pChild);
	
	void DeleteNephews(int iChild);

	// Lower and higher bounds, and the range
	int Lbnd() const;
	int Hbnd() const;
	int Range() const;
	
	// 'Alive' - this could do with an overhaul
	bool Alive() const {return m_bAlive;}
	void Alive(bool b) {m_bAlive = b;}
	void Kill()  {m_bAlive=0;}

	// 'Seen' - this could do with an overhaul
	bool isSeen() const {return m_bSeen;}
	void Seen(bool seen) {m_bSeen=seen;}
	
	// ControlNode-related
	bool ControlChild() const {return m_bControlChild;}
	ControlTree* GetControlTree() {return m_controltree;}
	
	//unsigned int Group() const {return m_iGroup;}
	symbol Symbol() const {return m_Symbol;}
	int Phase() const {return m_iPhase;}

	// DJW Sort out the colour scheme / phase confusion
	Opts::ColorSchemes ColorScheme() const {return m_ColorScheme;}
	int Colour() const {return m_iColour;}
	
	CDasherNode* const Get_node_under(int,myint y1,myint y2,myint smousex,myint smousey); // find node under given co-ords
	void Get_string_under(const int,const myint y1,const myint y2,const myint smousex,const myint smousey,std::vector<symbol>&) const; // get string under given co-ords

	void Delete_children();
	void Trace() const;                                // diagnostic

	// Set/replace the context
	void SetContext(CLanguageModel::Context Context);
	CLanguageModel::Context Context() const;

private:

	const int m_iLbnd,m_iHbnd;// the cumulative lower and upper bound prob relative to parent
	//const unsigned int m_iGroup;       // group membership - e.g. 0=nothing 1=caps 2=punc
	const symbol m_Symbol;             // the character to display
	
	CDasherNode** m_ppChildren;          // pointer to array of children
	unsigned int m_iChildCount;		   // number of children
	
	bool m_bAlive;                     // if true, then display node, else dont bother
	//bool m_bControlNode;               // if true, node is a control node
	bool m_bControlChild;              // if true, node is offspring of a control node
	bool m_bSeen;                      // if true, node has been output already
	Opts::ColorSchemes m_ColorScheme;
	int m_iPhase;                      // index for coloring
	int m_iColour;                     // for the advanced colour mode

	const CDasherModel& m_DasherModel;

	// Language Modelling 
	CLanguageModel* m_pLanguageModel;   // pointer to the language model - in future, could be different for each node	
	CLanguageModel::Context m_Context;

	CDasherNode *m_pParent;             // pointer to parent
	ControlTree *m_controltree;

	enum {
		typeRoot=0,
		typeSymbol=1
	};

namespace Dasher {
  class CDasherNode;
  class CDasherInterfaceBase;
}
#include <deque>
#include <iostream>
#include <vector>

// Node flag constants
/// NF_COMMITTED: The node is 'above' the root, i.e. all onscreen parts of it
/// are covered by the root; this is when we train the language model etc
#define NF_COMMITTED 1

/// NF_SEEN - Node is under the crosshair and has (already) been output
#define NF_SEEN 2

/// NF_CONVERTED - Node has been converted (eg Japanese mode)
#define NF_CONVERTED 4

/// NF_GAME - Node is on the path in game mode
#define NF_GAME 8

/// NF_ALLCHILDREN - Node has all children. TODO Since nodes only
/// ever have all their children, or none of them, can we not
/// just check it has children, and get rid of this flag?
#define NF_ALLCHILDREN 16

/// NF_SUPER - Node covers entire visible area (and so is eligible
/// to be made the new root)
#define NF_SUPER 32

/// NF_VISIBLE - an invisible node is one which lets its parent's
/// colour show through, and has no outline drawn round it (it may
/// still have a label). Note that this flag is set (i.e. the node
/// is drawn and outlined) by default in the constructor.
#define NF_VISIBLE 64

///Flags to assign to a newly created node:
#define DEFAULT_FLAGS NF_VISIBLE

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
  /// Colour; note invisible nodes just have the same colour as their parent.
  /// (so we know what colour to use when their parents are deleted)
  inline int getColour() {return m_iColour;}
  virtual CDasherScreen::Label *getLabel() { return m_pLabel; }
  ///Whether labels on child nodes should be displaced to the right of this node's label.
  /// (Default implementation returns true, subclasses should override if appropriate)
  virtual bool bShove() {return true;}

  ///Multiplier to apply to the speed (in dynamic modes, inc. default mouse
  /// control) when the crosshair is inside this node (but not inside any child.)
  /// This creates a sort of "viscosity", i.e. makes some nodes harder to move
  /// through than others - used to slow down movement inside control nodes,
  /// making it harder to make mistakes therein. The default just returns 1.0,
  /// i.e. no change.
  virtual double SpeedMul() {return 1.0;}
  
  inline int offset() const {return m_iOffset;}
  CDasherNode *onlyChildRendered; //cache that only one child was rendered (as it filled the screen)

  /// Container type for storing children. Note that it's worth
  /// optimising this as lookup happens a lot
  typedef std::deque<CDasherNode*> ChildMap;

  /// @brief Constructor
  ///
  /// Note the flags of the new node are initialized to DEFAULT_FLAGS,
  /// and the node has no parent, and range 0-CDasherModel::NORMALIZATION:
  /// caller will likely want to change these via a call to Reparent().
  ///
  /// \param pParent Parent of the new node; automatically adds to end of parent's child list
  /// \param iOffset Index into text buffer of character to LHS of cursor _after_ this node is Output().
  /// \param iColour background colour of node (for transparent nodes, same colour as parent)
  /// \param pLabel label to render onto node, NULL if no label required.
  CDasherNode(int iOffset, int iColour, CDasherScreen::Label *pLabel);

  /// @brief Destructor
  ///
  virtual ~CDasherNode();

  void Trace() const;           // diagnostic

  /// @name Routines for manipulating node status
  /// @{

  /// @brief Set a node flag
  ///
  /// Set various flags corresponding to the state of the node. 
  /// 
  /// @param iFlag The flag to set
  /// @param bValue The new value of the flag
  ///
  virtual void SetFlag(int iFlag, bool bValue);

  /// @brief Get the value of a flag for this node
  ///
  /// @param iFlag The flag to get
  ///
  /// @return The current value of the flag
  ///
  inline bool GetFlag(int iFlag) const;

  /// @}

  /// @name Routines relating to the size of the node
  /// @{

  // Lower and higher bounds, and the range

  /// @brief Get the lower bound of a node
  ///
  /// @return The lower bound
  ///
  inline unsigned int Lbnd() const;

  /// @brief Get the upper bound of a node
  ///
  /// @return The upper bound
  ///
  inline unsigned int Hbnd() const;

  /// @brief Get the range of a node (upper - lower bound)
  ///
  /// @return The range
  ///
  /// @todo Should this be here (trivial arithmethic of existing methods)
  ///
  inline unsigned int Range() const;

  /// @brief Get the size of the most probable child
  ///
  /// @return The size
  ///
  int MostProbableChild();
  /// @}

  /// @name Routines for manipulating relatives
  /// @{

  inline const ChildMap & GetChildren() const;
  inline unsigned int ChildCount() const;
  inline CDasherNode *Parent() const;
  
  /// Makes the node be the child of a new parent, and set its range amongst
  /// that parent's children. This node will be positioned AFTER any/all
  /// existing children of the new parent; so TODO - iLower redundant?
  /// Before the call is made, the (child) node must have no parent.
  void Reparent(CDasherNode *pNewParent, unsigned int iLower, unsigned int iUpper);
  
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

  ///
  /// Sees if a *child* / descendant of the specified node (not that node itself)
  /// represents the specified character. If so, set the child & intervening nodes'
  /// NF_GAME flag, and return true; otherwise, return false.
  ///
  bool GameSearchChildren(symbol sym);

  /// @name Management routines (once accessed via NodeManager)
  /// @{
  /// Gets the node manager for this object. Meaning defined by subclasses,
  ///  which should override and refine the return type appropriately;
  ///  the main use is to provide runtime type info to check casting!
  virtual CNodeManager *mgr() const = 0;
  ///
  /// Provide children for the supplied node
  ///

  virtual void PopulateChildren() = 0;

  /// The number of children which a call to PopulateChildren can be expected to generate.
  /// (This is not required to be 100% accurate, but any discrepancies will likely cause
  /// the node budgetting algorithm to behave sub-optimally)
  virtual int ExpectedNumChildren() = 0;

  ///
  /// Called whenever a node belonging to this manager first
  /// moves under the crosshair
  ///

  virtual void Output() {};
  virtual void Undo() {};

  ///Called by logging code to get information about node which has just been
  /// output. Subclasses performing output, should override to return appropriate
  /// information. (Will only be called if necessary i.e. logging enabled).
  /// The default returns a very uninformative SymbolProb with just display text.
  virtual SymbolProb GetSymbolProb() const {
    return SymbolProb(0,m_pLabel->m_strText,0.0);
  }

  virtual CDasherNode *RebuildParent() {
    return 0;
  };

  ///
  /// Get as many symbols of context, up to the _end_ of the specified range,
  /// as possible from this node and its uncommitted ancestors
  ///
  virtual void GetContext(CDasherInterfaceBase *pInterface, const CAlphabetMap *pAlphabetMap, std::vector<symbol> &vContextSymbols, int iOffset, int iLength);

  ///
  /// See if this node represents the specified symbol; if so, set it's NF_GAME flag and
  /// return true; otherwise, return false.
  ///
  virtual bool GameSearchNode(symbol sym) {return false;}

  virtual symbol GetAlphSymbol() {
    throw "Hack for pre-MandarinDasher ConversionManager::BuildTree method, needs to access CAlphabetManager-private struct";
  }

  /// @}

 private:
  inline ChildMap &Children();

  unsigned int m_iLbnd;
  unsigned int m_iHbnd;   // the cumulative lower and upper bound prob relative to parent

  ChildMap m_mChildren;         // pointer to array of children
  CDasherNode *m_pParent;       // pointer to parent

  // Binary flags representing the state of the node
  int m_iFlags;

  int m_iOffset;

 protected:
  const int m_iColour;
  CDasherScreen::Label * m_pLabel;
};
/// @}

namespace Dasher {
  /// Return the number of CDasherNode objects currently in existence.
  int currentNumNodeObjects();
}


/////////////////////////////////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////////////////////////////////

namespace Dasher {

inline CDasherNode::CDasherNode(const CDasherModel& dashermodel, CDasherNode* pParent,symbol Symbol, int iphase, ColorSchemes ColorScheme,int ilbnd,int ihbnd,CLanguageModel *lm, bool ControlChild, int Colour=-1, ControlTree *controltree=0)
	:  m_DasherModel(dashermodel), m_iLbnd(ilbnd), m_iHbnd(ihbnd), 
	m_iChildCount(0), m_bAlive(true), m_bControlChild(ControlChild), m_bSeen(false), 
	m_ColorScheme(ColorScheme), m_iPhase(iphase), m_iColour(Colour), m_Symbol(Symbol), 
	m_pLanguageModel(lm), m_ppChildren(0), m_pParent(pParent), m_Context(NULL), 
	m_controltree(controltree)
{

	/*
	switch (ColorScheme) {
		case Nodes1:
			m_ColorScheme = Nodes2;
			break;
		case Nodes2:
			m_ColorScheme = Nodes1;
			break;
		case Special1:
			m_ColorScheme = Special2;
			break;
		case Special2:
			m_ColorScheme = Special1;
			break;
		case default:
			m_ColorScheme = ColorScheme;
			break;
	}
	*/
}

inline unsigned int CDasherNode::Hbnd() const {
  return m_iHbnd;
}

inline unsigned int CDasherNode::Range() const {
  return m_iHbnd - m_iLbnd;
}

/////////////////////////////////////////////////////////////////////////////

inline	int CDasherNode::Lbnd() const {return m_iLbnd;}

/////////////////////////////////////////////////////////////////////////////

inline	int CDasherNode::Hbnd() const {return m_iHbnd;}

/////////////////////////////////////////////////////////////////////////////

inline	int CDasherNode::Range() const {return m_iHbnd-m_iLbnd;}

/////////////////////////////////////////////////////////////////////////////

inline void CDasherNode::SetChildren(CDasherNode** ppChildren, int iChildCount)
{
	// DJW - please make sure DASHER_ASSERT is implemented on your platform
	DASHER_ASSERT(m_ppChildren == NULL);
	m_ppChildren = ppChildren;
	m_iChildCount = iChildCount;
}

/////////////////////////////////////////////////////////////////////////////

inline CDasherNode** const CDasherNode::Children() const
{
	return m_ppChildren;
}

inline unsigned int CDasherNode::ChildCount() const {
  return m_mChildren.size();
}

inline bool CDasherNode::GetFlag(int iFlag) const {
  return ((m_iFlags & iFlag) != 0);
}

inline CDasherNode *CDasherNode::Parent() const {
  return m_pParent;
}

}
#endif /* #ifndef __DasherNode_h__ */
