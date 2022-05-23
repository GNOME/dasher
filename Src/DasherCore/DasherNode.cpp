// DasherNode.cpp
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

#include "../Common/Common.h"

// #include "AlphabetManager.h" - doesnt seem to be required - pconlon

#include "DasherInterfaceBase.h"

using namespace Dasher;
using namespace Opts;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif
static int iNumNodes = 0;

int Dasher::currentNumNodeObjects() {return iNumNodes;}

//TODO this used to be inline - should we make it so again?
CDasherNode::CDasherNode(int iOffset, int iColour, CDasherScreen::Label *pLabel)
: onlyChildRendered(NULL),  m_iLbnd(0), m_iHbnd(CDasherModel::NORMALIZATION), m_pParent(NULL), m_iFlags(DEFAULT_FLAGS), m_iOffset(iOffset), m_iColour(iColour), m_pLabel(pLabel) {
  iNumNodes++;
}

// TODO: put this back to being inlined
CDasherNode::~CDasherNode() {
  //  std::cout << "Deleting node: " << this << std::endl;
  // Release any storage that the node manager has allocated,
  // unreference ref counted stuff etc.
  Delete_children();

  //  std::cout << "done." << std::endl;

  iNumNodes--;
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
	if ( Children() ) 
	{
		myint miRange=miY2-miY1;
//		m_iAge=0;
		m_bAlive=true;
		unsigned int i;
		for (i=0;i<m_iChildCount;i++) 
		{
			CDasherNode* pChild = Children()[i];

			myint miNewy1=miY1+(miRange*pChild->m_iLbnd)/iNormalization;
			myint miNewy2=miY1+(miRange*pChild->m_iHbnd)/iNormalization;
			if (miMousey<miNewy2 && miMousey>miNewy1 && miMousex<miNewy2-miNewy1) 
				return pChild->Get_node_under(iNormalization,miNewy1,miNewy2,miMousex,miMousey);
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

  pChild->m_pParent=NULL;

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

// Delete nephews of the child which has the specified symbol
// TODO: Need to allow for subnode
void CDasherNode::DeleteNephews(CDasherNode *pChild) {
  DASHER_ASSERT(Children().size() > 0);

  ChildMap::iterator i;
  for(i = Children().begin(); i != Children().end(); i++) {
      if(*i != pChild) {
	(*i)->Delete_children();
    }
  }
}

// TODO: Need to allow for subnodes
// TODO: Incorporate into above routine
void CDasherNode::Delete_children() {
//  std::cout << "Start: " << this << std::endl;

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

int CDasherNode::MostProbableChild() {
  int iMax(0);
  int iCurrent;

  for(ChildMap::iterator it(m_mChildren.begin()); it != m_mChildren.end(); ++it) {
    iCurrent = (*it)->Range();

    if(iCurrent > iMax)
      iMax = iCurrent;
  }

  return iMax;
}

bool CDasherNode::GameSearchChildren(symbol sym) {
  for (ChildMap::iterator i = Children().begin(); i != Children().end(); i++) {
    if ((*i)->GameSearchNode(sym)) return true;
  }
  return false;
}
