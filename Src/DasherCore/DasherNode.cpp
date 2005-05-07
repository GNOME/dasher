// DasherNode.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherNode.h"
#include "../Common/assert.h"
using namespace Dasher;
using namespace Opts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////

void CDasherNode::Trace () const
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

bool CDasherNode::NodeIsParent(CDasherNode *oldnode) const
{
  if (oldnode==m_pParent)
	  return true;
  else
	  return false;

}

/////////////////////////////////////////////////////////////////////////////

void CDasherNode::Get_string_under(const int iNormalization,const myint miY1,const myint miY2,const myint miMousex,const myint miMousey, vector<symbol> &vString) const
{
	// we are over (*this) node so add it to the string 
	vString.push_back(m_Symbol);
	
	// look for children who might also be under the coords
	if (m_ppChildren)
	{
		myint miRange=miY2-miY1;
		unsigned int i;
		for (i=1;i<m_iChildCount;i++) 
		{
			myint miNewy1=miY1+(miRange* Children()[i]->m_iLbnd)/iNormalization;
			myint miNewy2=miY1+(miRange* Children()[i]->m_iHbnd)/iNormalization;
			if (miMousey<miNewy2 && miMousey>miNewy1 && miMousex<miNewy2-miNewy1) 
			{
				Children()[i]->Get_string_under(iNormalization,miNewy1,miNewy2,miMousex,miMousey,vString);
				return;
			}
		}
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////

CDasherNode * const CDasherNode::Get_node_under(int iNormalization,myint miY1,myint miY2,myint miMousex,myint miMousey) 
{
	if ( Children() ) {
		myint miRange=miY2-miY1;
//		m_iAge=0;
		m_bAlive=true;
		unsigned int i;
		for (i=0;i<m_iChildCount;i++) 
		{
			myint miNewy1=miY1+(miRange*Children()[i]->m_iLbnd)/iNormalization;
			myint miNewy2=miY1+(miRange*Children()[i]->m_iHbnd)/iNormalization;
			if (miMousey<miNewy2 && miMousey>miNewy1 && miMousex<miNewy2-miNewy1) 
				return Children()[i]->Get_node_under(iNormalization,miNewy1,miNewy2,miMousex,miMousey);
		}
	}
	return this;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherNode::OrphanChild(CDasherNode* pChild)
{
	DASHER_ASSERT ( Children() ) ;

	int i; 
	for (i=0;i< ChildCount(); i++) 
	{
		if ( Children()[i] != pChild )
		{
			Children()[i]->Delete_children();
			delete Children()[i];
		}

	}
	delete [] m_ppChildren;
	m_ppChildren=0;
	m_iChildCount=0;
}

/////////////////////////////////////////////////////////////////////////////

// Delete nephews
void CDasherNode::DeleteNephews(int iChild)
{
	DASHER_ASSERT ( Children() ) ;

	int i; 
	for (i=0;i< ChildCount(); i++) 
	{
		if (i != iChild)
		{
			Children()[i]->Delete_children();
		}

	}
}


/////////////////////////////////////////////////////////////////////////////

void CDasherNode::Delete_children() 
{
	if (m_ppChildren) 
	{
		for (int i=0;i<m_iChildCount;i++) 
		{
		     m_ppChildren[i]->Delete_children();
		     delete m_ppChildren[i];
		}
		delete [] m_ppChildren;
		m_ppChildren=0;	
		m_iChildCount=0;
	}
}

/////////////////////////////////////////////////////////////////////////////
