// DasherNode.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherNode_h__
#define __DasherNode_h__

#include "../Common/NoClones.h"
#include "DasherTypes.h"
#include "LanguageModel.h"


// CDasherNode represents a rectangle and character 

namespace Dasher {class CDasherNode;class CDasherModel;}
class Dasher::CDasherNode : private NoClones
{
public:
	
	CDasherNode(const CDasherModel& dashermodel, CDasherNode *parent,symbol Symbol, unsigned int igroup, int iphase, Opts::ColorSchemes ColorScheme,int ilbnd,int ihbnd,CLanguageModel *lm, bool ControlChild, int Colour, ControlTree *controltree);
	~CDasherNode();
	bool m_bForce;                     // flag to force a node to be drawn - shouldn't be public
    
	// return private data members - read only 
	CDasherNode ** const Children() const {return m_Children;}
	unsigned int Lbnd() const {return m_iLbnd;}
	bool Alive() {return m_bAlive;}
	bool Control() {return m_bControlChild;}
	bool isSeen() {return m_bSeen;}
	void Seen(bool seen) {m_bSeen=seen;}
	bool NodeIsParent(CDasherNode *oldnode);
	ControlTree* GetControlTree() {return m_controltree;}
	void Kill()  {m_bAlive=0;}
	unsigned int Hbnd() const {return m_iHbnd;}
	unsigned int Group() const {return m_iGroup;}
	symbol Symbol() const {return m_Symbol;}
	unsigned int ChildCount() const {return m_iChildCount;}
	int Phase() const {return m_iPhase;}
	Opts::ColorSchemes Cscheme() const {return m_ColorScheme;}
	int Colour() const {return m_iColour;}
	int GroupColour(int group) const {return m_languagemodel->GetGroupColour(group);}
	std::string GroupLabel(int group) const {return m_languagemodel->GetGroupLabel(group);}
	CDasherNode* Parent() const {return m_pParent;}

	CDasherNode* const Get_node_under(int,myint y1,myint y2,myint smousex,myint smousey); // find node under given co-ords
	void Get_string_under(const int,const myint y1,const myint y2,const myint smousex,const myint smousey,std::vector<symbol>&) const; // get string under given co-ords

	void Push_Node();                                      // give birth to children
	void Recursive_Push_Node(int depth);
	void Delete_children();
	void Delete_dead(CDasherNode* alive);
	void Dump_node() const;                                // diagnostic

	// Set/replace the context
	void SetContext(CContext *pContext);

private:

	const unsigned int m_iLbnd,m_iHbnd;// the cumulative lower and upper bound prob relative to parent
	const unsigned int m_iGroup;       // group membership - e.g. 0=nothing 1=caps 2=punc
	const symbol m_Symbol;             // the character to display
	
	CDasherNode **m_Children;          // pointer to array of children
	unsigned int m_iChildCount;		   // number of children
	
	bool m_bAlive;                     // if true, then display node, else dont bother
	bool m_bControlNode;               // if true, node is a control node
	bool m_bControlChild;              // if true, node is offspring of a control node
	bool m_bSeen;                      // if true, node has been output already
	Opts::ColorSchemes m_ColorScheme;
	int m_iPhase;                      // index for coloring
	int m_iColour;                     // for the advanced colour mode

	const CDasherModel& m_DasherModel;
	CLanguageModel *m_languagemodel;   // pointer to the language model - in future, could be different for each node	
	CDasherNode *m_pParent;             // pointer to parent - only needed to grab parent context
	CContext *m_pContext;
	ControlTree *m_controltree;


};

/////////////////////////////////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////////////////////////////////

using namespace Dasher;
using namespace Opts;
#include "DasherModel.h"
/////////////////////////////////////////////////////////////////////////////

inline CDasherNode::CDasherNode(const CDasherModel& dashermodel, CDasherNode* pParent,symbol Symbol, unsigned int igroup, int iphase, ColorSchemes ColorScheme,int ilbnd,int ihbnd,CLanguageModel *lm, bool ControlChild, int Colour=-1, ControlTree *controltree=0)
	:  m_DasherModel(dashermodel), m_iLbnd(ilbnd), m_iHbnd(ihbnd), m_iGroup(igroup), m_iChildCount(0), m_bAlive(true), m_bControlChild(ControlChild), m_bSeen(false), m_ColorScheme(ColorScheme), m_iPhase(iphase), m_iColour(Colour), m_Symbol(Symbol), m_languagemodel(lm), m_Children(0), m_pParent(pParent), m_pContext(NULL), m_controltree(controltree),m_bForce(false)
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

inline void CDasherNode::Delete_children() 
{
	if (m_Children) {
		unsigned int i; 
		for (i=1;i<m_iChildCount;i++) {
		        if (m_Children!=0 && m_Children[i]!=0) {
			      m_Children[i]->Delete_children();
			      delete m_Children[i];
			}
		}
		delete [] m_Children;
	}
	m_Children=0;	
}

/////////////////////////////////////////////////////////////////////////////

inline CDasherNode::~CDasherNode() 
{
	Delete_children();
	if (m_pContext)
		m_languagemodel->ReleaseContext(m_pContext);
}

/////////////////////////////////////////////////////////////////////////////

inline void CDasherNode::SetContext(CContext *pContext)
{
	if (m_pContext)
		m_languagemodel->ReleaseContext(m_pContext);
	m_pContext = pContext;
}

/////////////////////////////////////////////////////////////////////////////

#endif /* #ifndef __DasherNode_h__ */
