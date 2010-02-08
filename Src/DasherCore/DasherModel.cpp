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

#include <iostream>
#include <cstring>
#include "../Common/Random.h"
#include "DasherModel.h"
#include "DasherView.h"
#include "Parameters.h"

#include "Event.h"
#include "DasherInterfaceBase.h"
#include "NodeCreationManager.h"
#include "DasherGameMode.h"
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

CDasherModel::CDasherModel(CEventHandler *pEventHandler,
			   CSettingsStore *pSettingsStore,
			   CNodeCreationManager *pNCManager,
			   CDasherInterfaceBase *pDasherInterface,
			   CDasherView *pView, int iOffset)
  : CFrameRate(pEventHandler, pSettingsStore) {
  m_pNodeCreationManager = pNCManager;
  m_pDasherInterface = pDasherInterface;

  m_bGameMode = GetBoolParameter(BP_GAME_MODE);
  m_iOffset = iOffset; // TODO: Set through build routine

  DASHER_ASSERT(m_pNodeCreationManager != NULL);
  DASHER_ASSERT(m_pDasherInterface != NULL);

  m_Root = NULL;

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

  SetBoolParameter(BP_CONVERSION_MODE, m_bRequireConversion);

  m_dAddProb = 0.003;

  int iNormalization = GetLongParameter(LP_NORMALIZATION);
  m_Rootmin_min = int64_min / iNormalization / 2;
  m_Rootmax_max = int64_max / iNormalization / 2;

  InitialiseAtOffset(iOffset, pView);
}

CDasherModel::~CDasherModel() {
  if(oldroots.size() > 0) {
    delete oldroots[0];
    oldroots.clear();
    // At this point we have also deleted the root - so better NULL pointer
    m_Root = NULL;
  }

  if(m_Root) {
    delete m_Root;
    m_Root = NULL;
  }
}

void CDasherModel::HandleEvent(Dasher::CEvent *pEvent) {
  CFrameRate::HandleEvent(pEvent);

  if(pEvent->m_iEventType == EV_PARAM_NOTIFY) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));

    switch (pEvt->m_iParameter) {
    case BP_CONTROL_MODE: // Rebuild the model if control mode is switched on/off
      RebuildAroundCrosshair();
      break;
    case BP_SMOOTH_OFFSET:
      if (!GetBoolParameter(BP_SMOOTH_OFFSET))
        //smoothing has just been turned off. End any transition/jump currently
        // in progress at it's current point
        AbortOffset();
      break;
    case BP_DASHER_PAUSED:
      if(GetBoolParameter(BP_SLOW_START))
	TriggerSlowdown();
      //else, leave m_iStartTime as is - will result in no slow start
      break;
    case BP_GAME_MODE:
      m_bGameMode = GetBoolParameter(BP_GAME_MODE);
      // Maybe reload something here to begin game mode?
      break;
    default:
      break;
    }
  }
  else if(pEvent->m_iEventType == EV_EDIT) {
    // Keep track of where we are in the buffer
    CEditEvent *pEditEvent(static_cast < CEditEvent * >(pEvent));

    if(pEditEvent->m_iEditType == 1) {
      m_iOffset += pEditEvent->m_sText.size();
    }
    else if(pEditEvent->m_iEditType == 2) {
      m_iOffset -= pEditEvent->m_sText.size();
    }
  }
}

