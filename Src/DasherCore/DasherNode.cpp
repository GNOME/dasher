// DasherNode.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "DasherNode.h"
using namespace Dasher;
using namespace Opts;
using namespace std;

CDasherNode::CDasherNode(CDasherNode *parent,symbol Symbol, unsigned int igroup, int iphase, ColorSchemes ColorScheme,int ilbnd,int ihbnd,CLanguageModel *lm)
	: m_parent(parent),m_Symbol(Symbol),m_iGroup(igroup),m_iLbnd(ilbnd),m_iHbnd(ihbnd),m_languagemodel(lm),m_iPhase(iphase),
	m_context(0), m_iAge(0), m_bAlive(1), m_Children(0), m_bForce(false), m_iChars(0), m_ColorScheme(ColorScheme)
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


void CDasherNode::Delete_children() 
{
	if (m_Children) {
		unsigned int i; 
		for (i=1;i<m_iChars;i++)
			delete m_Children[i];
		delete [] m_Children;
	}
	m_Children=0;
	
}


CDasherNode::~CDasherNode() 
{
	Delete_children();
	if (m_context)
		m_languagemodel->ReleaseNodeContext(m_context);
}


void CDasherNode::Dump_node () const
{ 	
	/* TODO sort out
	dchar out[256];
	if (m_Symbol)
		wsprintf(out,TEXT("%7x %3c %7x %5d %7x  %5d %8x %8x      \n"),this,m_Symbol,m_iGroup,m_context,m_Children,m_Cscheme,m_iLbnd,m_iHbnd);
	else
		wsprintf(out,TEXT("%7x     %7x %5d %7x  %5d %8x %8x      \n"),this,m_iGroup,m_context,m_Children,m_Cscheme,m_iLbnd,m_iHbnd);
	
	OutputDebugString(out);
	
	if (m_Children) {
		unsigned int i; 
		for (i=1;i<m_iChars;i++)
			m_Children[i]->Dump_node();
	}
	*/
}

//TODO: There is a lot of copy-and-pasted code shared between two Push_Node functions
// need to rationalise.
void CDasherNode::Push_Node(CLanguageModel::CNodeContext *context) 
// push a node copying the specified context
{
	if (m_Children) {
		// if there are children just give them a poke
		unsigned int i;
		for (i=1;i<m_iChars;i++) {
			m_Children[i]->m_iAge=0;
			m_Children[i]->m_bAlive=true;
		}
		return;
	}
	
	// if we haven't got a context then try to get a new one
	m_context=m_languagemodel->CloneNodeContext(context);
	// if it fails, be patient
	if (!m_context)
		return;
	
	m_iAge=0;
	m_bAlive=true;
	if (m_Symbol && !m_iChars)   // make sure it's a valid symbol and do not enter if already done
		m_languagemodel->EnterNodeSymbol(m_context,m_Symbol);
	
	vector<symbol> newchars;   // place to put this list of characters
	vector<unsigned int> cum,groups;   // for the probability list
	m_languagemodel->GetNodeProbs(m_context,newchars,groups,cum,0.003);
	m_iChars=newchars.size();
	// work out cumulative probs
	unsigned int i;
	for (i=1;i<m_iChars;i++) 
		cum[i]+=cum[i-1];
	
	// create an array of pointers to nodes
	m_Children =new CDasherNode *[m_iChars];
	
	// create the children
	ColorSchemes NormalScheme, SpecialScheme;
	if ((m_ColorScheme==Nodes1) || (m_ColorScheme==Special1)) {
		NormalScheme = Nodes2;
		SpecialScheme = Special2;
	} else {
		NormalScheme = Nodes1;
		SpecialScheme = Special1;
	}
	
	ColorSchemes ChildScheme;
	for (unsigned int sym=1;sym<m_iChars;sym++) {
		if (newchars[sym]==this->m_languagemodel->GetSpaceSymbol())
			ChildScheme = SpecialScheme;
		else
			ChildScheme = NormalScheme;
		m_Children[sym]=new CDasherNode(this,newchars[sym],groups[sym],sym,ChildScheme,cum[sym-1],cum[sym],m_languagemodel);
	}
}


