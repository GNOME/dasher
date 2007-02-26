// DasherModel.h
//
// Copyright (c) 2001-2005 David Ward

#include "../Common/Common.h"

#include <iostream>
#include "../Common/Random.h"
#include "DasherModel.h"
#include "DasherView.h"
#include "Parameters.h"

using namespace Dasher;
using namespace std;

#include "Event.h"
#include "DasherInterfaceBase.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"
#include "LanguageModelling/DictLanguageModel.h"
#include "LanguageModelling/MixtureLanguageModel.h"
#include "NodeCreationManager.h"

using namespace Dasher;
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

// FIXME - need to get node deletion working properly and implement reference counting

// CDasherModel

CDasherModel::CDasherModel(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CNodeCreationManager *pNCManager, CDasherInterfaceBase *pDashIface, CDasherView *pView, int iOffset, bool bGameMode, const std::string &strGameModeText)
:CDasherComponent(pEventHandler, pSettingsStore), m_pDasherInterface(pDashIface) {

  m_Root = 0;
  //  m_pLanguageModel =NULL; 

  m_Rootmin = 0;
  m_Rootmax = 0;
  m_Rootmin_min = 0;
  m_Rootmax_max = 0;
  m_dAddProb = 0.0;
  m_dMaxRate = 0.0;
  m_iTargetOffset = 0;
  m_dTotalNats = 0.0;
  m_bGameMode = bGameMode;

  m_iOffset = iOffset;
 
  // TODO: Need to rationalise the require conversion methods

#ifdef JAPANESE
  m_bRequireConversion = true;
#else
  m_bRequireConversion = false;
#endif

  SetBoolParameter(BP_CONVERSION_MODE, m_bRequireConversion);

  // Set max bitrate in the FrameRate class
  m_dMaxRate = GetLongParameter(LP_MAX_BITRATE) / 100.0;
  m_fr.SetMaxBitrate(m_dMaxRate);
  m_dAddProb = 0.003;

  int iNormalization = GetLongParameter(LP_NORMALIZATION);
  m_Rootmin_min = int64_min / iNormalization / 2;
  m_Rootmax_max = int64_max / iNormalization / 2;

  m_pNodeCreationManager = pNCManager;

  //  m_pLanguageModel = m_pNodeCreationManager->GetLanguageModel();
  //  LearnContext = m_pNodeCreationManager->GetLearnContext();

  //  m_bContextSensitive = true;

  // TODO: Do something sensible here
  //  Start();

  InitialiseAtOffset(iOffset, pView);
}

CDasherModel::~CDasherModel() {

  if(oldroots.size() > 0) {
    delete oldroots[0];
    oldroots.clear();
    // At this point we have also deleted the root - so better NULL pointer
    m_Root = NULL;
  }

  if(m_Root)
    delete m_Root;
}

void CDasherModel::HandleEvent(Dasher::CEvent *pEvent) {

  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));

    switch (pEvt->m_iParameter) {
    case LP_MAX_BITRATE: // Delibarate fallthrough
    case LP_BOOSTFACTOR: // Deliberate fallthrough
    case LP_SPEED_DIVISOR:
      m_dMaxRate = GetLongParameter(LP_MAX_BITRATE) * GetLongParameter(LP_BOOSTFACTOR) / 100 / static_cast<double>(GetLongParameter(LP_SPEED_DIVISOR));
      m_fr.SetMaxBitrate(m_dMaxRate);
      break;
    case BP_CONTROL_MODE: // Rebuild the model if control mode is switched on/off
      RebuildAroundNode(Get_node_under_crosshair());
      break;
    case BP_DELAY_VIEW:
      MatchTarget(GetBoolParameter(BP_DELAY_VIEW));
      break;
    case BP_DASHER_PAUSED:
      if(GetBoolParameter(BP_SLOW_START))
	m_iStartTime = 0;
      else
	m_iStartTime = 1;
      break;
    default:
      break;
    }
  }
  else if(pEvent->m_iEventType == 2) {
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
  if(!pNewRoot->NodeIsParent(m_Root))
    return;

  m_Root->SetFlag(NF_COMMITTED, true);

  // Remove all children of current root other than the new root and
  // push the new root onto the stack

  // TODO: Is the stack necessary at all? We may as well just keep the
  // existing data structure?
  m_Root->DeleteNephews(pNewRoot);

  oldroots.push_back(m_Root);

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
  // TODO: Macros for this kind of thing
  if(!pNewRoot)
    return;

  // TODO: we really ought to check that pNewRoot is actually a
  // descendent of the root, although that should be guaranteed

  if(!pNewRoot->NodeIsParent(m_Root))
    RecursiveMakeRoot(pNewRoot->Parent());

  // Don't try to reparent subnodes
  if(!pNewRoot->GetFlag(NF_SUBNODE))
    Make_root(pNewRoot);
}

