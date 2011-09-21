// DasherModel.cpp
//
// Copyright (c) 2008 The Dasher Team
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

#include <sstream>

#include <iostream>
#include <cstring>
#include "../Common/Random.h"
#include "DasherModel.h"
#include "DasherView.h"
#include "Parameters.h"

#include "Event.h"
#include "NodeCreationManager.h"
#include "AlphabetManager.h"

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// FIXME - need to get node deletion working properly and implement reference counting

// CDasherModel

CDasherModel::CDasherModel(CSettingsUser *pCreateFrom)
: CSettingsUser(pCreateFrom) {
  
  m_pLastOutput = m_Root = NULL;

  m_Rootmin = 0;
  m_Rootmax = 0;
  m_iDisplayOffset = 0;
  m_dTotalNats = 0.0;

  // TODO: Need to rationalise the require conversion methods
#ifdef JAPANESE
  m_bRequireConversion = true;
#else
  m_bRequireConversion = false;
#endif

  m_dAddProb = 0.003;

  m_Rootmin_min = int64_min / NORMALIZATION / 2;
  m_Rootmax_max = int64_max / NORMALIZATION / 2;

}

CDasherModel::~CDasherModel() {
  if (m_pLastOutput) m_pLastOutput->Leave();

  if(oldroots.size() > 0) {
    delete oldroots[0];
    oldroots.clear();
    // At this point we have also deleted the root - so better NULL pointer
    m_Root = NULL;
  } else {
    delete m_Root;
    m_Root = NULL;
  }
}

void CDasherModel::Make_root(CDasherNode *pNewRoot) {
  //  std::cout << "Make root" << std::endl;

  DASHER_ASSERT(pNewRoot != NULL);
  DASHER_ASSERT(pNewRoot->Parent() == m_Root);

  m_Root->DeleteNephews(pNewRoot);
  m_Root->SetFlag(NF_COMMITTED, true);

  // TODO: Is the stack necessary at all? We may as well just keep the
  // existing data structure?
  oldroots.push_back(m_Root);

  // TODO: tidy up conditional
  while((oldroots.size() > 10) && (!m_bRequireConversion || (oldroots[0]->GetFlag(NF_CONVERTED)))) {
    oldroots[0]->OrphanChild(oldroots[1]);
    delete oldroots[0];
    oldroots.pop_front();
  }
  DASHER_ASSERT(pNewRoot->GetFlag(NF_SEEN));
  m_Root = pNewRoot;

  // Update the root coordinates, as well as any currently scheduled locations
  const myint range = m_Rootmax - m_Rootmin;
  m_Rootmax = m_Rootmin + (range * m_Root->Hbnd()) / NORMALIZATION;
  m_Rootmin = m_Rootmin + (range * m_Root->Lbnd()) / NORMALIZATION;

  for(std::deque<SGotoItem>::iterator it(m_deGotoQueue.begin()); it != m_deGotoQueue.end(); ++it) {
    //Some of these co-ordinate pairs can be bigger than m_Rootmin_min - m_Rootmax_max,
    // hence using unsigned type...
    const uint64 r = it->iN2 - it->iN1;
    it->iN2 = it->iN1 + (r * m_Root->Hbnd()) / NORMALIZATION;
    it->iN1 = it->iN1 + (r * m_Root->Lbnd()) / NORMALIZATION;
  }
}