void CDasherNode::Push_Node() 
{

	if (m_Children) {
		// if there are children just give them a poke
		unsigned int i;
		for (i=1;i<m_iChars;i++) {
			m_Children[i]->m_iAge=0;
			m_Children[i]->m_bAlive=1;
		}
		return;
	}

	// if we haven't got a context then try to get a new one
	if (m_parent) 
		m_context=m_languagemodel->CloneNodeContext(m_parent->m_context);
	else
		m_context=m_languagemodel->GetRootNodeContext();
	
	// if it fails, be patient
	if (!m_context)
		return;

	m_iAge=0;
	m_bAlive=true;
	if (m_Symbol && !m_iChars)   // make sure it's a valid symbol and don't enter if already done
		m_languagemodel->EnterNodeSymbol(m_context,m_Symbol);

	
	vector<symbol> newchars;   // place to put this list of characters
	vector<unsigned int> cum,groups;   // for the probability list
	m_languagemodel->GetNodeProbs(m_context,newchars,groups,cum,0.003);
	m_iChars=newchars.size();
	// work out cumulative probs
	unsigned int i;
	for (i=1;i<m_iChars;i++)
		cum[i]+=cum[i-1];

	m_Children =new CDasherNode *[m_iChars];

	// create the children
	ColorSchemes NormalScheme, SpecialScheme;
	if ((m_ColorScheme==Nodes1) || (m_ColorScheme==Special1)) {
		NormalScheme = Nodes2;
		SpecialScheme = Special2;
	} else {
		NormalScheme = Nodes1;
		SpecialScheme = Special1;
	}
	
	ColorSchemes ChildScheme;
	for (i=1;i<m_iChars;i++) {
		if (newchars[i]==this->m_languagemodel->GetSpaceSymbol())
			ChildScheme = SpecialScheme;
		else
			ChildScheme = NormalScheme;
		m_Children[i]=new CDasherNode(this,newchars[i],groups[i],i,ChildScheme,cum[i-1],cum[i],m_languagemodel);
	}

}


void CDasherNode::Get_string_under(const int iNormalization,const myint miY1,const myint miY2,const myint miMousex,const myint miMousey, vector<symbol> &vtString) const
{
	// we are over (*this) node so add it to the string 
	vtString.push_back(m_Symbol);
	
	// look for children who might also be under the coords
	if (m_Children) {
		myint miRange=miY2-miY1;
		unsigned int i;
		for (i=1;i<m_iChars;i++) {
			myint miNewy1=miY1+(miRange*m_Children[i]->m_iLbnd)/iNormalization;
			myint miNewy2=miY1+(miRange*m_Children[i]->m_iHbnd)/iNormalization;
			if (miMousey<miNewy2 && miMousey>miNewy1 && miMousex<miNewy2-miNewy1) {
				m_Children[i]->Get_string_under(iNormalization,miNewy1,miNewy2,miMousex,miMousey,vtString);
				return;
			}
		}
	}
	return;
}


CDasherNode * const CDasherNode::Get_node_under(int iNormalization,myint miY1,myint miY2,myint miMousex,myint miMousey) 
{
	if (m_Children) {
		myint miRange=miY2-miY1;
		m_iAge=0;
		m_bAlive=true;
		unsigned int i;
		for (i=1;i<m_iChars;i++) {
			myint miNewy1=miY1+(miRange*m_Children[i]->m_iLbnd)/iNormalization;
			myint miNewy2=miY1+(miRange*m_Children[i]->m_iHbnd)/iNormalization;
		if (miMousey<miNewy2 && miMousey>miNewy1 && miMousex<miNewy2-miNewy1) 
				return m_Children[i]->Get_node_under(iNormalization,miNewy1,miNewy2,miMousex,miMousey);
		}
	}
	return this;
}