void CDasherModel::Make_root(CDasherNode *pNewRoot) {
  //  std::cout << "Make root" << std::endl;

  DASHER_ASSERT(pNewRoot != NULL);
  DASHER_ASSERT(pNewRoot->Parent() == m_Root);

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
  
  m_Root = pNewRoot;

  // Update the root coordinates, as well as any currently scheduled locations
  myint range = m_Rootmax - m_Rootmin;
  m_Rootmax = m_Rootmin + (range * m_Root->Hbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
  m_Rootmin = m_Rootmin + (range * m_Root->Lbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
  
  for(std::deque<SGotoItem>::iterator it(m_deGotoQueue.begin()); it != m_deGotoQueue.end(); ++it) {
    myint r = it->iN2 - it->iN1;
    it->iN2 = it->iN1 + (r * m_Root->Hbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
    it->iN1 = it->iN1 + (r * m_Root->Lbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
  }
}

void CDasherModel::RecursiveMakeRoot(CDasherNode *pNewRoot) {
  DASHER_ASSERT(pNewRoot != NULL);
  DASHER_ASSERT(m_Root != NULL);

  if(pNewRoot == m_Root)
    return;

  // TODO: we really ought to check that pNewRoot is actually a
  // descendent of the root, although that should be guaranteed

  if(pNewRoot->Parent() != m_Root)
    RecursiveMakeRoot(pNewRoot->Parent());

  Make_root(pNewRoot);
}

// only used when BP_CONTROL changes, so not very often.
void CDasherModel::RebuildAroundCrosshair() {
  CDasherNode *pNode = Get_node_under_crosshair();
  DASHER_ASSERT(pNode != NULL);
  DASHER_ASSERT(pNode == m_pLastOutput);

  RecursiveMakeRoot(pNode);
  DASHER_ASSERT(m_Root == pNode);
  ClearRootQueue();
  m_Root->Delete_children();

  m_Root->PopulateChildren();
}

void CDasherModel::Reparent_root(int lower, int upper) {
  DASHER_ASSERT(m_Root != NULL);

  // Change the root node to the parent of the existing node. We need
  // to recalculate the coordinates for the "new" root as the user may
  // have moved around within the current root
  CDasherNode *pNewRoot;

  if(oldroots.size() == 0) {
    pNewRoot = m_Root->RebuildParent();
  }
  else {
    pNewRoot = oldroots.back();
    oldroots.pop_back();
  }

  // Return if there's no existing parent and no way of recreating one
  if(pNewRoot == NULL)
    return;

  pNewRoot->SetFlag(NF_COMMITTED, false);

  CDasherNode *pCurrent = m_Root;

  // Need to iterate through group pseudo-nodes
  while(pCurrent != pNewRoot) {

    lower = pCurrent->Lbnd();
    upper = pCurrent->Hbnd();

    pCurrent = pCurrent->Parent();
        
    myint iWidth = upper - lower;
    myint iRootWidth = m_Rootmax - m_Rootmin;
    
    // Fail and undo root creation if the new root is bigger than allowed by normalisation
    if(((myint((GetLongParameter(LP_NORMALIZATION) - upper)) / static_cast<double>(iWidth)) >
	 (m_Rootmax_max - m_Rootmax)/static_cast<double>(iRootWidth)) || 
	((myint(lower) / static_cast<double>(iWidth)) > 
	 (m_Rootmin - m_Rootmin_min) / static_cast<double>(iRootWidth))) {
      pNewRoot->OrphanChild(m_Root);
      delete pNewRoot;
      return;
    }
    
    //Update the root coordinates to reflect the new root
    m_Root = pNewRoot;
    
    m_Rootmax = m_Rootmax + (myint((GetLongParameter(LP_NORMALIZATION) - upper)) * iRootWidth / iWidth);
    m_Rootmin = m_Rootmin - (myint(lower) * iRootWidth / iWidth);
    
    for(std::deque<SGotoItem>::iterator it(m_deGotoQueue.begin()); it != m_deGotoQueue.end(); ++it) {
      iRootWidth = it->iN2 - it->iN1;
      it->iN2 = it->iN2 + (myint((GetLongParameter(LP_NORMALIZATION) - upper)) * iRootWidth / iWidth);
      it->iN1 = it->iN1 - (myint(lower) * iRootWidth / iWidth);
    }
  }
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

CDasherNode *CDasherModel::Get_node_under_crosshair() {
  DASHER_ASSERT(m_Root != NULL);

  return m_Root->Get_node_under(GetLongParameter(LP_NORMALIZATION), m_Rootmin + m_iDisplayOffset, m_Rootmax + m_iDisplayOffset, GetLongParameter(LP_OX), GetLongParameter(LP_OY));
}

CDasherNode *CDasherModel::Get_node_under_mouse(myint Mousex, myint Mousey) {
  DASHER_ASSERT(m_Root != NULL);

  return m_Root->Get_node_under(GetLongParameter(LP_NORMALIZATION), m_Rootmin + m_iDisplayOffset, m_Rootmax + m_iDisplayOffset, Mousex, Mousey);
}

void CDasherModel::DeleteTree() {
  ClearRootQueue();
  delete m_Root;
  m_Root = NULL;
}

void CDasherModel::InitialiseAtOffset(int iOffset, CDasherView *pView) {
  DeleteTree();

  m_Root = m_pNodeCreationManager->GetAlphRoot(NULL, 0,GetLongParameter(LP_NORMALIZATION), iOffset!=0, iOffset);
  m_pLastOutput = (m_Root->GetFlag(NF_SEEN)) ? m_Root : NULL;
  
  // Create children of the root...
  ExpandNode(m_Root);
	
  // Set the root coordinates so that the root node is an appropriate
  // size and we're not in any of the children

  double dFraction( 1 - (1 - m_Root->MostProbableChild() / static_cast<double>(GetLongParameter(LP_NORMALIZATION))) / 2.0 );

  int iWidth( static_cast<int>( (GetLongParameter(LP_MAX_Y) / (2.0*dFraction)) ) );

  m_Rootmin = GetLongParameter(LP_MAX_Y) / 2 - iWidth / 2;
  m_Rootmax = GetLongParameter(LP_MAX_Y) / 2 + iWidth / 2;

  m_iDisplayOffset = 0;

  //now (re)create parents, while they show on the screen
  if(pView) {
    while(pView->IsNodeVisible(m_Rootmin,m_Rootmax)) {
      CDasherNode *pOldRoot = m_Root;
      Reparent_root(m_Root->Lbnd(), m_Root->Hbnd());
      if(m_Root == pOldRoot)
	break;
    }
  }

  // TODO: See if this is better positioned elsewhere
  m_pDasherInterface->ScheduleRedraw();
}

void CDasherModel::Get_new_root_coords(dasherint X, dasherint Y, dasherint &r1, dasherint &r2, unsigned long iTime) {
  DASHER_ASSERT(m_Root != NULL);
  if(m_iStartTime == 0)
    m_iStartTime = iTime;

  int iSteps = Steps();

  double dFactor;

  if(GetBoolParameter(BP_SLOW_START) &&
     ((iTime - m_iStartTime) < GetLongParameter(LP_SLOW_START_TIME)))
    dFactor = 0.1 * (1 + 9 * ((iTime - m_iStartTime) / static_cast<double>(GetLongParameter(LP_SLOW_START_TIME))));
  else 
    dFactor = 1.0;

  iSteps = static_cast<int>(iSteps / dFactor);

  // Avoid X == 0, as this corresponds to infinite zoom
  if (X <= 0) X = 1;

  // If X is too large we risk overflow errors, so limit it
  dasherint iMaxX = (1 << 29) / iSteps;
  if (X > iMaxX) X = iMaxX;

  // Mouse coords X, Y
  // new root{min,max} r1,r2, old root{min,max} R1,R2
  const dasherint R1 = m_Rootmin;
  const dasherint R2 = m_Rootmax;
  // const dasherint Y1 = 0;
  dasherint Y2(GetLongParameter(LP_MAX_Y));
  dasherint iOX(GetLongParameter(LP_OX));
  dasherint iOY(GetLongParameter(LP_OY));


  // Calculate what the extremes of the viewport will be when the
  // point under the cursor is at the cross-hair. This is where 
  // we want to be in iSteps updates

  dasherint y1(Y - (Y2 * X) / (2 * iOX));
  dasherint y2(Y + (Y2 * X) / (2 * iOY));

  // iSteps is the number of update steps we need to get the point
  // under the cursor over to the cross hair. Calculated in order to
  // keep a constant bit-rate.

  DASHER_ASSERT(iSteps > 0);

  // Calculate the new values of y1 and y2 required to perform a single update
  // step.

  dasherint newy1, newy2, denom;

  denom = Y2 + (iSteps - 1) * (y2 - y1);
  newy1 = y1 * Y2 / denom;
  newy2 = ((y2 * iSteps - y1 * (iSteps - 1)) * Y2) / denom;

  y1 = newy1;
  y2 = newy2;

  // Calculate the minimum size of the viewport corresponding to the
  // maximum zoom.

  dasherint iMinSize(MinSize(Y2, dFactor));

  if((y2 - y1) < iMinSize) {
    newy1 = y1 * (Y2 - iMinSize) / (Y2 - (y2 - y1));
    newy2 = newy1 + iMinSize;

    y1 = newy1;
    y2 = newy2;
  }

  // If |(0,Y2)| = |(y1,y2)|, the "zoom factor" is 1, so we just translate.
  if (Y2 == y2 - y1)
    {
      r1 = R1 - y1;
      r2 = R2 - y1;
      return;
    }

  // There is a point C on the y-axis such the ratios (y1-C):(Y1-C) and
  // (y2-C):(Y2-C) are equal. (Obvious when drawn on separate parallel axes.)
  const dasherint C = (y1 * Y2) / (y1 + Y2 - y2);

  r1 = ((R1 - C) * Y2) / (y2 - y1) + C;
  r2 = ((R2 - C) * Y2) / (y2 - y1) + C;
}

bool CDasherModel::NextScheduledStep(unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted)
{
  DASHER_ASSERT(GetBoolParameter(BP_DASHER_PAUSED));
  if (m_deGotoQueue.size() == 0) return false;
  myint iNewMin, iNewMax;
  iNewMin = m_deGotoQueue.front().iN1;
  iNewMax = m_deGotoQueue.front().iN2;
  m_deGotoQueue.pop_front();

  UpdateBounds(iNewMin, iNewMax, iTime, pAdded, pNumDeleted);
  return true;
}

void CDasherModel::OneStepTowards(myint miMousex, myint miMousey, unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted) {
  //if (GetBoolParameter(BP_DASHER_PAUSED)) return false;
  m_deGotoQueue.clear();

  myint iNewMin, iNewMax;
  // works out next viewpoint
  Get_new_root_coords(miMousex, miMousey, iNewMin, iNewMax, iTime);
  
  UpdateBounds(iNewMin, iNewMax, iTime, pAdded, pNumDeleted);
}

void CDasherModel::UpdateBounds(myint iNewMin, myint iNewMax, unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted) {
  
  m_dTotalNats += log((iNewMax - iNewMin) / static_cast<double>(m_Rootmax - m_Rootmin));

  // Now actually zoom to the new location
  NewGoTo(iNewMin, iNewMax, pAdded, pNumDeleted);


  // Check whether new nodes need to be created
  ExpandNode(Get_node_under_crosshair());
  
// This'll get done again when we render the frame, later, but we use NF_SEEN
// (set here) to ensure the node under the cursor can never be collapsed
// (even when the node budget is exceeded) when we do the rendering...
  HandleOutput(pAdded, pNumDeleted);
}

void CDasherModel::NewFrame(unsigned long Time) {
  CFrameRate::NewFrame(Time);
  ///GAME MODE TEMP///Pass new frame events onto our teacher
  GameMode::CDasherGameMode* pTeacher = GameMode::CDasherGameMode::GetTeacher();
  if(m_bGameMode && pTeacher)
    pTeacher->NewFrame(Time);
}

void CDasherModel::RecursiveOutput(CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded) {
  if(pNode->Parent()) {
    if (!pNode->Parent()->GetFlag(NF_SEEN))
      RecursiveOutput(pNode->Parent(), pAdded);

    pNode->Parent()->Leave();
  }
  
  pNode->Enter();
  
  m_pLastOutput = pNode;
  pNode->SetFlag(NF_SEEN, true);
  pNode->Output(pAdded, GetLongParameter(LP_NORMALIZATION));

  // If the node we are outputting is the last one in a game target sentence, then
  // notify the game mode teacher.
  if(m_bGameMode)
    if(pNode->GetFlag(NF_END_GAME))
      GameMode::CDasherGameMode::GetTeacher()->SentenceFinished();
}

void CDasherModel::NewGoTo(myint newRootmin, myint newRootmax, Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted) {

  // Update the max and min of the root node to make iTargetMin and
  // iTargetMax the edges of the viewport.

  if(newRootmin + m_iDisplayOffset > (myint)GetLongParameter(LP_MAX_Y) / 2 - 1)
    newRootmin = (myint)GetLongParameter(LP_MAX_Y) / 2 - 1 - m_iDisplayOffset;

  if(newRootmax + m_iDisplayOffset < (myint)GetLongParameter(LP_MAX_Y) / 2 + 1)
    newRootmax = (myint)GetLongParameter(LP_MAX_Y) / 2 + 1 - m_iDisplayOffset;

  // Check that we haven't drifted too far. The rule is that we're not
  // allowed to let the root max and min cross the midpoint of the
  // screen.

  if(newRootmax < m_Rootmax_max && newRootmin > m_Rootmin_min && (newRootmax - newRootmin) > (myint)GetLongParameter(LP_MAX_Y) / 4) {
    // Only update if we're not making things big enough to risk
    // overflow. In theory we should have reparented the root well
    // before getting this far.
    //
    // Also don't allow the update if it will result in making the
    // root too small. Again, we should have re-generated a deeper
    // root in most cases, but the original root is an exception.

    m_Rootmax = newRootmax;
    m_Rootmin = newRootmin;

    m_iDisplayOffset = (m_iDisplayOffset * 90) / 100;
  }
  else {
    // TODO - force a new root to be chosen, so that we get better
    // behaviour than just having Dasher stop at this point.
  }
}

void CDasherModel::HandleOutput(Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted) {
  CDasherNode *pNewNode = Get_node_under_crosshair();
  DASHER_ASSERT(pNewNode != NULL);
  
  //  std::cout << "HandleOutput: " << m_pLastOutput << " => " << pNewNode << std::endl;
  
  CDasherNode *pLastSeen = pNewNode;
  while (pLastSeen && !pLastSeen->GetFlag(NF_SEEN))
    pLastSeen = pLastSeen->Parent();
  
  while (m_pLastOutput != pLastSeen) {
    m_pLastOutput->Undo();
    m_pLastOutput->Leave(); //Should we? I think so, but the old code didn't...?
    m_pLastOutput->SetFlag(NF_SEEN, false);
    // TODO: Is this the right place to trap output?
    if(pNumDeleted != NULL)
      (*pNumDeleted) += m_pLastOutput->m_iNumSymbols;
    
    m_pLastOutput = m_pLastOutput->Parent();
    if (m_pLastOutput) m_pLastOutput->Enter();
  }
  
  if(!pNewNode->GetFlag(NF_SEEN)) {
    RecursiveOutput(pNewNode, pAdded);
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

  // We get here if all our children (groups) and grandchildren (symbols) are created.
  // So lets find the correct letters.
  ///GAME MODE TEMP///////////
  // If we are in GameMode, then we do a bit of cooperation with the teacher object when we create
  // new children.

  GameMode::CDasherGameMode* pTeacher = GameMode::CDasherGameMode::GetTeacher();
  if(m_bGameMode && pNode->GetFlag(NF_GAME) && pTeacher )
  {
    std::string strTargetUtf8Char(pTeacher->GetSymbolAtOffset(pNode->m_iOffset + 1));
      
    // Check if this is the last node in the sentence...
    if(strTargetUtf8Char == "GameEnd")
	    pNode->SetFlag(NF_END_GAME, true);
	  else if (!pNode->GameSearchChildren(strTargetUtf8Char)) {
      // Target character not found - not in our current alphabet?!?!
      // Let's give up!
      pNode->SetFlag(NF_END_GAME, true); 
    }
  }
  ////////////////////////////
  

}

bool CDasherModel::RenderToView(CDasherView *pView, CExpansionPolicy &policy) {

  DASHER_ASSERT(pView != NULL);
  DASHER_ASSERT(m_Root != NULL);

  // XXX we HandleOutput in RenderToView
  // DASHER_ASSERT(Get_node_under_crosshair() == m_pLastOutput);

  bool bReturnValue = false;
  std::vector<std::pair<myint,bool> > vGameTargetY;
  
  // The Render routine will fill iGameTargetY with the Dasher Coordinate of the 
  // youngest node with NF_GAME set. The model is responsible for setting NF_GAME on
  // the appropriate Nodes.
  pView->Render(m_Root, m_Rootmin + m_iDisplayOffset, m_Rootmax + m_iDisplayOffset, policy, true, &vGameTargetY);  

  /////////GAME MODE TEMP//////////////
  if(m_bGameMode)
    if(GameMode::CDasherGameMode* pTeacher = GameMode::CDasherGameMode::GetTeacher())
      pTeacher->SetTargetY(vGameTargetY);
  //////////////////////////////////////

  // TODO: Fix up stats
  // TODO: Is this the right way to handle this?
  HandleOutput(NULL, NULL);

  //ACL Off-screen nodes (zero collapse cost) will have been collapsed already.
  //Hence, this acts to maintain the node budget....or whatever the queue's policy is!
  if (policy.apply(m_pNodeCreationManager,this)) bReturnValue=true;
  
  return bReturnValue;
}

bool CDasherModel::CheckForNewRoot(CDasherView *pView) {
  DASHER_ASSERT(m_Root != NULL);
  // TODO: pView is redundant here

#ifdef DEBUG
  CDasherNode *pOldNode = Get_node_under_crosshair();
#endif

  CDasherNode *root(m_Root);

  if(!(m_Root->GetFlag(NF_SUPER))) {
    Reparent_root(root->Lbnd(), root->Hbnd());
    return(m_Root != root);
  }

  CDasherNode *pNewRoot = NULL;

  for (CDasherNode::ChildMap::const_iterator it = m_Root->GetChildren().begin(); it != m_Root->GetChildren().end(); it++) {
    if ((*it)->GetFlag(NF_SUPER)) {
      //at most one child should have NF_SUPER set...
      DASHER_ASSERT(pNewRoot == NULL);
      pNewRoot = *it;
#ifndef DEBUG
      break;
#endif
    }
  }
  ////GAME MODE TEMP - only change the root if it is on the game path/////////
  if (pNewRoot && (!m_bGameMode || pNewRoot->GetFlag(NF_GAME))) {
    m_Root->DeleteNephews(pNewRoot);
    RecursiveMakeRoot(pNewRoot);
  }

  DASHER_ASSERT(Get_node_under_crosshair() == pOldNode);

  return false;
}


void CDasherModel::ScheduleZoom(dasherint X, dasherint Y, int iMaxZoom)
{
  // 1 = min, 2 = max. y1, y2 is the length we select from Y1, Y2. With
  // that ratio we calculate the new root{min,max} r1, r2 from current R1, R2.
  const int nsteps = GetLongParameter(LP_ZOOMSTEPS);
  const int safety = GetLongParameter(LP_S); // over safety_denom gives %
  const int safety_denom = 1024;
  const int ymax = GetLongParameter(LP_MAX_Y);
  const int scale = ymax; // (0,1) -> (ymin=0,ymax)

  // (X,Y) is mouse position in dasher coordinates
  // Prevent clicking too far to the right => y1 <> y2 see below
  if (X < 2) X = 2;

  // Lines with gradient +/- 1 passing through (X,Y) intersect y-axis at
  dasherint y1 = Y - X;     // y =  x + (Y - X)
  dasherint y2 = Y + X;     // y = -x + (Y + X)

  // Rename for readability.
  const dasherint Y1 = 0;
  const dasherint Y2 = ymax;
  const dasherint R1 = m_Rootmin;
  const dasherint R2 = m_Rootmax;

  // So, want to zoom (y1 - safety/2, y2 + safety/2) -> (Y1, Y2)
  // Adjust y1, y2 for safety margin
  dasherint ds = (safety * scale) / (2 * safety_denom);
  y1 -= ds;
  y2 += ds;

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
  // iMaxZoom seems to be in tenths
      if (iMaxZoom != 0 && 10 * (r2 - r1) > iMaxZoom * (R2 - R1)) {
          r1 = ((R1 - C) * iMaxZoom) / 10 + C;
          r2 = ((R2 - C) * iMaxZoom) / 10 + C;
      }
  }

  // sNewItem seems to contain a list of root{min,max} for the frames of the
  // zoom, so split r -> R into n steps, with accurate R
  m_deGotoQueue.clear();
  for (int s = nsteps - 1; s >= 0; --s) {
      SGotoItem sNewItem;
      sNewItem.iN1 = r1 - (s * (r1 - R1)) / nsteps;
      sNewItem.iN2 = r2 - (s * (r2 - R2)) / nsteps;
      m_deGotoQueue.push_back(sNewItem);
  }
}


void CDasherModel::Offset(int iOffset) {
  m_Rootmin += iOffset;
  m_Rootmax += iOffset;

  if (GetBoolParameter(BP_SMOOTH_OFFSET))
    m_iDisplayOffset -= iOffset;
} 

void CDasherModel::AbortOffset() {
  m_Rootmin += m_iDisplayOffset;
  m_Rootmax += m_iDisplayOffset;

  m_iDisplayOffset = 0;
}

void CDasherModel::LimitRoot(int iMaxWidth) {
  m_Rootmin = GetLongParameter(LP_MAX_Y) / 2 - iMaxWidth / 2;
  m_Rootmax = GetLongParameter(LP_MAX_Y) / 2 + iMaxWidth / 2;
}

void CDasherModel::SetOffset(int iLocation, CDasherView *pView) {
  if(iLocation == m_iOffset)
    return; // We're already there
  
  //  std::cout << "Initialising at offset: " << iLocation << std::endl;

  // TODO: Special cases, ie this can be done without rebuilding the
  // model

  m_iOffset = iLocation;

  // Now actually rebuild the model
  InitialiseAtOffset(iLocation, pView);
}

void CDasherModel::SetControlOffset(int iOffset) {
  // This is a hack, making many dubious assumptions which happen to
  // work right now.
  CDasherNode *pNode = Get_node_under_crosshair();
  
  pNode->SetControlOffset(iOffset);
}
