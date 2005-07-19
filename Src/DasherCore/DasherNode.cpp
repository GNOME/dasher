// DasherNode.cpp
//
// Copyright (c) 2001-2004 David Ward

#include "../Common/Common.h"

#include "DasherNode.h"
using namespace Dasher;
using namespace Opts;
using namespace std;

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

bool CDasherNode::NodeIsParent(CDasherNode *oldnode) const {
  if(oldnode == m_pParent)
    return true;
  else
    return false;

}

void CDasherNode::Get_string_under(const int iNormalization, const myint miY1, const myint miY2, const myint miMousex, const myint miMousey, vector <symbol >&vString) const {
  // we are over (*this) node so add it to the string 
  vString.push_back(m_Symbol);

  // look for children who might also be under the coords
  // FIXME what if not all children are instantiated?
  myint miRange = miY2 - miY1;
  ChildMap::const_iterator i;
  for(i = GetChildren().begin(); i != GetChildren().end(); i++) {
    myint miNewy1 = miY1 + (miRange * i->second->m_iLbnd) / iNormalization;
    myint miNewy2 = miY1 + (miRange * i->second->m_iHbnd) / iNormalization;
    if(miMousey < miNewy2 && miMousey > miNewy1 && miMousex < miNewy2 - miNewy1) {
      i->second->Get_string_under(iNormalization, miNewy1, miNewy2, miMousex, miMousey, vString);
      return;
    }
  }
  return;
}

CDasherNode *const CDasherNode::Get_node_under(int iNormalization, myint miY1, myint miY2, myint miMousex, myint miMousey) {
  myint miRange = miY2 - miY1;
//              m_iAge=0;
  m_bAlive = true;
  ChildMap::const_iterator i;
  for(i = GetChildren().begin(); i != GetChildren().end(); i++) {
    CDasherNode *pChild = i->second;

    myint miNewy1 = miY1 + (miRange * pChild->m_iLbnd) / iNormalization;
    myint miNewy2 = miY1 + (miRange * pChild->m_iHbnd) / iNormalization;
    if(miMousey < miNewy2 && miMousey > miNewy1 && miMousex < miNewy2 - miNewy1)
      return pChild->Get_node_under(iNormalization, miNewy1, miNewy2, miMousex, miMousey);
  }
  return this;
}

// kill ourselves and all other children except for the specified
// child
// FIXME this probably shouldn't be called after history stuff is working
void CDasherNode::OrphanChild(CDasherNode *pChild) {
  DASHER_ASSERT(ChildCount() > 0);

  ChildMap::const_iterator i;
  for(i = GetChildren().begin(); i != GetChildren().end(); i++) {
    if(i->second != pChild) {
      i->second->Delete_children();
      delete i->second;
    }

  }
  Children().clear();
  SetHasAllChildren(false);
}

// Delete nephews of the child which has the specified symbol
void CDasherNode::DeleteNephews(int iSym) {
  DASHER_ASSERT(Children().size() > 0);

  ChildMap::iterator i;
  for(i = Children().begin(); i != Children().end(); i++) {
    if(i->first != iSym) {
      i->second->Delete_children();
    }

  }
}

void CDasherNode::Delete_children() {
  ChildMap::iterator i;
  for(i = Children().begin(); i != Children().end(); i++) {
    // i->second->Delete_children(); (gets called by destructor)
    delete i->second;
  }
  Children().clear();
  SetHasAllChildren(false);
}