void CDasherModel::RebuildAroundNode(CDasherNode *pNode) {
  RecursiveMakeRoot(pNode);
  ClearRootQueue();
  m_Root->Delete_children();
  m_Root->m_pNodeManager->PopulateChildren(m_Root);
}

void CDasherModel::Reparent_root(int lower, int upper) {
  // Change the root node to the parent of the existing node. We need
  // to recalculate the coordinates for the "new" root as the user may
  // have moved around within the current root

  CDasherNode *pNewRoot;

  if(oldroots.size() == 0) {
    // Figure out how many nodes are between the crosshair and the
    // current root. This is used to figure out the cursor position of
    // the current root in order to obtain context.

    // TODO: store cursor position in the node itself. This would make
    // life a lot easier in several ways
    //    CDasherNode *pCurrentNode(Get_node_under_crosshair());
    int iGenerations(0);

    // TODO: This was failing - figure out why
    
//     while(pCurrentNode != m_Root) {
//       ++iGenerations;
//       pCurrentNode = pCurrentNode->Parent();
//     }

    // Tell the node manager to rebuild the parent
    pNewRoot = m_Root->m_pNodeManager->RebuildParent(m_Root, iGenerations);

  }
  else {
    pNewRoot = oldroots.back();
    oldroots.pop_back();
  }

  // Return if there's no existing parent and no way of recreating one
  if(!pNewRoot) { 
    return;
  }

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
    if(!(pNewRoot->GetFlag(NF_SUBNODE)) && (((myint((GetLongParameter(LP_NORMALIZATION) - upper)) / static_cast<double>(iWidth)) 
	> (m_Rootmax_max - m_Rootmax)/static_cast<double>(iRootWidth)) || 
       ((myint(lower) / static_cast<double>(iWidth)) 
	> (m_Rootmin - m_Rootmin_min) / static_cast<double>(iRootWidth)))) {
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
    return m_Root->Get_node_under(GetLongParameter(LP_NORMALIZATION), m_Rootmin + m_iTargetOffset, m_Rootmax + m_iTargetOffset, GetLongParameter(LP_OX), GetLongParameter(LP_OY));
}

CDasherNode *CDasherModel::Get_node_under_mouse(myint Mousex, myint Mousey) {
  return m_Root->Get_node_under(GetLongParameter(LP_NORMALIZATION), m_Rootmin + m_iTargetOffset, m_Rootmax + m_iTargetOffset, Mousex, Mousey);
}

// void CDasherModel::Start() {

//   // FIXME - re-evaluate this function and SetContext...

// //              m_pEditbox->get_new_context(ContextString,5);

//   std::cout << "Called CDasherModel::Start()" << std::endl;

//   std::string strNewContext("");

//   SetContext(strNewContext);    // FIXME - REALLY REALLY broken!

//   CEditContextEvent oEvent(5);

//   InsertEvent(&oEvent);

//   // FIXME - what if we don't get a reply?

// //      m_pLanguageModel->ReleaseNodeContext(therootcontext);
// //      ppmmodel->dump();
// //      dump();

// }

void CDasherModel::DeleteTree() {
  if(oldroots.size() > 0) {
    delete oldroots[0];
    oldroots.clear();
    m_Root = NULL;
  }

  delete m_Root;
}

void CDasherModel::InitialiseAtOffset(int iOffset, CDasherView *pView) {
  DeleteTree();

  if(iOffset == 0)
    m_Root = GetRoot(0, NULL, 0,GetLongParameter(LP_NORMALIZATION), NULL);
  else {
    // Get the most recent character
    std::string strContext = m_pDasherInterface->GetContext(iOffset - 1, 1);

    CAlphabetManager::SRootData oData;

    oData.szContext = new char[strContext.size() + 1];
    strcpy(oData.szContext, strContext.c_str());

    oData.iOffset = iOffset;

    m_Root = GetRoot(0, NULL, 0, GetLongParameter(LP_NORMALIZATION), &oData);

    delete[] oData.szContext;
  }

  // Create children of the root
  // TODO: What about parents?

  Recursive_Push_Node(m_Root, 0);

  // Set the root coordinates so that the root node is an appropriate
  // size and we're not in any of the children

  double dFraction( 1 - (1 - m_Root->MostProbableChild() / static_cast<double>(GetLongParameter(LP_NORMALIZATION))) / 2.0 );

  int iWidth( static_cast<int>( (GetLongParameter(LP_MAX_Y) / (2.0*dFraction)) ) );

  m_Rootmin = GetLongParameter(LP_MAX_Y) / 2 - iWidth / 2;
  m_Rootmax = GetLongParameter(LP_MAX_Y) / 2 + iWidth / 2;

  m_iTargetOffset = 0;

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

// void CDasherModel::SetContext(std::string &sNewContext) {

//   std::cout << "Called CDasherModel::SetContext()" << std::endl;

//   // TODO: This is probably the area most significantly in need of a
//   // rethink. Get the language model out of here!  
//   m_deGotoQueue.clear();

//   DeleteTree();

//   // CLanguageModel::Context therootcontext = m_pLanguageModel->CreateEmptyContext();

//   if(sNewContext.size() == 0) {
//     m_Root = GetRoot(0, NULL, 0,GetLongParameter(LP_NORMALIZATION), NULL);
    
//     //    EnterText(therootcontext, ". ");  
//   }
//   else {
//     std::vector<symbol> vSymbols;
//     //    m_pLanguageModel->SymbolAlphabet().GetAlphabetPointer()->GetSymbols(&vSymbols, &sNewContext, false);
    
//     int iRootSymbol(vSymbols[vSymbols.size()-1]);
    
//     m_Root = GetRoot(0, NULL, 0,GetLongParameter(LP_NORMALIZATION), &iRootSymbol);
    
//     //    EnterText(therootcontext, sNewContext);  
//   }

//   // TODO: Reimplement
//   //  m_pLanguageModel->ReleaseContext(LearnContext);
//   // LearnContext = m_pLanguageModel->CloneContext(therootcontext);

//   // TODO: Reimplement
//   //  m_Root->SetContext(therootcontext);   // node takes control of the context
//   Recursive_Push_Node(m_Root, 0);

//   double dFraction( 1 - (1 - m_Root->MostProbableChild() / static_cast<double>(GetLongParameter(LP_NORMALIZATION))) / 2.0 );

//   int iWidth( static_cast<int>( (GetLongParameter(LP_MAX_Y) / (2.0*dFraction)) ) );

//   m_Rootmin = GetLongParameter(LP_MAX_Y) / 2 - iWidth / 2;
//   m_Rootmax = GetLongParameter(LP_MAX_Y) / 2 + iWidth / 2;

//   m_iTargetOffset = 0;

// //   m_iTargetMin = m_Rootmin;
// //   m_iTargetMax = m_Rootmax;
// }

void CDasherModel::Get_new_root_coords(myint Mousex, myint Mousey, myint &iNewMin, myint &iNewMax, unsigned long iTime) {

  // Avoid Mousex == 0, as this corresponds to infinite zoom
  if(Mousex <= 0) {
    Mousex = 1;
  }

  if(m_iStartTime == 0)
    m_iStartTime = iTime;

  int iSteps = m_fr.Steps();

  double dFactor;

  if(IsSlowdown(iTime))
    dFactor = 0.1 * (1 + 9 * ((iTime - m_iStartTime) / static_cast<double>(GetLongParameter(LP_SLOW_START_TIME))));
  else 
    dFactor = 1.0;

  iSteps = static_cast<int>(iSteps / dFactor);

  // If Mousex is too large we risk overflow errors, so limit it
  int iMaxX = (1 << 29) / iSteps;

  if(Mousex > iMaxX)
    Mousex = iMaxX;

  // Cache some results so we don't do a huge number of parameter lookups

  myint iMaxY(GetLongParameter(LP_MAX_Y));
  myint iOX(GetLongParameter(LP_OX));
  myint iOY(GetLongParameter(LP_OY));

  // Calculate what the extremes of the viewport will be when the
  // point under the cursor is at the cross-hair. This is where 
  // we want to be in iSteps updates

  int iTargetMin(Mousey - ((myint)iMaxY * Mousex) / (2 * (myint)iOX));
  int iTargetMax(Mousey + ((myint)iMaxY * Mousex) / (2 * (myint)iOY));

  // iSteps is the number of update steps we need to get the point
  // under the cursor over to the cross hair. Calculated in order to
  // keep a constant bit-rate.

  DASHER_ASSERT(iSteps > 0);

  // Calculate the new values of iTargetMin and iTargetMax required to
  // perform a single update step. Note that the slightly awkward
  // expressions are in order to reproduce the behaviour of the old
  // algorithm

  int iNewTargetMin;
  int iNewTargetMax;

  iNewTargetMin = (iTargetMin * iMaxY / (iMaxY + (iSteps - 1) * (iTargetMax - iTargetMin)));

  iNewTargetMax = ((iTargetMax * iSteps - iTargetMin * (iSteps - 1)) * iMaxY) / (iMaxY + (iSteps - 1) * (iTargetMax - iTargetMin));

  iTargetMin = iNewTargetMin;
  iTargetMax = iNewTargetMax;

  // Calculate the minimum size of the viewport corresponding to the
  // maximum zoom.

  myint iMinSize(m_fr.MinSize(iMaxY, dFactor));

  if((iTargetMax - iTargetMin) < iMinSize) {
    iNewTargetMin = iTargetMin * (iMaxY - iMinSize) / (iMaxY - (iTargetMax - iTargetMin));
    iNewTargetMax = iNewTargetMin + iMinSize;

    iTargetMin = iNewTargetMin;
    iTargetMax = iNewTargetMax;
  }

  iNewMin = (((m_Rootmin - iTargetMin) * (myint)GetLongParameter(LP_MAX_Y)) / (iTargetMax - iTargetMin));
  iNewMax = (((m_Rootmax - iTargetMax) * (myint)GetLongParameter(LP_MAX_Y)) / (iTargetMax - iTargetMin) + (myint)GetLongParameter(LP_MAX_Y));
}

bool CDasherModel::UpdatePosition(myint miMousex, myint miMousey, unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted) {
  // Clear out parameters that might get passed in to track user activity
  if (pAdded != NULL)
    pAdded->clear();
  if (pNumDeleted != NULL)
    *pNumDeleted = 0;

  if(GetBoolParameter(BP_DASHER_PAUSED) && (m_deGotoQueue.size() == 0))
     return false;

  myint iNewMin;
  myint iNewMax;

  if(m_deGotoQueue.size() == 0) {
    // works out next viewpoint
    Get_new_root_coords(miMousex, miMousey, iNewMin, iNewMax, iTime);
  
    if(GetBoolParameter(BP_OLD_STYLE_PUSH))
      OldPush(miMousex, miMousey);
  }
  else {
    iNewMin = m_deGotoQueue.front().iN1;
    iNewMax = m_deGotoQueue.front().iN2;
    m_deGotoQueue.pop_front();
  }

  m_dTotalNats += log((iNewMax - iNewMin) / static_cast<double>(m_Rootmax - m_Rootmin));

  // Now actually zoom to the new location
  NewGoTo(iNewMin, iNewMax, pAdded, pNumDeleted);

  return true;
}

void CDasherModel::OldPush(myint iMousex, myint iMousey) {
  // push node under mouse
  CDasherNode *pUnderMouse = Get_node_under_mouse(iMousex, iMousey);

  Push_Node(pUnderMouse);

  if(Framerate() > 4) {
    // push node under mouse but with x coord on RHS
    CDasherNode *pRight = Get_node_under_mouse(50, iMousey);
    Push_Node(pRight);
  }

  if(Framerate() > 8) {
    // push node under the crosshair
    CDasherNode *pUnderCross = Get_node_under_crosshair();
    Push_Node(pUnderCross);
  }

  int iRandom = RandomInt();

  if(Framerate() > 8) {
    // add some noise and push another node
    CDasherNode *pRight = Get_node_under_mouse(50, iMousey + iRandom % 500 - 250);
    Push_Node(pRight);
  }

  iRandom = RandomInt();

  if(Framerate() > 15) {
    // add some noise and push another node
    CDasherNode *pRight = Get_node_under_mouse(50, iMousey + iRandom % 500 - 250);
    Push_Node(pRight);
  }

  // only do this is Dasher is flying
  if(Framerate() > 30) {
    for(int i = 1; i < int (Framerate() - 30) / 3; i++) {

      int iRandom = RandomInt();
      
      if(Framerate() > 8) {
	      // add some noise and push another node
      	CDasherNode *pRight = Get_node_under_mouse(50, iMousey + iRandom % 500 - 250);
	      Push_Node(pRight);
      }
      
      iRandom = RandomInt();
      // push at a random node on the RHS
      CDasherNode *pRight = Get_node_under_mouse(50, iMousey + iRandom % 1000 - 500);
      Push_Node(pRight);

    }
  }
}

void CDasherModel::RecursiveOutput(CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded) {
  if(pNode->Parent() && (!pNode->Parent()->GetFlag(NF_SEEN)))
    RecursiveOutput(pNode->Parent(), pAdded);

  if(pNode->Parent())
    pNode->Parent()->m_pNodeManager->Leave(pNode->Parent());

  pNode->m_pNodeManager->Enter(pNode);

  pNode->SetFlag(NF_SEEN, true);
  pNode->m_pNodeManager->Output(pNode, pAdded, GetLongParameter(LP_NORMALIZATION));
}


// This is similar to Get_new_goto_coords, but doesn't actually change Rootmax and Rootmin.
// Instead it gives information for NewGoTo to make direct changes in the root coordinates.
// #define ZOOMDENOM (1<<10)
// #define STEPNUM   48 
// #define STEPDENOM 64
// double CDasherModel::Plan_new_goto_coords(int iRxnew, myint mousey, int *iSteps, myint *o1, myint *o2 , myint *n1, myint *n2)
// {
//   m_Stepnum = GetLongParameter(LP_ZOOMSTEPS);
//   int iRxnew_dup = iRxnew;
//   // note -- iRxnew is the zoom factor  in units of ZOOMDENOM
//   *o1 = m_Rootmin ;
//   *o2 = m_Rootmax ;
//   DASHER_ASSERT(iRxnew > 0);
//   if (iRxnew < ZOOMDENOM && m_Rootmax<(myint)GetLongParameter(LP_MAX_Y) && m_Rootmin>0 ) {
//     // refuse to zoom backwards if the entire root node is visible.
//     *iSteps = 0 ;
//     *n1 = m_Rootmin;
//     *n2 = m_Rootmax;
//   } 
//   else {
//     myint above=(mousey-*o1);
//     myint below=(*o2-mousey);

//     myint miNewrootzoom= GetLongParameter(LP_MAX_Y)/2 ;
//     myint newRootmax=miNewrootzoom+(below*iRxnew/ZOOMDENOM); // is there a risk of overflow in this multiply?
//     myint newRootmin=miNewrootzoom-(above*iRxnew/ZOOMDENOM);
    
//     *n1 = newRootmin;
//     *n2 = newRootmax;

//     *iSteps = 1;
    
//     // We might be moving at zoomfactor one vertically, in which case the below invention won't
//     // come up with more than one step.  Look for a mousey difference and use an iSteps concordant
//     // to that if it would be larger than the iSteps created by taking the log of the zoomfactor. 
//     int distance = mousey - ((myint)GetLongParameter(LP_MAX_Y)/2);

//     double s = (log(2.0) * 2 / log( (STEPDENOM*1.0)/(m_Stepnum*1.0)) ) / 4096;

//     double alpha = 2 * (2 * s);
//     int alternateSteps = int(alpha * abs(distance));

//     // Take log of iRxnew to base ( STEPDENOM / STEPNUM ):
//     if ( STEPDENOM > m_Stepnum && m_Stepnum > 0 ) { // check that the following loop will terminate.
//       //cout << "iRxnew is " << iRxnew << " and ZOOMDENOM is" << ZOOMDENOM << endl;
//       if ( iRxnew > ZOOMDENOM ) {
//         while ( iRxnew > ZOOMDENOM ) {
//           *iSteps += 1;
//           iRxnew = iRxnew * m_Stepnum / STEPDENOM;
//         }
//       } else {
//         while ( iRxnew < ZOOMDENOM ) {
//           *iSteps += 1;
//           iRxnew = iRxnew * STEPDENOM / m_Stepnum;
//         }
//       }
//     }

//     // Done taking log of iRxnew. 
//     if (alternateSteps > *iSteps) {
//       *iSteps = alternateSteps;
//     }
//    }

//   double iRxnew_ratio = (double) iRxnew_dup / ZOOMDENOM;
//   double iRxnew_log = log(iRxnew_ratio);
//   return iRxnew_log;
// }

void CDasherModel::NewGoTo(myint newRootmin, myint newRootmax, Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted) {
  // Find out the current node under the crosshair
  CDasherNode *old_under_cross=Get_node_under_crosshair();

  // Update the max and min of the root node to make iTargetMin and
  // iTargetMax the edges of the viewport.

  if(newRootmin + m_iTargetOffset > (myint)GetLongParameter(LP_MAX_Y) / 2 - 1)
    newRootmin = (myint)GetLongParameter(LP_MAX_Y) / 2 - 1 - m_iTargetOffset;

  if(newRootmax + m_iTargetOffset < (myint)GetLongParameter(LP_MAX_Y) / 2 + 1)
    newRootmax = (myint)GetLongParameter(LP_MAX_Y) / 2 + 1 - m_iTargetOffset;

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

    m_iTargetOffset = (m_iTargetOffset * 90) / 100;
  }
  else {
    // TODO - force a new root to be chosen, so that we get better
    // behaviour than just having Dasher stop at this point.
  }

  // Check whether new nodes need to be created
  CDasherNode* new_under_cross = Get_node_under_crosshair();
  Push_Node(new_under_cross);

  HandleOutput(new_under_cross, old_under_cross, pAdded, pNumDeleted);
}

void CDasherModel::HandleOutput(CDasherNode *pNewNode, CDasherNode *pOldNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted) {
  if(pNewNode != pOldNode)
    DeleteCharacters(pNewNode, pOldNode, pNumDeleted);
  
  if(pNewNode->GetFlag(NF_SEEN))
    return;

  RecursiveOutput(pNewNode, pAdded);
}

bool CDasherModel::DeleteCharacters(CDasherNode *newnode, CDasherNode *oldnode, int* pNumDeleted) {
  // DJW cant see how either of these can ever be NULL
  DASHER_ASSERT_VALIDPTR_RW(newnode);
  DASHER_ASSERT_VALIDPTR_RW(oldnode);

  if(newnode == NULL || oldnode == NULL)
    return false;

  // This deals with the trivial instance - we're reversing back over
  // text that we've seen already
  if(newnode->GetFlag(NF_SEEN)) {
    if(oldnode->Parent() == newnode) {
      oldnode->m_pNodeManager->Undo(oldnode);
      oldnode->Parent()->m_pNodeManager->Enter(oldnode->Parent());
      if (pNumDeleted != NULL)
        (*pNumDeleted)++;
      oldnode->SetFlag(NF_SEEN, false);
      return true;
    }
    if(DeleteCharacters(newnode, oldnode->Parent(), pNumDeleted) == true) {
      oldnode->m_pNodeManager->Undo(oldnode);
      oldnode->Parent()->m_pNodeManager->Enter(oldnode->Parent());
      if (pNumDeleted != NULL)
	(*pNumDeleted)++;
      oldnode->SetFlag(NF_SEEN, false);
      return true;
    }
  }
  else {
    // This one's more complicated - the user may have moved onto a new branch
    // Find the last seen node on the new branch
    CDasherNode *lastseen = newnode->Parent();

    while(lastseen != NULL && !(lastseen->GetFlag(NF_SEEN))) {
      lastseen = lastseen->Parent();
    };
    // Delete back to last seen node
    while(oldnode != lastseen) {

      oldnode->SetFlag(NF_SEEN, false);
      
      oldnode->m_pNodeManager->Undo(oldnode);

      if(oldnode->Parent())
	oldnode->Parent()->m_pNodeManager->Enter(oldnode->Parent());

      if (pNumDeleted != NULL)
	(*pNumDeleted)++;
      oldnode = oldnode->Parent();
      if(oldnode == NULL) {
        return false;
      }
    }
  }
  return false;
}

// void CDasherModel::LearnText(CLanguageModel::Context context, string *TheText, bool IsMore) {
//   m_pNodeCreationManager->LearnText(context, TheText, IsMore);
// }

// void CDasherModel::EnterText(CLanguageModel::Context context, string TheText) const {
//   m_pNodeCreationManager->EnterText(context, TheText);
// }

void CDasherModel::Push_Node(CDasherNode *pNode) {

  if(pNode->GetFlag(NF_ALLCHILDREN)) {
    DASHER_ASSERT(pNode->Children().size() > 0);
    // if there are children just give them a poke
    CDasherNode::ChildMap::iterator i;
    for(i = pNode->Children().begin(); i != pNode->Children().end(); i++)
      (*i)->SetFlag(NF_ALIVE, true);
    return;
  }

  // TODO: Do we really need to delete all of the children at this point?
  pNode->Delete_children();

  pNode->SetFlag(NF_ALIVE, true);

  pNode->m_pNodeManager->PopulateChildren(pNode);
  pNode->SetFlag(NF_ALLCHILDREN, true);
}

void CDasherModel::Recursive_Push_Node(CDasherNode *pNode, int iDepth) {

  if(pNode->Range() < 0.1 * GetLongParameter(LP_NORMALIZATION)) {
    return;
  }

  Push_Node(pNode);

  if(iDepth == 0)
    return;

  for(unsigned int i(0); i < pNode->ChildCount(); i++) {
    Recursive_Push_Node(pNode->Children()[i], iDepth - 1);
  }
}

bool CDasherModel::RenderToView(CDasherView *pView, bool bRedrawDisplay) {
  std::vector<CDasherNode *> vNodeList;
  std::vector<CDasherNode *> vDeleteList;

  bool bReturnValue;

  bReturnValue = pView->Render(m_Root, m_Rootmin + m_iTargetOffset, m_Rootmax + m_iTargetOffset, vNodeList, vDeleteList, bRedrawDisplay, m_bGameMode);

  if(!GetBoolParameter(BP_OLD_STYLE_PUSH)) {
  for(std::vector<CDasherNode *>::iterator it(vNodeList.begin()); it != vNodeList.end(); ++it)
    Push_Node(*it);
  }

  // TODO: Temporarily disabled 
//   for(std::vector<CDasherNode *>::iterator it(vDeleteList.begin()); it != vDeleteList.end(); ++it)
//     (*it)->Delete_children();

  return bReturnValue;
}

bool CDasherModel::CheckForNewRoot(CDasherView *pView) {

//  if(m_deGotoQueue.size() > 0)
 //   return false;

  CDasherNode *root(m_Root);
  CDasherNode::ChildMap & children = m_Root->Children();

  if(pView->IsNodeVisible(m_Rootmin,m_Rootmax)) {
    Reparent_root(root->Lbnd(), root->Hbnd());
    return(m_Root != root);
  }

  if(children.size() == 0)
    return false;

  int alive = 0;
  CDasherNode *theone = 0;
 
  // Find whether there is exactly one alive child; if more, we don't care.
  CDasherNode::ChildMap::iterator i;
  for(i = children.begin(); i != children.end(); i++) {
    if((*i)->GetFlag(NF_ALIVE)) {
      alive++;
      theone = *i;
      if(alive > 1)
        break;
    }
  }

  if(alive == 1) {
    // We must have zoomed sufficiently that only one child of the root node 
    // is still alive.  Let's make it the root.

    myint y1 = m_Rootmin;
    myint y2 = m_Rootmax;
    myint range = y2 - y1;
    
    myint newy1 = y1 + (range * theone->Lbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
    myint newy2 = y1 + (range * theone->Hbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
    if(!pView->IsNodeVisible(newy1, newy2)) {
        Make_root(theone);
        return false;
    }
  }
  
  return false;
}

double CDasherModel::CorrectionFactor(int dasherx, int dashery) {
  double dX = 1 - dasherx/2048.0;
  double dY = dashery/2048.0 - 1;

  double dR = sqrt(pow(dX, 2.0) + pow(dY, 2.0));

  if(fabs(dX) < 0.1)
    return dR * (1 + dX /2.0+ pow(dX, 2.0) / 3.0 + pow(dX, 3.0) / 4.0 + pow(dX, 4.0) / 5.0);
  else
    return -dR * log(1 - dX) / dX;
}

void CDasherModel::ScheduleZoom(int dasherx, int dashery) {

   // TODO: What is the following line for?
   if (dasherx < 2) { dasherx = 100; }

   double dCFactor(CorrectionFactor(dasherx, dashery));

   int iSteps = static_cast<int>(GetLongParameter(LP_ZOOMSTEPS) * dCFactor);
  // myint iRxnew = ((GetLongParameter(LP_OX)/2) * GetLongParameter(LP_OX)) / dasherx;
   myint n1, n2, iTarget1, iTarget2;
   //Plan_new_goto_coords(iRxnew, dashery, &iSteps, &o1,&o2,&n1,&n2);
 
   iTarget1 = dashery - dasherx;
   iTarget2 = dashery + dasherx;

   double dZ = 4096 / static_cast<double>(iTarget2 - iTarget1);

   n1 = static_cast<int>((m_Rootmin - iTarget1) * dZ);
   n2 = static_cast<int>((m_Rootmax - iTarget2) * dZ + 4096);
 
   m_deGotoQueue.clear();
   SGotoItem sNewItem;

   for(int s(1); s < iSteps; ++s) {
     // use simple linear interpolation. Really should do logarithmic interpolation, but
     // this will probably look fine.
    
     sNewItem.iN1 = (s * n1 + (iSteps-s) * m_Rootmin) / iSteps;
     sNewItem.iN2 = (s * n2 + (iSteps-s) * m_Rootmax) / iSteps;
     sNewItem.iStyle = 1;

     m_deGotoQueue.push_back(sNewItem);
   } 

   sNewItem.iN1 = n1;
   sNewItem.iN2 = n2;
   sNewItem.iStyle = 2;
 
   m_deGotoQueue.push_back(sNewItem);
}

void CDasherModel::Offset(int iOffset) {
  m_Rootmin += iOffset;
  m_Rootmax += iOffset;

  m_iTargetOffset -= iOffset;
} 

void CDasherModel::MatchTarget(bool bReverse) {
  // TODO: Does anything need to happen wrt bReverse here?

  m_Rootmin += m_iTargetOffset;
  m_Rootmax += m_iTargetOffset;

  m_iTargetOffset = 0;
}

void CDasherModel::LimitRoot(int iMaxWidth) {
  m_Rootmin = GetLongParameter(LP_MAX_Y) / 2 - iMaxWidth / 2;
  m_Rootmax = GetLongParameter(LP_MAX_Y) / 2 + iMaxWidth / 2;
}

void CDasherModel::SetOffset(int iLocation, CDasherView *pView) {
  if(iLocation == m_iOffset)
    return; // We're already there
  
  // TODO: Special cases, ie this can be done without rebuilding the
  // model

  m_iOffset = iLocation;

  // Now actually rebuild the model
  InitialiseAtOffset(iLocation, pView);
  
}
