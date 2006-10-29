// DasherNode.cpp
//
// Copyright (c) 2001-2004 David Ward

#include "../Common/Common.h"

#include "DasherNode.h"

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

// TODO: put this back to being inlined
CDasherNode::~CDasherNode() {

  // Release any storage that the node manager has allocated,
  // unreference ref counted stuff etc.

  if(m_bWatchDelete)
    std::cout << "Deleting " << this << std::endl;

  m_pNodeManager->ClearNode( this );

  Delete_children();
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

bool CDasherNode::NodeIsParent(CDasherNode *oldnode) const {
  if(oldnode == m_pParent)
    return true;
  else
    return false;

}

CDasherNode *const CDasherNode::Get_node_under(int iNormalization, myint miY1, myint miY2, myint miMousex, myint miMousey) {
  myint miRange = miY2 - miY1;
//              m_iAge=0;
  m_bAlive = true;
  ChildMap::const_iterator i;
  for(i = GetChildren().begin(); i != GetChildren().end(); i++) {
    CDasherNode *pChild = *i;

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
    if((*i) != pChild) {
      (*i)->Delete_children();
      delete (*i);
    }
  }
  
  pChild->SetParent(NULL);

  Children().clear();
  SetHasAllChildren(false);
}

// Delete nephews of the child which has the specified symbol
void CDasherNode::DeleteNephews(CDasherNode *pChild) {
  DASHER_ASSERT(Children().size() > 0);

  ChildMap::iterator i;
  for(i = Children().begin(); i != Children().end(); i++) {
    if(*i != pChild) {
      (*i)->Delete_children();
    }

  }
}

void CDasherNode::Delete_children() {
  ChildMap::iterator i;
  for(i = Children().begin(); i != Children().end(); i++) {
    // i->second->Delete_children(); (gets called by destructor)
    delete (*i);
  }
  Children().clear();
  SetHasAllChildren(false);
}

// Gets the probability of this node, conditioned on the parent
double CDasherNode::GetProb(int iNormalization) {    
  return (double) (m_iHbnd - m_iLbnd) / (double) iNormalization;
}

void CDasherNode::ConvertWithAncestors() {
  if(m_bConverted)
    return;
  
  m_bConverted = true;

  if(m_pParent)
    m_pParent->ConvertWithAncestors();
}
