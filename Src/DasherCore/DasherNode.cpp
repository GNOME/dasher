// DasherNode.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "DasherNode.h"
#include <cassert>
using namespace Dasher;
using namespace Opts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////

bool CDasherNode::NodeIsParent(CDasherNode *oldnode) {
  if (oldnode==m_pParent) {
    return true;
  } else {
    return false;
  }
}

/////////////////////////////////////////////////////////////////////////////

void CDasherNode::Push_Node() 
{
	if (m_Children) {
		// if there are children just give them a poke
		unsigned int i;
		for (i=1;i<m_iChildCount;i++)
			m_Children[i]->m_bAlive=1;
		return;
	}

	// if we haven't got a context then derive it
	
	if (m_pContext==NULL)
	{
		if (m_Symbol!=0)
		{
			assert (m_pParent !=NULL) ;
			// Normal symbol - derive context from parent
			m_pContext = m_languagemodel->CloneContext(m_pParent->m_pContext);
			m_languagemodel->EnterSymbol(m_pContext,m_Symbol);
		}
		else
		{
			// For new "root" nodes (such as under control mode), we want to 
			// mimic the root context
			m_pContext=m_languagemodel->GetEmptyContext();
			m_languagemodel->EnterText(m_pContext, ". ");

		}

	}

	m_bAlive=true;

	if (m_Symbol==m_languagemodel->GetControlSymbol() || m_bControlChild==true) {
		int i,quantum;
		ControlTree *controltree;
		if (m_controltree==NULL) { // Root of the tree 
			controltree = m_DasherModel.GetControlTree();
		}
		else { // some way down
			controltree = m_controltree->children;
		}

		m_iChildCount=1;

		if (controltree!=NULL) {
			m_iChildCount++;
			while(controltree->next!=NULL) {
				m_iChildCount++;
				controltree=controltree->next;
			}
		}

		// Now we go back and build the node tree	  
		if (m_controltree==NULL) {
			controltree=m_DasherModel.GetControlTree();
		} else {
			controltree=m_controltree->children; 
		}

		i=1;

		quantum=int(m_DasherModel.Normalization()/m_iChildCount);

		m_iChildCount++;

		m_Children=new CDasherNode *[m_iChildCount];

		ColorSchemes ChildScheme;
		if (m_ColorScheme==Nodes1) {
			ChildScheme = Nodes2;
		} else {
			ChildScheme = Nodes1;
		}

		m_Children[1]=new CDasherNode(m_DasherModel,this,0,0,0,Opts::Nodes1,0,int(i*quantum),m_languagemodel,false,240);

		while(controltree!=NULL) {
			i++;
			if (controltree->colour!=-1) {
				m_Children[i]=new CDasherNode(m_DasherModel,this,0,0,i,ChildScheme,int((i-1)*quantum),int(i*quantum),m_languagemodel,true,controltree->colour,controltree);
			} else {
				m_Children[i]=new CDasherNode(m_DasherModel,this,0,0,i,ChildScheme,int((i-1)*quantum),int(i*quantum),m_languagemodel,true,(i%99)+11,controltree);
			}
			controltree=controltree->next;
		}
		return;
	}

	vector<symbol> newchars;   // place to put this list of characters
	vector<unsigned int> cum,groups;   // for the probability list
	m_languagemodel->GetProbs(m_pContext,newchars,groups,cum,m_DasherModel.Normalization());
	m_iChildCount=newchars.size();
	// work out cumulative probs
	unsigned int i;
	for (i=1;i<m_iChildCount;i++)
		cum[i]+=cum[i-1];

	assert(m_Children==0); // otherwise we have another memory leak
	m_Children =new CDasherNode *[m_iChildCount];

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

	for (i=1;i<m_iChildCount;i++) {
		if (newchars[i]==this->m_languagemodel->GetSpaceSymbol())
			ChildScheme = SpecialScheme;
		else
			ChildScheme = NormalScheme;
		m_Children[i]=new CDasherNode(m_DasherModel,this,newchars[i],groups[i],i,ChildScheme,cum[i-1],cum[i],m_languagemodel,false,m_languagemodel->GetColour(i));
	}
}

void CDasherNode::Recursive_Push_Node(int depth) {

  if ((m_iHbnd-m_iLbnd)<0.1*(m_DasherModel.Normalization())) {
    return;
  }

  if (m_Symbol==m_languagemodel->GetControlSymbol()) {
    return;
  }

  Push_Node();

  if (depth>2)
    return;

  for (unsigned int i=1; i<m_iChildCount; i++) {
    if (m_Children!=0 && m_Children[i]!=0) {
      m_Children[i]->Recursive_Push_Node(depth+1);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void CDasherNode::Get_string_under(const int iNormalization,const myint miY1,const myint miY2,const myint miMousex,const myint miMousey, vector<symbol> &vString) const
{
	// we are over (*this) node so add it to the string 
	vString.push_back(m_Symbol);
	
	// look for children who might also be under the coords
	if (m_Children) {
		myint miRange=miY2-miY1;
		unsigned int i;
		for (i=1;i<m_iChildCount;i++) {
			myint miNewy1=miY1+(miRange*m_Children[i]->m_iLbnd)/iNormalization;
			myint miNewy2=miY1+(miRange*m_Children[i]->m_iHbnd)/iNormalization;
			if (miMousey<miNewy2 && miMousey>miNewy1 && miMousex<miNewy2-miNewy1) {
				m_Children[i]->Get_string_under(iNormalization,miNewy1,miNewy2,miMousex,miMousey,vString);
				return;
			}
		}
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////

CDasherNode * const CDasherNode::Get_node_under(int iNormalization,myint miY1,myint miY2,myint miMousex,myint miMousey) 
{
	if (m_Children) {
		myint miRange=miY2-miY1;
//		m_iAge=0;
		m_bAlive=true;
		unsigned int i;
		for (i=1;i<m_iChildCount;i++) {
			myint miNewy1=miY1+(miRange*m_Children[i]->m_iLbnd)/iNormalization;
			myint miNewy2=miY1+(miRange*m_Children[i]->m_iHbnd)/iNormalization;
		if (miMousey<miNewy2 && miMousey>miNewy1 && miMousex<miNewy2-miNewy1) 
				return m_Children[i]->Get_node_under(iNormalization,miNewy1,miNewy2,miMousex,miMousey);
		}
	}
	return this;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherNode::Delete_dead(CDasherNode* alive) 
{
  if (m_Children) {
		unsigned int i; 
		for (i=1;i<m_iChildCount;i++) {
		        if (m_Children[i]!=0 && m_Children[i]!=alive) {
			      m_Children[i]->Delete_children();
			      delete m_Children[i];
			}
		}
		delete [] m_Children;
	}
  m_Children=0;
}