bool CDasherModel::Reparent_root() {
  DASHER_ASSERT(m_Root != NULL);

  // Change the root node to the parent of the existing node. We need
  // to recalculate the coordinates for the "new" root as the user may
  // have moved around within the current root
  CDasherNode *pNewRoot;

  if(oldroots.size() == 0) {
    pNewRoot = m_Root->RebuildParent();
    // Fail if there's no existing parent and no way of recreating one
    if(pNewRoot == NULL) return false;
    //better propagate gameness backwards, the original nodes must have been NF_GAME too
    if (m_Root->GetFlag(NF_GAME))
      for (CDasherNode *pTemp=pNewRoot; pTemp; pTemp=pTemp->Parent())
        pTemp->SetFlag(NF_GAME, true);
    //RebuildParent() can create multiple generations of parents at once;
    // make sure our cache has all such that were created, so we delete them
    // if we ever delete all our other nodes.
    for (CDasherNode *pTemp = pNewRoot; (pTemp = pTemp->Parent()); )
      oldroots.push_front(pTemp);
  }
  else {
    pNewRoot = oldroots.back();
    oldroots.pop_back();
  }

  DASHER_ASSERT(m_Root->Parent() == pNewRoot);

  const myint lower(m_Root->Lbnd()), upper(m_Root->Hbnd());
  const myint iRange(upper-lower);
  myint iRootWidth(m_Rootmax - m_Rootmin);

  // Fail if the new root is bigger than allowed by normalisation
  if(((myint(NORMALIZATION - upper) / static_cast<double>(iRange)) >
           (m_Rootmax_max - m_Rootmax)/static_cast<double>(iRootWidth)) ||
      ((myint(lower) / static_cast<double>(iRange)) >
           (m_Rootmin - m_Rootmin_min) / static_cast<double>(iRootWidth))) {
    //but cache the (currently-unusable) root node - else we'll keep recreating (and deleting) it on every frame...
    oldroots.push_back(pNewRoot);
    return false;
  }

  //don't uncommit until they reverse out of the node
  // (or committing would enter the node into the LM a second time)

  //Update the root coordinates to reflect the new root
  DASHER_ASSERT(pNewRoot->GetFlag(NF_SEEN));
  m_Root = pNewRoot;

  m_Rootmax = m_Rootmax + ((NORMALIZATION - upper) * iRootWidth) / iRange;
  m_Rootmin = m_Rootmin - (lower * iRootWidth) / iRange;

  for(std::deque<SGotoItem>::iterator it(m_deGotoQueue.begin()); it != m_deGotoQueue.end(); ++it) {
    iRootWidth = it->iN2 - it->iN1;
    it->iN2 = it->iN2 + (myint(NORMALIZATION - upper) * iRootWidth / iRange);
    it->iN1 = it->iN1 - (myint(lower) * iRootWidth / iRange);
  }
  return true;
}

void CDasherModel::ClearRootQueue() {
  while(oldroots.size() > 0) {
    if(oldroots.size() > 1) {
      oldroots[0]->OrphanChild(oldroots[1]);
    }
    else {
      oldroots[0]->OrphanChild(m_Root);
    }
    delete oldroots[0];
    oldroots.pop_front();
  }
}

void CDasherModel::SetOffset(int iOffset, CAlphabetManager *pMgr, CDasherView *pView, bool bForce) {
  //if we don't have a root, always "re"build the tree!
  // (if we have a root, only rebuild to move location or if bForce says to)
  if (m_Root && iOffset == GetOffset() && !bForce) return;

  if (m_pLastOutput) m_pLastOutput->Leave();

  ClearRootQueue();
  delete m_Root;

  m_Root = pMgr->GetRoot(NULL, 0, NORMALIZATION, iOffset!=0, iOffset);
  if (iOffset) {
    //there were preceding characters. It's nonetheless possible that they weren't
    // part of the current alphabet, and so we may have got a simple group node as root,
    // rather than a character node (responsible for the last said preceding character),
    // but even so, it seems fair enough to say we've "seen" the root:
    m_Root->SetFlag(NF_SEEN, true);
    m_Root->Enter();
    // (of course, we don't do Output() - the context contains it already!)
    m_pLastOutput = m_Root;

    //We also want to avoid training the LM on nodes representing already-written context
    m_Root->SetFlag(NF_COMMITTED, true);

  } else
    m_pLastOutput = NULL;
  if (GetBoolParameter(BP_GAME_MODE)) m_Root->SetFlag(NF_GAME, true);

  // Create children of the root...
  ExpandNode(m_Root);

  // Set the root coordinates so that the root node is an appropriate
  // size and we're not in any of the children

  double dFraction( 1 - (1 - m_Root->MostProbableChild() / static_cast<double>(NORMALIZATION)) / 2.0 );

  //TODO somewhere round here, old code checked whether the InputFilter implemented
  // GetMinWidth, if so called LimitRoot w/that width - i.e., make sure iWidth
  // is no more than that minimum. Should we do something similar here???

  int iWidth( static_cast<int>( MAX_Y / (2.0*dFraction) ) );

  m_Rootmin = MAX_Y / 2 - iWidth / 2;
  m_Rootmax = MAX_Y / 2 + iWidth / 2;

  m_iDisplayOffset = 0;
}

int CDasherModel::GetOffset() {
  return m_pLastOutput ? m_pLastOutput->offset()+1 : m_Root ? m_Root->offset()+1 : 0;
};

