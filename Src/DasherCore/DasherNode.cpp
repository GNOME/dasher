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

void CDasherNode::Generic_Push_Node(CLanguageModel::CNodeContext *context) {

	m_iAge=0;
	m_bAlive=true;

	if (m_Symbol && !m_iChars)   // make sure it's a valid symbol and don't enter if already done
		m_languagemodel->EnterNodeSymbol(m_context,m_Symbol);

      if (m_Symbol==m_languagemodel->GetControlSymbol() || m_bControlChild==true) {
	  int i,quantum;
	  ControlTree *controltree;
	  if (m_controltree==NULL) // Root of the tree
	    controltree = m_languagemodel->GetControlTree();
	  else // some way down
	    controltree = m_controltree->children;
	  
	  m_iChars=1;
	  
	  if (controltree!=NULL) {
	    m_iChars++;
	    while(controltree->next!=NULL) {
	      m_iChars++;
	      controltree=controltree->next;
	    }
	  }

	  // Now we go back and build the node tree	  
	  if (m_controltree==NULL) {
	    controltree=m_languagemodel->GetControlTree();
	  } else {
	    controltree=m_controltree->children; 
	  }

	  i=1;
	  // FIXME - hard coded height
	  quantum=int(1024/m_iChars);

	  m_iChars++;

	  m_Children=new CDasherNode *[m_iChars];

	  ColorSchemes ChildScheme;
	  if (m_ColorScheme==Nodes1) {
	    ChildScheme = Nodes2;
	  } else {
	    ChildScheme = Nodes1;
	  }

	  m_Children[1]=new CDasherNode(this,0,0,0,Opts::Nodes1,0,int(i*quantum),m_languagemodel,false,0);

	  while(controltree!=NULL) {
	    i++;
	    m_Children[i]=new CDasherNode(this,0,0,i,ChildScheme,int((i-1)*quantum),int(i*quantum),m_languagemodel,true,(i%99)+11,controltree);
	    controltree=controltree->next;
	  }
	  return;
	}

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
		m_Children[i]=new CDasherNode(this,newchars[i],groups[i],i,ChildScheme,cum[i-1],cum[i],m_languagemodel,false,m_languagemodel->GetColour(i));
	}
}

/////////////////////////////////////////////////////////////////////////////

void CDasherNode::Push_Node(CLanguageModel::CNodeContext *context) 
// push a node copying the specified context
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
	m_context=m_languagemodel->CloneNodeContext(context);
	// if it fails, be patient
	if (!m_context)
		return;
	Generic_Push_Node(m_context);
}

/////////////////////////////////////////////////////////////////////////////

bool CDasherNode::NodeIsParent(CDasherNode *oldnode) {
  if (oldnode==m_parent) {
    return true;
  } else {
    return false;
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
	Generic_Push_Node(m_context);
}

void CDasherNode::Recursive_Push_Node(int depth) {

  if ((m_iHbnd-m_iLbnd)<10) {
    return;
  }

  Push_Node();

  if (depth>2)
    return;

  for (int i=1; i<m_iChars; i++) {
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
		for (i=1;i<m_iChars;i++) {
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

/////////////////////////////////////////////////////////////////////////////

void CDasherNode::Delete_dead(CDasherNode* alive) 
{
  if (m_Children) {
		unsigned int i; 
		for (i=1;i<m_iChars;i++) {
		        if (m_Children[i]!=0 && m_Children[i]!=alive) {
			      m_Children[i]->Delete_children();
			      delete m_Children[i];
			}
		}
		delete [] m_Children;
	}
	m_Children=0;
}
