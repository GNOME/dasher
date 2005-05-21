// DasherNode.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherNode_h__
#define __DasherNode_h__

#include "../Common/Common.h"
#include "../Common/NoClones.h"
#include "DasherTypes.h"
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

};

/////////////////////////////////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////////////////////////////////

using namespace Dasher;
using namespace Opts;
#include "DasherModel.h"
/////////////////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////////////////

inline CDasherNode::~CDasherNode() 
{
	Delete_children();
	if (m_Context)
		m_pLanguageModel->ReleaseContext(m_Context);
}

/////////////////////////////////////////////////////////////////////////////

inline void CDasherNode::SetContext(CLanguageModel::Context Context)
{
	if (m_Context)
		m_pLanguageModel->ReleaseContext(m_Context);
	m_Context = Context;
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

/////////////////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CDasherNode::Context() const
{
	return m_Context;
}

/////////////////////////////////////////////////////////////////////////////

#endif /* #ifndef __DasherNode_h__ */