bool CDasherModel::NextScheduledStep()
{
  if (m_deGotoQueue.size() == 0) return false;
  myint newRootmin(m_deGotoQueue.front().iN1), newRootmax(m_deGotoQueue.front().iN2);
  m_deGotoQueue.pop_front();

  m_dTotalNats += log((newRootmax - newRootmin) / static_cast<double>(m_Rootmax - m_Rootmin));

  m_iDisplayOffset = (m_iDisplayOffset * 90) / 100;

  // Now actually zoom to the new location

  while (newRootmax >= m_Rootmax_max || newRootmin <= m_Rootmin_min) {
    // can't make existing root any bigger because of overflow. So force a new root
    // to be chosen (so that Dasher doesn't just stop!)...

    //pick _child_ covering crosshair...
    const myint iWidth(m_Rootmax-m_Rootmin);
    for (CDasherNode::ChildMap::const_iterator it = m_Root->GetChildren().begin(), E = m_Root->GetChildren().end(); ;) {
      CDasherNode *pChild(*it);
      DASHER_ASSERT(m_Rootmin + ((pChild->Lbnd() * iWidth) / NORMALIZATION) <= ORIGIN_Y);
      if (m_Rootmin + ((pChild->Hbnd() * iWidth) / NORMALIZATION) > ORIGIN_Y) {
        //found child to make root. proceed only if new root is on the game path....
        if (GetBoolParameter(BP_GAME_MODE) && !pChild->GetFlag(NF_GAME)) {
          //If the user's strayed that far off the game path,
          // having Dasher stop seems reasonable!
          return false;
        }

        //make pChild the root node...
        //first we're gonna have to force it to be output, as a non-output root won't work...
        if (!pChild->GetFlag(NF_SEEN)) {
          DASHER_ASSERT(m_pLastOutput == m_Root);
          OutputTo(pChild);
        }
        //we need to update the target coords (newRootmin,newRootmax)
        // to reflect the new coordinate system based upon pChild as root.
        //Make_root automatically updates any such pairs stored in m_deGotoQueue, so:
        SGotoItem temp; temp.iN1 = newRootmin; temp.iN2 = newRootmax;
        m_deGotoQueue.push_back(temp);
        //...when we make pChild the root...
        Make_root(pChild);
        //...we can retrieve new, equivalent, coordinates for it
        newRootmin = m_deGotoQueue.back().iN1; newRootmax = m_deGotoQueue.back().iN2;
        m_deGotoQueue.pop_back();
        // (note that the next check below will make sure these coords do cover (0, ORIGIN_Y))
        break;
      }
      ++it;
      DASHER_ASSERT (it != E); //must find a child!
    }
  }

  // Check that we haven't drifted too far. The rule is that we're not
  // allowed to let the root max and min cross the midpoint of the
  // screen.
  newRootmin = min(newRootmin, ORIGIN_Y - 1 - m_iDisplayOffset);
  newRootmax = max(newRootmax, ORIGIN_Y + 1 - m_iDisplayOffset);

  // Only allow the update if it won't make the
  // root too small. We should have re-generated a deeper root
  // before now already, but the original root is an exception.
  // (as is trying to go back beyond the earliest char in the current
  // alphabet, if there are preceding characters not in that alphabet)
  if ((newRootmax - newRootmin) > MAX_Y / 4) {
    m_Rootmax = newRootmax;
    m_Rootmin = newRootmin;
    return true;
  } //else, we just stop - this prevents the user from zooming too far back
    //outside the root node (when we can't generate an older root).  return true;
  return false;
}

