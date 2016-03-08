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


//If preprocessor variable DEBUG_DYNAMICS is defined, will display the difference
// between computed (approximate) one-step motion, and ideal/exact motion (using pow()).
//#define DEBUG_DYNAMICS

CDasherModel::CDasherModel() {
  
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

  for(std::deque<pair<myint,myint> >::iterator it(m_deGotoQueue.begin()); it != m_deGotoQueue.end(); ++it) {
    //Some of these co-ordinate pairs can be bigger than m_Rootmin_min - m_Rootmax_max,
    // hence using unsigned type...
    const uint64 r = it->second - it->first;
    it->second = it->first + (r * m_Root->Hbnd()) / NORMALIZATION;
    it->first += (r * m_Root->Lbnd()) / NORMALIZATION;
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

  for(std::deque<pair<myint,myint> >::iterator it(m_deGotoQueue.begin()); it != m_deGotoQueue.end(); ++it) {
    iRootWidth = it->second - it->first;
    it->second += (myint(NORMALIZATION - upper) * iRootWidth / iRange);
    it->first -= (myint(lower) * iRootWidth / iRange);
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

void CDasherModel::SetNode(CDasherNode *pNewRoot) {

  AbortOffset();
  ClearRootQueue();
  delete m_Root;

  m_Root = pNewRoot;

  // Create children of the root...
  ExpandNode(m_Root);

  // Set the root coordinates so that the root node is an appropriate
  // size and we're not in any of the children
  m_Root->SetFlag(NF_SEEN, true); //(but we are in the node itself)
  m_pLastOutput = m_Root;

  double dFraction( 1 - (1 - m_Root->MostProbableChild() / static_cast<double>(NORMALIZATION)) / 2.0 );

  //TODO somewhere round here, old code checked whether the InputFilter implemented
  // GetMinWidth, if so called LimitRoot w/that width - i.e., make sure iWidth
  // is no more than that minimum. Should we do something similar here???

  int iWidth( static_cast<int>( MAX_Y / (2.0*dFraction) ) );

  m_Rootmin = MAX_Y / 2 - iWidth / 2;
  m_Rootmax = MAX_Y / 2 + iWidth / 2;
}

int CDasherModel::GetOffset() {
  return m_pLastOutput ? m_pLastOutput->offset()+1 : m_Root ? m_Root->offset()+1 : 0;
}

CDasherNode *CDasherModel::Get_node_under_crosshair() {
  return m_pLastOutput;
}

bool CDasherModel::NextScheduledStep()
{
  if (m_deGotoQueue.size() == 0) return false;
  myint newRootmin(m_deGotoQueue.front().first), newRootmax(m_deGotoQueue.front().second);
  m_deGotoQueue.pop_front();

  m_dTotalNats += log((newRootmax - newRootmin) / static_cast<double>(m_Rootmax - m_Rootmin));

  m_iDisplayOffset = (m_iDisplayOffset * 90) / 100;

  // Now actually zoom to the new location

  while (newRootmax >= m_Rootmax_max || newRootmin <= m_Rootmin_min) {
    // can't make existing root any bigger because of overflow. So force a new root
    // to be chosen (so that Dasher doesn't just stop!)...

    //pick _child_ covering crosshair...
    const myint iWidth(m_Rootmax-m_Rootmin);
    for (CDasherNode::ChildMap::const_iterator it = m_Root->GetChildren().begin(); ;) {
      CDasherNode *pChild(*it);
      DASHER_ASSERT(m_Rootmin + ((pChild->Lbnd() * iWidth) / NORMALIZATION) <= ORIGIN_Y);
      if (m_Rootmin + ((pChild->Hbnd() * iWidth) / NORMALIZATION) > ORIGIN_Y) {
        //found child to make root. proceed only if new root is on the game path....
        if (m_Root->GetFlag(NF_GAME) && !pChild->GetFlag(NF_GAME)) {
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
        m_deGotoQueue.push_back(pair<myint,myint>(newRootmin,newRootmax));
        //...when we make pChild the root...
        Make_root(pChild);
        //...we can retrieve new, equivalent, coordinates for it
        newRootmin = m_deGotoQueue.back().first; newRootmax = m_deGotoQueue.back().second;
        m_deGotoQueue.pop_back();
        // (note that the next check below will make sure these coords do cover (0, ORIGIN_Y))
        break;
      }
      ++it;
      DASHER_ASSERT(it != m_Root->GetChildren().end()); //must find a child!
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

///A very approximate square root. Finds the square root of (just) the
/// most significant bit, then two iterations of Newton.
inline dasherint mysqrt(dasherint in) {
  //1. Find greatest i satisfying 1<<(i<<1) < in; let rt = 1<<i be first approx
  // but find by binary chop: at first double each time..
  dasherint i=1;
  while (dasherint(1)<<4*i < in) i*=2;
  //then try successively smaller bits.
  for (dasherint test=i; test/=2;)
    if (dasherint(1)<<2*(i+test) < in) i+=test;
  //so, first approx:
  dasherint rt = 1<<i;
  rt = (rt+in/rt)/2;//better
  return (rt+in/rt)/2;//better still
  
  //Some empirical results (from DEBUG_DYNAMICS, at about 40fps with XLimit=400)
  // with one iteration, error in rate of data entry is ~~10% near xhair, falls
  // as we get further away, then abruptly jumps up to 30% near the x limit
  // (and beyond it, but also before reaching it).
  //With two iterations, error is 0-1% near xhair, gradually rising to 10%
  // near/at the x limit.
  //However, reversing is less good - it can go twice as fast at extreme x...
}

void CDasherModel::ScheduleOneStep(dasherint y1, dasherint y2, int nSteps, int limX, bool bExact) {
  
  m_deGotoQueue.clear();
  
  // Rename for readability.
  const dasherint R1 = m_Rootmin;
  const dasherint R2 = m_Rootmax;
  
  // Calculate the bounds of the root node when the target range y1-y2
  // fills the viewport.
  // This is where we want to be in iSteps updates
  dasherint targetRange=y2-y1;

  const dasherint r1 = MAX_Y*(R1-y1)/targetRange;
  const dasherint r2 = MAX_Y*(R2-y1)/targetRange;
  
  dasherint m1=(r1-R1),m2=(r2-R2);
  
  //Any interpolation (R1,R2) + alpha*(m1,m2) moves along the correct path.
  // Just have to decide how far, i.e. what alpha.
  
  //Possible schemes (using rw=r2-r1, Rw=R2-R1)
  // (Note: if y2-y1 == MAX_Y, alpha=1/nSteps is correct, and in some schemes must be a special case)
  // alpha = (pow(rw/Rw,1/nSteps)-1)*rW / (rw-Rw) : correct/ideal, but uses pow
  // alpha = 1/nSteps : moves forwards too fast, reverses too slow (correct for translation)
  // alpha = MAX_Y / (MAX_Y + (nSteps-1)*(y2-y1)) : (same eqn as old Dasher) more so! reversing ~~ 1/3 ideal speed, and maxes out at moderate dasherX.
  // alpha = (y2-y1) / (MAX_Y*(nSteps-1) + y2-y1) : too slow forwards, reverses too quick
  //We are using:
  // alpha = sqrt(y2-y1) / (sqrt(MAX_Y)*(nSteps-1) + sqrt(y2-y1))
  //      with approx sqrt on y2-y1
  //this is pretty good going forwards, but reverses faster than the ideal, on the order of 2*
  
  if (targetRange < 2*limX) {
#ifdef DEBUG_DYNAMICS
    {
      const dasherint Rw=R2-R1, rw=r2-r1;
      dasherint apsq = mysqrt(y2-y1);
      dasherint denom = 64*(nSteps-1) + apsq;
      dasherint nw = (rw*apsq + Rw*64*(nSteps-1))/denom;
      double bits = (log(nw) - log(Rw))/log(2);
      std::cout << "Too fast at X " << (y2-y1)/2 << ": would enter " << bits << "b = " << (bits*nSteps) << " in " << nSteps << "steps; will now enter ";
    }
#endif
    //atm we have Rw=R2-R1, rw=r2-r1 = Rw*MAX_Y/targetRange, (m1,m2) to take us there
    
    //if targetRange were = 2*limX, we'd have rw' = Rw*MAX_Y/2*limX < rw
    //the movement necessary to take us to rw', rather than rw, is thus:
    // (m1',m2') = (m1,m2) * (rw' - Rw) / (rw-Rw) => scale m1,m2 by (rw'-Rw)/(rw-Rw)
    // = (Rw*MAX_Y/(2*limX) - Rw)/(Rw*MAX_Y/targetRange-Rw)
    // = (MAX_Y/(2*limX)-1) / (MAX_Y/targetRange-1)
    // = (MAX_Y-(2*limX))/(2*limX) / ((MAX_Y-targetRange)/targetRange)
    // = (MAX_Y-(2*limX)) / (2*limX) * targetRange / (MAX_Y-targetRange)
    {
      const dasherint n=targetRange*(MAX_Y-2*limX), d=(MAX_Y-targetRange)*2*limX;
      bool bOver=max(abs(m1),abs(m2))>std::numeric_limits<dasherint>::max()/n;
      if (bOver) {
        //std::cout << "Overflow in max-speed-limit " << m1 << "," << m2 << " =wd> " << ((m1*n)/d) << "," << ((m2*n)/d);
        //so do it a harder way, but which uses smaller intermediates:
        // (Yes, this is valid even if !bOver. Could use it all the time?)
        m1 = (m1/d)*n + ((m1 % d) * n) / d;
        m2 = (m2/d)*n + ((m2 % d) * n) / d;
        //std::cout << " => " << m1 << "," << m2 << std::endl;
      } else {
        m1 = (m1*n)/d;
        m2 = (m2*n)/d;
      } 
    }
    //then make the stepping function, which follows, behave as if we were at limX:
    targetRange=2*limX;
  }
  
#ifndef DEBUG_DYNAMICS
  if (bExact) {
    //#else, for DEBUG_DYNAMICS, we compute the exact movement either way, to compare.
#endif
    double frac;
    if (targetRange == MAX_Y) {
      frac=1.0/nSteps;
    } else {
      double tr(targetRange);
      //expansion factor (of root node) for one step, post-speed-limit
      double eFac = pow(MAX_Y/tr,1.0/nSteps);
      //fraction of way along linear interpolation Rw->rw that yields that width:
      // = (Rw*eFac - Rw) / (rw-Rw)
      // = Rw * (eFac-1.0) / (Rw*MAX_Y/tr-Rw)
      // = (eFac - 1.0) / (MAX_Y/tr - 1.0)
      frac = (eFac-1.0) /  (MAX_Y/tr - 1.0);
    }
#ifdef DEBUG_DYNAMICS
    const dasherint m1t=m1*frac, m2t=m2*frac; //keep original m1,m2 to compare
#else
    m1*=frac; m2*=frac;
  } else //conditional - only do one of exact/approx
#endif
  { //begin block A (regardless of #ifdef)
    
    //approximate dynamics: interpolate
    // apsq parts rw to 64*(nSteps-1) parts Rw
    // (no need to compute target width)
    dasherint apsq = mysqrt(targetRange);
    dasherint denom = 64*(nSteps-1) + apsq;
    
    // so new width nw = (64*(nSteps-1)*Rw + apsq*rw)/denom
    // = Rw*(64*(nSteps-1) + apsq*MAX_Y/targetRange)/denom
    m1 = (m1*apsq)/denom, m2=(m2*apsq)/denom;
#ifdef DEBUG_DYNAMICS
    std::cout << "Move " << m1 << "," << m2 << " should be " << m1t << "," << m2t;
    double dActualBits = (log((R2+m2)-(R1+m1))-log(R2-R1))/log(2);
    double dDesiredBits = (log((R2+m2t)-(R1+m1t))-log(R2-R1))/log(2);
    std::cout << " enters " << dActualBits << "b = " << (dActualBits*nSteps) << " in " << nSteps << "steps, should be "
    << dDesiredBits << "=>" << (dDesiredBits*nSteps) << ", error " << int(abs(dDesiredBits-dActualBits)*100/dDesiredBits) << "%" << std::endl;
    if (bExact)
      m1=m1t, m2=m2t; //overwrite approx values (we needed them somewhere!)
#endif
  } //end block A (regardless of #ifdef)
  
  m_deGotoQueue.push_back(pair<myint,myint>(R1+m1, R2+m2));
}

void CDasherModel::OutputTo(CDasherNode *pNewNode) {
  //first, recurse back up to last seen node (must be processed ancestor-first)
  if (pNewNode && !pNewNode->GetFlag(NF_SEEN)) {
    OutputTo(pNewNode->Parent());

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
      m_pLastOutput->SetFlag(NF_SEEN, false);

      m_pLastOutput = m_pLastOutput->Parent();
      DASHER_ASSERT(m_pLastOutput || !pNewNode); //if m_pLastOutput null, then pNewNode is too.
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
        // Stay on the game path, if there is one (!)
        (!m_Root->GetFlag(NF_GAME) || pNewRoot->GetFlag(NF_GAME))) {
      Make_root(pNewRoot);
    } else
      break;
  }

}

void CDasherModel::ScheduleZoom(dasherint y1, dasherint y2, int nsteps) {
  
  m_deGotoQueue.clear();
  
  // Rename for readability.
  const dasherint R1 = m_Rootmin;
  const dasherint R2 = m_Rootmax;

  const dasherint r1 = MAX_Y*(m_Rootmin-y1)/(y2-y1);
  const dasherint r2 = MAX_Y*(m_Rootmax-y1)/(y2-y1);

  //We're going to interpolate in steps whose size starts at nsteps
  // and decreases by one each time - so cumulatively: 
  // <nsteps> <2*nsteps-1> <3*nsteps-3> <4*nsteps-6>
  // (until the next value is the same as the previous)
  //These will sum to / reach (triangular number formula):
  const int max((nsteps*(nsteps+1))/2);
  //heights:
  const myint oh(R2-R1), nh(r2-r1);
  //log(the amount by which we wish to multiply the height):
  const double logHeightMul(nh==oh ? 0 : log(nh/static_cast<double>(oh)));
  for (int s = nsteps; nsteps>1; s+=(--nsteps)) {
    double dFrac; //(linear) fraction of way from oh to nh...
    if (nh==oh)
      dFrac = s/static_cast<double>(max);
    else {
      //interpolate expansion logarithmically to get new height:
      const double h(oh*exp((logHeightMul*s)/max));
      //then treat that as a fraction of the way between oh to nh linearly
      dFrac = (h-oh)/(nh-oh);
    }
    //and use that fraction to interpolate from R to r
    m_deGotoQueue.push_back(pair<myint,myint>(R1+dFrac*(r1-R1), R2+dFrac*(r2-R2)));
  }
  //final point, done accurately/simply:
  m_deGotoQueue.push_back(pair<myint,myint>(r1,r2));
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

