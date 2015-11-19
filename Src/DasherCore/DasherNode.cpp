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

void CDasherNode::Trace() const {
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

void CDasherNode::GetContext(CDasherInterfaceBase *pInterface, const CAlphabetMap *pAlphabet, vector<symbol> &vContextSymbols, int iOffset, int iLength) {
  if (!GetFlag(NF_SEEN)) {
    DASHER_ASSERT(m_pParent);
    if (m_pParent) m_pParent->GetContext(pInterface, pAlphabet, vContextSymbols, iOffset,iLength);
  } else {
    std::string strContext = pInterface->GetContext(iOffset, iLength);
    pAlphabet->GetSymbols(vContextSymbols, strContext);
  }
}

// kill ourselves and all other children except for the specified
// child
// FIXME this probably shouldn't be called after history stuff is working
void CDasherNode::OrphanChild(CDasherNode *pChild) {
  DASHER_ASSERT(ChildCount() > 0);

  ChildMap::const_iterator i;
  for(i = GetChildren().begin(); i != GetChildren().end(); i++) {
    if((*i) != pChild) {
      (*i)->Delete_children();
      delete (*i);
    }
  }

  pChild->m_pParent=NULL;

  Children().clear();
  SetFlag(NF_ALLCHILDREN, false);
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

  ChildMap::iterator i;
  for(i = Children().begin(); i != Children().end(); i++) {
    //    std::cout << "CNM: " << (*i)->MgrID() << (*i) << " " << (*i)->Parent() << std::endl;
    delete (*i);
  }
  Children().clear();
  //  std::cout << "NM: " << MgrID() << std::endl;
  SetFlag(NF_ALLCHILDREN, false);
  onlyChildRendered = NULL;
}

void CDasherNode::SetFlag(int iFlag, bool bValue) {

 if(bValue)
    m_iFlags = m_iFlags | iFlag;
  else
    m_iFlags = m_iFlags & (~iFlag);
}

void CDasherNode::Reparent(CDasherNode *pNewParent, unsigned int iLbnd, unsigned int iHbnd) {
  DASHER_ASSERT(!m_pParent);
  DASHER_ASSERT(pNewParent);
  DASHER_ASSERT(!pNewParent->GetFlag(NF_ALLCHILDREN));
  DASHER_ASSERT(iLbnd == (pNewParent->GetChildren().empty() ? 0 : pNewParent->GetChildren().back()->m_iHbnd));
  m_pParent = pNewParent;
  pNewParent->Children().push_back(this);
  m_iLbnd = iLbnd;
  m_iHbnd = iHbnd;
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