void CDasherModel::ScheduleOneStep(myint X, myint Y, int iSteps, dasherint iMinSize) {
  myint r1, r2;
  // works out next viewpoint

  DASHER_ASSERT(m_Root != NULL);
  // Avoid X == 0, as this corresponds to infinite zoom
  if (X <= 0) X = 1;
  
  // If X is too large we risk overflow errors, so limit it
  dasherint iMaxX = (1 << 29) / iSteps;
  if (X > iMaxX) X = iMaxX;
  
  // Mouse coords X, Y
  // const dasherint Y1 = 0;
  const dasherint Y2(MAX_Y);
  
  // Calculate what the extremes of the viewport will be when the
  // point under the cursor is at the cross-hair. This is where
  // we want to be in iSteps updates
  
  dasherint y1(Y - (Y2 * X) / (2 * ORIGIN_X));
  dasherint y2(Y + (Y2 * X) / (2 * ORIGIN_Y));
  dasherint oy1(y1),oy2(y2); //back these up to use later
  
  // iSteps is the number of update steps we need to get the point
  // under the cursor over to the cross hair. Calculated in order to
  // keep a constant bit-rate.
  DASHER_ASSERT(iSteps > 0);
  
  // Calculate the new values of y1 and y2 required to perform a single update
  // step.
  {
    const dasherint denom = Y2 + (iSteps - 1) * (y2 - y1),
    newy1 = y1 * Y2 / denom,
    newy2 = ((y2 * iSteps - y1 * (iSteps - 1)) * Y2) / denom;
    
    y1 = newy1;
    y2 = newy2;
  }
  
  // Calculate the minimum size of the viewport corresponding to the
  // maximum zoom.
  
  if((y2 - y1) < iMinSize) {
    const dasherint newy1 = y1 * (Y2 - iMinSize) / (Y2 - (y2 - y1)),
    newy2 = newy1 + iMinSize;
    
    y1 = newy1;
    y2 = newy2;
  }
  
  //okay, we now have target bounds for the viewport, after allowing for framerate etc.
  // we now go there in one step...
  
  // new root{min,max} r1,r2, old root{min,max} R1,R2
  const dasherint R1 = m_Rootmin;
  const dasherint R2 = m_Rootmax;  
  
  // If |(0,Y2)| = |(y1,y2)|, the "zoom factor" is 1, so we just translate.
  if (Y2 == y2 - y1) {
    r1 = R1 - y1;
    r2 = R2 - y1;
  } else {
    // There is a point C on the y-axis such the ratios (y1-C):(Y1-C) and
    // (y2-C):(Y2-C) are equal - iow that divides the "target" region y1-y2
    // into the same proportions as it divides the screen (0-Y2). I.e., this
    // is the center of expansion - the point on the y-axis which everything
    // moves away from (or towards, if reversing).
    
    //We prefer to compute C from the _original_ (y1,y2) pair, as this is more
    // accurate (and avoids drifting up/down when heading straight along the
    // x-axis in dynamic button modes). However...
    if (((y2-y1) < Y2) ^ ((oy2-oy1) < Y2)) {
      //Sometimes (very occasionally), the calculation of a single-step above
      // can turn a zoom-in into a zoom-out, or vice versa, when the movement
      // is mostly translation. In which case, must compute C consistently with
      // the (scaled, single-step) movement we are going to perform, or else we
      // will end up suddenly going the wrong way along the y-axis (i.e., the
      // sense of translation will be reversed) !
      oy1=y1; oy2=y2;
    }
    const dasherint C = (oy1 * Y2) / (oy1 + Y2 - oy2);
    
    r1 = ((R1 - C) * Y2) / (y2 - y1) + C;
    r2 = ((R2 - C) * Y2) / (y2 - y1) + C;
  }
  m_deGotoQueue.clear();
  SGotoItem item;
  item.iN1 = r1;
  item.iN2 = r2;
  m_deGotoQueue.push_back(item);
}

void CDasherModel::OutputTo(CDasherNode *pNewNode) {
  //first, recurse back up to last seen node (must be processed ancestor-first)
  if (pNewNode && !pNewNode->GetFlag(NF_SEEN)) {
    OutputTo(pNewNode->Parent());
    if (pNewNode->Parent()) pNewNode->Parent()->Leave();
    pNewNode->Enter();

    m_pLastOutput = pNewNode;
    pNewNode->Output();
    pNewNode->SetFlag(NF_SEEN, true); //becomes NF_SEEN after output.

  } else {
    //either pNewNode is null, or else it's been seen. So delete back to that...
    while (m_pLastOutput != pNewNode) {
      // if pNewNode is null, m_pLastOutput is not; else, pNewNode has been seen,
      // so we should encounter it on the way back out to the root, _before_ null
      m_pLastOutput->SetFlag(NF_COMMITTED, false);
      m_pLastOutput->Undo();
      m_pLastOutput->Leave(); //Should we? I think so, but the old code didn't...?
      m_pLastOutput->SetFlag(NF_SEEN, false);

      m_pLastOutput = m_pLastOutput->Parent();
      if (m_pLastOutput) m_pLastOutput->Enter();
      else DASHER_ASSERT (!pNewNode); //both null
    }
  }
}

void CDasherModel::ExpandNode(CDasherNode *pNode) {
  DASHER_ASSERT(pNode != NULL);

  // TODO: Is NF_ALLCHILDREN any more useful/efficient than reading the map size?

  if(pNode->GetFlag(NF_ALLCHILDREN)) {
    DASHER_ASSERT(pNode->GetChildren().size() > 0);
    return;
  }

  // TODO: Do we really need to delete all of the children at this point?
  pNode->Delete_children(); // trial commented out - pconlon

#ifdef DEBUG
  unsigned int iExpect = pNode->ExpectedNumChildren();
#endif
  pNode->PopulateChildren();
#ifdef DEBUG
  if (iExpect != pNode->GetChildren().size()) {
    std::cout << "(Note: expected " << iExpect << " children, actually created " << pNode->GetChildren().size() << ")" << std::endl;
  }
#endif

  pNode->SetFlag(NF_ALLCHILDREN, true);

  DispatchEvent(pNode);
}

void CDasherModel::RenderToView(CDasherView *pView, CExpansionPolicy &policy) {

  DASHER_ASSERT(pView != NULL);
  DASHER_ASSERT(m_Root != NULL);

  while(pView->IsSpaceAroundNode(m_Rootmin,m_Rootmax)) {
    if (!Reparent_root()) break;
  }

  // The Render routine will fill iGameTargetY with the Dasher Coordinate of the
  // youngest node with NF_GAME set. The model is responsible for setting NF_GAME on
  // the appropriate Nodes.
  CDasherNode *pOutput = pView->Render(m_Root, m_Rootmin + m_iDisplayOffset, m_Rootmax + m_iDisplayOffset, policy);


  OutputTo(pOutput);

  while (CDasherNode *pNewRoot = m_Root->onlyChildRendered) {
#ifdef DEBUG
    //if only one child was rendered, no other child covers the screen -
    // as no other child was onscreen at all!
    for (CDasherNode::ChildMap::const_iterator it = m_Root->GetChildren().begin(); it != m_Root->GetChildren().end(); it++) {
      DASHER_ASSERT(*it == pNewRoot || !(*it)->GetFlag(NF_SUPER));
    }
#endif
    if (pNewRoot->GetFlag(NF_SUPER) &&
        ////GAME MODE TEMP - only change the root if it is on the game path/////////
        (!GetBoolParameter(BP_GAME_MODE) || m_Root->onlyChildRendered->GetFlag(NF_GAME))) {
      Make_root(pNewRoot);
    } else
      break;
  }

}

void CDasherModel::ScheduleZoom(dasherint y1, dasherint y2) {
  DASHER_ASSERT(y2>y1);

  // Rename for readability.
  const dasherint Y1 = 0;
  const dasherint Y2 = MAX_Y;
  const dasherint R1 = m_Rootmin;
  const dasherint R2 = m_Rootmax;

  dasherint C, r1, r2;

  // If |(y1,y2)| = |(Y1,Y2)|, the "zoom factor" is 1, so we just translate.
  // y2 - y1 == Y2 - Y1 => y1 - Y1 == y2 - Y2
  C = y1 - Y1;
  if (C == y2 - Y2) {
      r1 = R1 + C;
      r2 = R2 + C;
  } else {
  // There is a point C on the y-axis such the ratios (y1-C):(Y1-C) and
  // (y2-C):(Y2-C) are equal. (Obvious when drawn on separate parallel axes.)
      C = (y1 * Y2 - y2 * Y1) / (y1 + Y2 - y2 - Y1);

  // So another point r's zoomed y coordinate R, has the same ratio (r-C):(R-C)
      if (y1 != C) {
          r1 = ((R1 - C) * (Y1 - C)) / (y1 - C) + C;
          r2 = ((R2 - C) * (Y1 - C)) / (y1 - C) + C;
      } else if (y2 != C) {
          r1 = ((R1 - C) * (Y2 - C)) / (y2 - C) + C;
          r2 = ((R2 - C) * (Y2 - C)) / (y2 - C) + C;
      } else { // implies y1 = y2
          std::cerr << "Impossible geometry in CDasherModel::ScheduleZoom\n";
      }
  }

  // sNewItem seems to contain a list of root{min,max} for the frames of the
  // zoom, so split r -> R into n steps, with accurate R
  const int nsteps = GetLongParameter(LP_ZOOMSTEPS);
  m_deGotoQueue.clear();
  for (int s = nsteps - 1; s >= 0; --s) {
      SGotoItem sNewItem;
      sNewItem.iN1 = r1 - (s * (r1 - R1)) / nsteps;
      sNewItem.iN2 = r2 - (s * (r2 - R2)) / nsteps;
      m_deGotoQueue.push_back(sNewItem);
  }
}

void CDasherModel::ClearScheduledSteps() {
  m_deGotoQueue.clear();
}


void CDasherModel::Offset(int iOffset) {
  m_Rootmin += iOffset;
  m_Rootmax += iOffset;

  m_iDisplayOffset -= iOffset;
}

void CDasherModel::AbortOffset() {
  m_Rootmin += m_iDisplayOffset;
  m_Rootmax += m_iDisplayOffset;

  m_iDisplayOffset = 0;
}

void CDasherModel::LimitRoot(int iMaxWidth) {
  m_Rootmin = ORIGIN_Y - iMaxWidth / 2;
  m_Rootmax = ORIGIN_Y + iMaxWidth / 2;
}

