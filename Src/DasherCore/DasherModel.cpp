// DasherModel.h
//
// Copyright (c) 2001-2005 David Ward

#include "../Common/Common.h"
#include <iostream>
#include "../Common/Random.h"
#include "DasherModel.h"
#include "DasherView.h"

using namespace Dasher;
using namespace std;

#include "Event.h"
#include "DasherInterfaceBase.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"
#include "LanguageModelling/DictLanguageModel.h"
#include "LanguageModelling/MixtureLanguageModel.h"

#ifdef JAPANESE
#include "LanguageModelling/JapaneseLanguageModel.h"
#endif

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

CDasherModel::CDasherModel(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pDashIface)
:CDasherComponent(pEventHandler, pSettingsStore), m_pDasherInterface(pDashIface), m_Root(0), total_nats(0.0), 
m_pLanguageModel(NULL), m_pcAlphabet(NULL), m_pGameMode(NULL), m_Rootmin(0), m_Rootmax(0), m_Rootmin_min(0),
m_Rootmax_max(0), m_dAddProb(0.0), m_dMaxRate(0.0) {

  // Set max bitrate in the FrameRate class
  m_dMaxRate = GetLongParameter(LP_MAX_BITRATE) / 100.0;
  m_fr.SetMaxBitrate(m_dMaxRate);

  // Convert the full alphabet to a symbolic representation for use in the language model
  m_pcAlphabet = m_pDasherInterface->GetAlphabet();
  CSymbolAlphabet alphabet(m_pcAlphabet->GetNumberTextSymbols());
  alphabet.SetSpaceSymbol(m_pcAlphabet->GetSpaceSymbol());      // FIXME - is this right, or do we have to do some kind of translation?
  alphabet.SetAlphabetPointer(m_pcAlphabet);    // Horrible hack, but ignore for now.

  // Create an appropriate language model;

  // FIXME - return to using enum here


  switch (GetLongParameter(LP_LANGUAGE_MODEL_ID)) {
  case 0:
    m_pLanguageModel = new CPPMLanguageModel(m_pEventHandler, m_pSettingsStore, alphabet);
    break;
  case 2:
    m_pLanguageModel = new CWordLanguageModel(m_pEventHandler, m_pSettingsStore, alphabet);
    break;
  case 3:
    m_pLanguageModel = new CMixtureLanguageModel(m_pEventHandler, m_pSettingsStore, alphabet);
    break;  
#ifdef JAPANESE
  case 4:
    m_pLanguageModel = new CJapaneseLanguageModel(m_pEventHandler, m_pSettingsStore, alphabet);
    break;
#endif
  default:
    // If there is a bogus value for the language model ID, we'll default
    // to our trusty old PPM language model.
    m_pLanguageModel = new CPPMLanguageModel(m_pEventHandler, m_pSettingsStore, alphabet);    
    break;
  }

  LearnContext = m_pLanguageModel->CreateEmptyContext();

  // various settings
  int iShift = 12;
  // m_DasherY = 1 << iShift;
  //  m_DasherOY = m_DasherY / 2;
  //  m_DasherOX = m_DasherY / 2;
  m_dAddProb = 0.003;

  //  m_Active = CRange(0, GetLongParameter(LP_MAX_Y));

  int iNormalization = GetLongParameter(LP_NORMALIZATION);
  m_Rootmin_min = int64_min / iNormalization / 2;
  m_Rootmax_max = int64_max / iNormalization / 2;

  // Initialize Game Mode object
  m_pGameMode = new CDasherGameMode(pEventHandler, pSettingsStore, m_pDasherInterface, this);

  m_pAlphabetManagerFactory = new CAlphabetManagerFactory(this, m_pLanguageModel);
  m_pControlManagerFactory = new CControlManagerFactory(this, m_pLanguageModel);

  m_bContextSensitive = true;
}

CDasherModel::~CDasherModel() {

  if(oldroots.size() > 0) {
    delete oldroots[0];
    oldroots.clear();
    // At this point we have also deleted the root - so better NULL pointer
    m_Root = NULL;
  }

  delete m_Root;

  delete m_pAlphabetManagerFactory;
  delete m_pControlManagerFactory;

  if (m_pGameMode != NULL) {
    delete m_pGameMode;
    m_pGameMode = NULL;
  }

  m_pLanguageModel->ReleaseContext(LearnContext);
  delete m_pLanguageModel;

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
    default:
      break;
    }
  }

}

void CDasherModel::Make_root(CDasherNode *whichchild)
// find a new root node 
{

  symbol t = m_Root->Symbol();
  if(t < m_pDasherInterface->GetAlphabet()->GetNumberTextSymbols()) {
    // Only learn if we have adaptive behaviour enabled

    // This was m_bAdaptive, which should be a setting for all dasher?
    if(true)
      // SYM0
      m_pLanguageModel->LearnSymbol(LearnContext, t);
  }

  m_Root->DeleteNephews(whichchild);

  oldroots.push_back(m_Root);

  m_Root = whichchild;

  while(oldroots.size() > 10) {
    oldroots[0]->OrphanChild(oldroots[1]);
    delete oldroots[0];
    oldroots.pop_front();
  }

  myint range = m_Rootmax - m_Rootmin;
  m_Rootmax = m_Rootmin + (range * m_Root->Hbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
  m_Rootmin = m_Rootmin + (range * m_Root->Lbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
}

void CDasherModel::Reparent_root(int lower, int upper) {

  /* Change the root node to the parent of the existing node
     We need to recalculate the coordinates for the "new" root as the 
     user may have moved around within the current root */

  if(m_Root->Symbol() == 0)
    return; // Don't try to reparent the root symbol


  CDasherNode *pNewRoot;

  if(oldroots.size() == 0) {
    CDasherNode *pCurrentNode(Get_node_under_crosshair());
    int iGenerations(0);
    
    while(pCurrentNode != m_Root) {
      ++iGenerations;
      pCurrentNode = pCurrentNode->Parent();
    }
    
    pNewRoot = m_Root->m_pNodeManager->RebuildParent(m_Root, iGenerations);

    lower = m_Root->Lbnd();
    upper = m_Root->Hbnd();

  }
  else {
    pNewRoot = oldroots.back();
    oldroots.pop_back();
  }

  // Return if there's no existing parent and no way of recreating one

  if(!pNewRoot) { 
    return;
  }

  /* Determine how zoomed in we are */

  myint iRootWidth = m_Rootmax - m_Rootmin;
  myint iWidth = upper - lower;
//  double scalefactor=(m_Rootmax-m_Rootmin)/static_cast<double>(upper-lower);

  m_Root = pNewRoot;

  m_Rootmax = m_Rootmax + (myint((GetLongParameter(LP_NORMALIZATION) - upper)) * iRootWidth / iWidth);
  m_Rootmin = m_Rootmin - (myint(lower) * iRootWidth / iWidth);
}

/////////////////////////////////////////////////////////////////////////////

CDasherNode *CDasherModel::Get_node_under_crosshair() {
  return m_Root->Get_node_under(GetLongParameter(LP_NORMALIZATION), m_Rootmin, m_Rootmax, GetLongParameter(LP_OX), GetLongParameter(LP_OY));
}

/////////////////////////////////////////////////////////////////////////////

CDasherNode *CDasherModel::Get_node_under_mouse(myint Mousex, myint Mousey) {
  return m_Root->Get_node_under(GetLongParameter(LP_NORMALIZATION), m_Rootmin, m_Rootmax, Mousex, Mousey);
}

/////////////////////////////////////////////////////////////////////////////

void CDasherModel::Get_string_under_mouse(const myint Mousex, const myint Mousey, vector <symbol >&str) {
  m_Root->Get_string_under(GetLongParameter(LP_NORMALIZATION), m_Rootmin, m_Rootmax, Mousex, Mousey, str);
  return;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherModel::Start() {

  // FIXME - re-evaluate this function and SetContext...

//              m_pEditbox->get_new_context(ContextString,5);

  std::string strNewContext("");

  SetContext(strNewContext);    // FIXME - REALLY REALLY broken!

  CEditContextEvent oEvent(5);

  InsertEvent(&oEvent);

  // FIXME - what if we don't get a reply?

//      m_pLanguageModel->ReleaseNodeContext(therootcontext);
//      ppmmodel->dump();
//      dump();

}

void CDasherModel::SetContext(std::string &sNewContext) {
  

  if(oldroots.size() > 0) {
    delete oldroots[0];
    oldroots.clear();
    // At this point we have also deleted the root - so better NULL pointer
    m_Root = NULL;
  }
  delete m_Root;


  CLanguageModel::Context therootcontext = m_pLanguageModel->CreateEmptyContext();

  if(sNewContext.size() == 0) {
    m_Root = m_pAlphabetManagerFactory->GetRoot(NULL, 0,GetLongParameter(LP_NORMALIZATION), NULL);
    
    EnterText(therootcontext, ". ");  
  }
  else {
    std::vector<symbol> vSymbols;
    m_pLanguageModel->SymbolAlphabet().GetAlphabetPointer()->GetSymbols(&vSymbols, &sNewContext, false);
    
    int iRootSymbol(vSymbols[vSymbols.size()-1]);
    
    m_Root = m_pAlphabetManagerFactory->GetRoot(NULL, 0,GetLongParameter(LP_NORMALIZATION), &iRootSymbol);
    
    EnterText(therootcontext, sNewContext);  
  }

  m_pLanguageModel->ReleaseContext(LearnContext);
  LearnContext = m_pLanguageModel->CloneContext(therootcontext);

  m_Root->SetContext(therootcontext);   // node takes control of the context
  Recursive_Push_Node(m_Root, 0);

  double dFraction( 1 - (1 - m_Root->MostProbableChild() / static_cast<double>(GetLongParameter(LP_NORMALIZATION))) / 2.0 );

  int iWidth( GetLongParameter(LP_MAX_Y) / (2.0*dFraction) );

  m_Rootmin = GetLongParameter(LP_MAX_Y) / 2 - iWidth / 2;
  m_Rootmax = GetLongParameter(LP_MAX_Y) / 2 + iWidth / 2;
}

/////////////////////////////////////////////////////////////////////////////

///
/// CDasherModel::Get_new_root_coords( myint Mousex,myint Mousey )
/// 
/// Calculate the new co-ordinates for the root node after a single
/// update step. For further information, see Doc/geometry.tex.
/// 
/// \param Mousex x mouse co-ordinate measured right to left.
/// \param Mousey y mouse co-ordinate measured top to bottom.
/// \return Returns the number of nats entered
///

double CDasherModel::Get_new_root_coords(myint Mousex, myint Mousey) {
  // Comments refer to the code immedialtely before them

  if(Mousex <= 0) {
    Mousex = 1;
  }

  // Avoid Mousex=0, as this corresponds to infinite zoom

  // If Mousex is too large we risk overflow errors, so make limit it
  // (this is a somewhat empirical limit - at some point we should
  // probably do it a little more scientifically)

  if(Mousex > 60000000)
    Mousex = 60000000;

  // Cache some results so we don't do a huge number of parameter lookups

  myint iMaxY(GetLongParameter(LP_MAX_Y));
  myint iOX(GetLongParameter(LP_OX));
  myint iOY(GetLongParameter(LP_OY));

  int iTargetMin(Mousey - ((myint)iMaxY * Mousex) / (2 * (myint)iOX));
  int iTargetMax(Mousey + ((myint)iMaxY * Mousex) / (2 * (myint)iOY));

  // Calculate what the extremes of the viewport will be when the
  // point under the cursor is at the cross-hair. This is where 
  // we want to be in iSteps updates

  //  std::cout << iTargetMin << " " << iTargetMax << std::endl;

  int iSteps = m_fr.Steps();

  DASHER_ASSERT(iSteps > 0);

  // iSteps is the number of update steps we need to get the point
  // under the cursor over to the cross hair. Calculated in order to
  // keep a constant bit-rate.

  int iNewTargetMin;
  int iNewTargetMax;

  iNewTargetMin = (iTargetMin * iMaxY / (iMaxY + (iSteps - 1) * (iTargetMax - iTargetMin)));

  iNewTargetMax = ((iTargetMax * iSteps - iTargetMin * (iSteps - 1)) * iMaxY) / (iMaxY + (iSteps - 1) * (iTargetMax - iTargetMin));

  iTargetMin = iNewTargetMin;
  iTargetMax = iNewTargetMax;

  // Calculate the new values of iTargetMin and iTargetMax required to
  // perform a single update step. Note that the slightly awkward
  // expressions are in order to reproduce the behaviour of the old
  // algorithm

  myint iMinSize(m_fr.MinSize(iMaxY));

  // Calculate the minimum size of the viewport corresponding to the
  // maximum zoom.

  if((iTargetMax - iTargetMin) < iMinSize) {

    iNewTargetMin = iTargetMin * (iMaxY - iMinSize) / (iMaxY - (iTargetMax - iTargetMin));
    iNewTargetMax = iNewTargetMin + iMinSize;

    iTargetMin = iNewTargetMin;
    iTargetMax = iNewTargetMax;

  }

  // Check we're not going faster than the speed slider setting
  // allows, and adjust if necessary. Note that if we re-size the
  // target we need to be careful about where we centre the new range
  // (hence the slightly complicated expression above)

  DoZoom(iTargetMin, iTargetMax);

  // Actually do the zooming

  return -1.0 * log((iTargetMax - iTargetMin) / static_cast < double >(iMaxY));

  // Return value is the zoom factor so we can keep track of bitrate.
}

/// Zoom the display so that [iTargetMin,iTargetMax] (in current
/// Dasher co-ordinates) are the new extremes of the viewport.

void CDasherModel::DoZoom(myint iTargetMin, myint iTargetMax) {

  // std::cout << "iTargetMin: " << iTargetMin << " iTargetMax: " << iTargetMax << std::endl;

  myint newRootmin(((m_Rootmin - iTargetMin) * (myint)GetLongParameter(LP_MAX_Y)) / (iTargetMax - iTargetMin));
  myint newRootmax(((m_Rootmax - iTargetMax) * (myint)GetLongParameter(LP_MAX_Y)) / (iTargetMax - iTargetMin) + (myint)GetLongParameter(LP_MAX_Y));

  // Update the max and min of the root node to make iTargetMin and iTargetMax the edges of the viewport.

  if(newRootmin > (myint)GetLongParameter(LP_MAX_Y) / 2 - 1)
    newRootmin = (myint)GetLongParameter(LP_MAX_Y) / 2 - 1;

  if(newRootmax < (myint)GetLongParameter(LP_MAX_Y) / 2 + 1)
    newRootmax = (myint)GetLongParameter(LP_MAX_Y) / 2 + 1;

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
  }
  else {
    // TODO - force a new root to be chosen, so that we get better
    // behaviour than just having Dasher stop at this point.
  }
}

void CDasherModel::Get_new_goto_coords(double zoomfactor, myint MouseY)
                                       // this was mousex.
{
  // First, we need to work out how far we need to zoom in
  //float zoomfactor=(m_DasherOX-MouseX)/(m_DasherOX*1.0);

  // Then zoom in appropriately
  m_Rootmax = m_Rootmax + myint(zoomfactor * (m_Rootmax - (myint)GetLongParameter(LP_MAX_Y) / 2));
  m_Rootmin = m_Rootmin + myint(zoomfactor * (m_Rootmin - (myint)GetLongParameter(LP_MAX_Y) / 2));

  // Afterwards, we need to take care of the vertical offset.
  myint up = ((myint)GetLongParameter(LP_MAX_Y) / 2) - MouseY;
  m_Rootmax = m_Rootmax + up;
  m_Rootmin = m_Rootmin + up;
}

myint CDasherModel::PlotGoTo(myint MouseX, myint MouseY) {
  // First, we need to work out how far we need to zoom in
  double zoomfactor = ((myint)GetLongParameter(LP_OX) - MouseX) / ((myint)GetLongParameter(LP_OX) * 1.0);
  zoomfactor = pow(0.5, zoomfactor);

  myint height = int ((myint)GetLongParameter(LP_MAX_Y) * zoomfactor / 2);

  return height;
}

void CDasherModel::Tap_on_display(myint miMousex,
                                  myint miMousey, 
                                  unsigned long Time, 
                                  Dasher::VECTOR_SYMBOL_PROB* pAdded, 
                                  int* pNumDeleted) 
        // work out the next viewpoint, opens some new nodes
{

  //  std::cout << "miMouseX: " << miMousex << " miMouseY: " << miMousey << std::endl;

  if(!GetBoolParameter(BP_DASHER_PAUSED)) {

  // Clear out parameters that might get passed in to track user activity
  if (pAdded != NULL)
    pAdded->clear();
  if (pNumDeleted != NULL)
    *pNumDeleted = 0;

  // Find out the current node under the crosshair
  CDasherNode *old_under_cross = Get_node_under_crosshair();

  // works out next viewpoint
  total_nats += Get_new_root_coords(miMousex, miMousey);
  
 

  // opens up new nodes

  if(GetBoolParameter(BP_OLD_STYLE_PUSH))
  {

  // push node under mouse
  CDasherNode *pUnderMouse = Get_node_under_mouse(miMousex, miMousey);

  Push_Node(pUnderMouse);

  if(Framerate() > 4) {
    // push node under mouse but with x coord on RHS
    CDasherNode *pRight = Get_node_under_mouse(50, miMousey);
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
    CDasherNode *pRight = Get_node_under_mouse(50, miMousey + iRandom % 500 - 250);
    Push_Node(pRight);
  }

  iRandom = RandomInt();

  if(Framerate() > 15) {
    // add some noise and push another node
    CDasherNode *pRight = Get_node_under_mouse(50, miMousey + iRandom % 500 - 250);
    Push_Node(pRight);
  }

  // only do this is Dasher is flying
  if(Framerate() > 30) {
    for(int i = 1; i < int (Framerate() - 30) / 3; i++) {

      iRandom = RandomInt();
      // push at a random node on the RHS
      CDasherNode *pRight = Get_node_under_mouse(50, miMousey + iRandom % 1000 - 500);
      Push_Node(pRight);

    }
  }
  }
//      Update(m_Root,under_mouse,0);

  CDasherNode *new_under_cross = Get_node_under_crosshair();

  // FIXME - Reimplement

  if(new_under_cross != old_under_cross) {
    DeleteCharacters(new_under_cross, old_under_cross, pNumDeleted);
  }

  if(new_under_cross->isSeen() == true) {
//     if(new_under_cross->ControlChild() != true) {
//       SetBitrate(m_dMaxRate);
//     }
     return;
   }




  // FIXME - Need to recurse up possibly unseen parents


  RecursiveOutput(new_under_cross, pAdded);

  // FIXME - Reimplement

//   if(new_under_cross->ControlChild() == true) {
//     //            m_pEditbox->outputcontrol(new_under_cross->GetControlTree()->pointer,new_under_cross->GetControlTree()->data,new_under_cross->GetControlTree()->type);
//     OutputCharacters(new_under_cross);
//     SetBitrate(m_dMaxRate / 3);
//   }
//   else {
//     OutputCharacters(new_under_cross);
//     SetBitrate(m_dMaxRate);
//   }
  //      m_Root->Recursive_Push_Node(0);

  }

  if(m_deGotoQueue.size() > 0) {
    NewGoTo(m_deGotoQueue.front().iN1, m_deGotoQueue.front().iN2, m_deGotoQueue.front().iStyle);
    m_deGotoQueue.pop_front();
  }

}


void CDasherModel::RecursiveOutput(CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded) {
  if(pNode->Parent() && (!pNode->Parent()->isSeen()))
    RecursiveOutput(pNode->Parent(), pAdded);

  if(pNode->Parent())
    pNode->Parent()->m_pNodeManager->Leave(pNode->Parent());

  pNode->m_pNodeManager->Enter(pNode);

  pNode->Seen(true);
  pNode->m_pNodeManager->Output(pNode, pAdded, GetLongParameter(LP_NORMALIZATION));
}


void CDasherModel::GoTo(double zoomfactor, myint miMousey)
        // work out the next viewpoint, opens some new nodes
{
  // Find out the current node under the crosshair
  CDasherNode *old_under_cross = Get_node_under_crosshair();

  // works out next viewpoint
  Get_new_goto_coords(zoomfactor, miMousey);

  // push node under crosshair

  CDasherNode *new_under_cross = Get_node_under_crosshair();

  Push_Node(new_under_cross);

  // push node under goto point

  // We don't have a mousex, so "emulating" one.
  CDasherNode *node_under_goto = Get_node_under_mouse(50, miMousey);

  Push_Node(node_under_goto);

//      Update(m_Root,new_under_cross,0);

  if(new_under_cross != old_under_cross) {
    DeleteCharacters(new_under_cross, old_under_cross);
  }

  if(new_under_cross->isSeen() == true)
    return;

  new_under_cross->Seen(true);

  OutputCharacters(new_under_cross);
}

// This is similar to Get_new_goto_coords, but doesn't actually change Rootmax and Rootmin.
// Instead it gives information for NewGoTo to make direct changes in the root coordinates.
#define ZOOMDENOM (1<<10)
#define STEPNUM   48 
#define STEPDENOM 64
double CDasherModel::Plan_new_goto_coords(int iRxnew, myint mousey, int *iSteps, myint *o1, myint *o2 , myint *n1, myint *n2)
{
  m_Stepnum = GetLongParameter(LP_ZOOMSTEPS);
  int iRxnew_dup = iRxnew;
  // note -- iRxnew is the zoom factor  in units of ZOOMDENOM
  *o1 = m_Rootmin ;
  *o2 = m_Rootmax ;
  DASHER_ASSERT(iRxnew > 0);
  if (iRxnew < ZOOMDENOM && m_Rootmax<(myint)GetLongParameter(LP_MAX_Y) && m_Rootmin>0 ) {
    // refuse to zoom backwards if the entire root node is visible.
    cout << "Refusing to zoom backwards." << endl;
    *iSteps = 0 ;
    *n1 = m_Rootmin;
    *n2 = m_Rootmax;
  } 
  else {
    myint above=(mousey-*o1);
    myint below=(*o2-mousey);

    myint miNewrootzoom= GetLongParameter(LP_MAX_Y)/2 ;
    myint newRootmax=miNewrootzoom+(below*iRxnew/ZOOMDENOM); // is there a risk of overflow in this multiply?
    myint newRootmin=miNewrootzoom-(above*iRxnew/ZOOMDENOM);
    
    *n1 = newRootmin;
    *n2 = newRootmax;

    *iSteps = 1;
    
    // We might be moving at zoomfactor one vertically, in which case the below invention won't
    // come up with more than one step.  Look for a mousey difference and use an iSteps concordant
    // to that if it would be larger than the iSteps created by taking the log of the zoomfactor. 
    int distance = mousey - ((myint)GetLongParameter(LP_MAX_Y)/2);

    double s = (log(2.0) * 2 / log( (STEPDENOM*1.0)/(m_Stepnum*1.0)) ) / 4096;

    double alpha = 2 * (2 * s);
    int alternateSteps = int(alpha * distance);
    // Take log of iRxnew to base ( STEPDENOM / STEPNUM ):
    if ( STEPDENOM > m_Stepnum && m_Stepnum > 0 ) { // check that the following loop will terminate.
      //cout << "iRxnew is " << iRxnew << " and ZOOMDENOM is" << ZOOMDENOM << endl;
      if ( iRxnew > ZOOMDENOM ) {
        while ( iRxnew > ZOOMDENOM ) {
          *iSteps += 1;
          iRxnew = iRxnew * m_Stepnum / STEPDENOM;
        }
      } else {
        while ( iRxnew < ZOOMDENOM ) {
          *iSteps += 1;
          iRxnew = iRxnew * STEPDENOM / m_Stepnum;
        }
      }
    }
    // Done taking log of iRxnew. 
    if (alternateSteps > *iSteps) {
      *iSteps = alternateSteps;
    }
   }

  double iRxnew_ratio = (double) iRxnew_dup / ZOOMDENOM;
  double iRxnew_log = log(iRxnew_ratio);
  return iRxnew_log;
}

void CDasherModel::NewGoTo(myint n1 , myint n2 , int style) {
  // Find out the current node under the crosshair
  CDasherNode *old_under_cross=Get_node_under_crosshair();

  // establish next viewpoint
  m_Rootmin = n1 ;
  m_Rootmax = n2 ;
   
  // push node under crosshair
  CDasherNode* new_under_cross = Get_node_under_crosshair();
  Push_Node(new_under_cross);
  
  if ( style >= 2 ) { // "2" means FINAL. "1" means STEP
	// Ugly hack  --
    // Push at all locations on the right hand side.
    int Hack=60 ; // was 20
    int hack ;
    int y1 , y2 , hackystep = 34 , mid=GetLongParameter(LP_MAX_Y)/2 , top=GetLongParameter(LP_MAX_Y)*1/20 , bot = GetLongParameter(LP_MAX_Y)*19/20 ;
    // y1 sweeps the top half of the screen; y2 sweeps the bottom half
    // choice of `random' hackystep intended to give fairly uniform coverage. 
    for ( y1 = mid , y2 = mid+hackystep/2 , hack = 1 ; hack <= Hack ; hack ++ ) {
      // We don't have a mousex, so "emulating" one.
      CDasherNode* node_under_goto = Get_node_under_mouse(50, y1);
      Push_Node(node_under_goto);
      node_under_goto = Get_node_under_mouse(50, y2);
      Push_Node(node_under_goto);
    
      y1 -= hackystep;
      if ( y1 < top ) { y1 += mid; }

      y2 += hackystep;
      if ( y1 > bot ) { y2 -= mid; }
    }
    // end hack
  }
  
  //Update(m_Root,new_under_cross,0);

  if (new_under_cross!=old_under_cross) {
    DeleteCharacters(new_under_cross,old_under_cross);
  }

  if (new_under_cross->isSeen()==true)
    return;

  new_under_cross->Seen(true);

  OutputCharacters(new_under_cross);
}



void CDasherModel::OutputCharacters(CDasherNode *node) {
  if(node->Parent() != NULL && node->Parent()->isSeen() != true) {
    node->Parent()->Seen(true);
    OutputCharacters(node->Parent());
  }
  symbol t = node->Symbol();
  if(t)                         // SYM0
  {
    Dasher::CEditEvent oEvent(1, GetAlphabet().GetText(t));
    InsertEvent(&oEvent);
  }
  else if(node->ControlChild() == true) {

    // FIXME - control events currently not implemented
//        m_pEditbox->outputcontrol(node->GetControlTree()->pointer,node->GetControlTree()->data,node->GetControlTree()->type);
  }
}

bool CDasherModel::DeleteCharacters(CDasherNode *newnode, CDasherNode *oldnode, int* pNumDeleted) {

  // DJW cant see how either of these can ever be NULL
  DASHER_ASSERT_VALIDPTR_RW(newnode);
  DASHER_ASSERT_VALIDPTR_RW(oldnode);

  if(newnode == NULL || oldnode == NULL)
    return false;

  // This deals with the trivial instance - we're reversing back over
  // text that we've seen already
  if(newnode->isSeen() == true) {
    if(oldnode->Parent() == newnode) {
      oldnode->m_pNodeManager->Undo(oldnode);
      oldnode->Parent()->m_pNodeManager->Enter(oldnode->Parent());
      if (pNumDeleted != NULL)
        (*pNumDeleted)++;
      oldnode->Seen(false);
      return true;
    }
    if(DeleteCharacters(newnode, oldnode->Parent(), pNumDeleted) == true) {
      oldnode->m_pNodeManager->Undo(oldnode);
      oldnode->Parent()->m_pNodeManager->Enter(oldnode->Parent());
      if (pNumDeleted != NULL)
	(*pNumDeleted)++;
      oldnode->Seen(false);
      return true;
    }
  }
  else {
    // This one's more complicated - the user may have moved onto a new branch
    // Find the last seen node on the new branch
    CDasherNode *lastseen = newnode->Parent();

    while(lastseen != NULL && lastseen->isSeen() == false) {
      lastseen = lastseen->Parent();
    };
    // Delete back to last seen node
    while(oldnode != lastseen) {

      oldnode->Seen(false);
      if(oldnode->ControlChild() == true || oldnode->Symbol() == GetControlSymbol() || oldnode->Symbol() == 0) {
        oldnode = oldnode->Parent();
        continue;
      }

      oldnode->m_pNodeManager->Undo(oldnode);
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

/////////////////////////////////////////////////////////////////////////////

// Diagnostic trace
void CDasherModel::Trace() const {
  // OutputDebugString(TEXT(" ptr   symbol   context Next  Child    pushme pushed cscheme   lbnd  hbnd \n"));
  m_Root->Trace();
}

///////////////////////////////////////////////////////////////////

void CDasherModel::GetProbs(CLanguageModel::Context context, vector <symbol >&NewSymbols, vector <unsigned int >&Probs, int iNorm) const {
  // Total number of symbols
  int iSymbols = m_pcAlphabet->GetNumberSymbols();      // note that this includes the control node and the root node

  // Number of text symbols, for which the language model gives the distribution
  // int iTextSymbols = m_pcAlphabet->GetNumberTextSymbols();
  
  NewSymbols.resize(iSymbols);
//      Groups.resize(iSymbols);
  for(int i = 0; i < iSymbols; i++) {
    NewSymbols[i] = i;          // This will be replaced by something that works out valid nodes for this context
    //      Groups[i]=m_pcAlphabet->get_group(i);
  }

  // TODO - sort out size of control node - for the timebeing I'll fix the control node at 5%

  int uniform_add;
  int nonuniform_norm;
  int control_space;
  int uniform = GetLongParameter(LP_UNIFORM);

  if(!GetBoolParameter(BP_CONTROL_MODE)) {
    control_space = 0;
    uniform_add = ((iNorm * uniform) / 1000) / (iSymbols - 2);  // Subtract 2 from no symbols to lose control/root nodes
    nonuniform_norm = iNorm - (iSymbols - 2) * uniform_add;
  }
  else {
    control_space = int (iNorm * 0.05);
    uniform_add = (((iNorm - control_space) * uniform / 1000) / (iSymbols - 2));        // Subtract 2 from no symbols to lose control/root nodes
    nonuniform_norm = iNorm - control_space - (iSymbols - 2) * uniform_add;
  }

  m_pLanguageModel->GetProbs(context, Probs, nonuniform_norm);

#if _DEBUG
  int iTotal = 0;
  for(int k = 0; k < Probs.size(); ++k)
    iTotal += Probs[k];
  DASHER_ASSERT(iTotal == nonuniform_norm);
#endif

  //  Probs.insert(Probs.begin(), 0);

  for(unsigned int k(1); k < Probs.size(); ++k)
    Probs[k] += uniform_add;

  Probs.push_back(control_space);

#if _DEBUG
  iTotal = 0;
  for(int k = 0; k < Probs.size(); ++k)
    iTotal += Probs[k];
//      DASHER_ASSERT(iTotal == iNorm);
#endif

}

void CDasherModel::LearnText(CLanguageModel::Context context, string *TheText, bool IsMore) {
  vector < symbol > Symbols;

  m_pcAlphabet->GetSymbols(&Symbols, TheText, IsMore);

  for(unsigned int i = 0; i < Symbols.size(); i++)
    m_pLanguageModel->LearnSymbol(context, Symbols[i]); // FIXME - conversion to symbol alphabet
}

void CDasherModel::EnterText(CLanguageModel::Context context, string TheText) const {
  vector < symbol > Symbols;
  m_pcAlphabet->GetSymbols(&Symbols, &TheText, false);
  for(unsigned int i = 0; i < Symbols.size(); i++)
    m_pLanguageModel->EnterSymbol(context, Symbols[i]); // FIXME - conversion to symbol alphabet
}

CDasherModel::CTrainer::CTrainer(CDasherModel &DasherModel)
:m_DasherModel(DasherModel) {
  m_Context = m_DasherModel.m_pLanguageModel->CreateEmptyContext();
}

void CDasherModel::CTrainer::Train(const std::vector <symbol >&vSymbols) {
  for(unsigned int i(0); i < vSymbols.size(); i++)
    m_DasherModel.m_pLanguageModel->LearnSymbol(m_Context, vSymbols[i]);
}

CDasherModel::CTrainer::~CTrainer() {
  m_DasherModel.m_pLanguageModel->ReleaseContext(m_Context);

}

CDasherModel::CTrainer * CDasherModel::GetTrainer() {
  return new CDasherModel::CTrainer(*this);
}

void CDasherModel::Push_Node(CDasherNode *pNode) {

  if(pNode->HasAllChildren()) {
    DASHER_ASSERT(pNode->Children().size() > 0);
    // if there are children just give them a poke
    CDasherNode::ChildMap::iterator i;
    for(i = pNode->Children().begin(); i != pNode->Children().end(); i++)
      (*i)->Alive(true);
    return;
  }

  pNode->Delete_children();

  // This ASSERT seems to routinely fail
  //DASHER_ASSERT(pNode->Symbol()!=0);

  // if we haven't got a context then derive it

  if(!pNode->Context()) {
    CLanguageModel::Context cont;
    // sym0
    if(pNode->Symbol() < m_pcAlphabet->GetNumberTextSymbols() && pNode->Symbol() > 0) {
      CDasherNode *pParent = pNode->Parent();
      DASHER_ASSERT(pParent != NULL);
      // Normal symbol - derive context from parent
      cont = m_pLanguageModel->CloneContext(pParent->Context());
      m_pLanguageModel->EnterSymbol(cont, pNode->Symbol());
    } else {
      // For new "root" nodes (such as under control mode), we want to 
      // mimic the root context
      cont = CreateEmptyContext();
      //      EnterText(cont, "");
    }
    pNode->SetContext(cont);

  }

  pNode->Alive(true);
  
  pNode->m_pNodeManager->PopulateChildren(pNode);
  pNode->SetHasAllChildren(true);
}

void CDasherModel::Recursive_Push_Node(CDasherNode *pNode, int iDepth) {

  if(pNode->Range() < 0.1 * GetLongParameter(LP_NORMALIZATION)) {
    return;
  }

  if(pNode->Symbol() == GetControlSymbol()) {
    return;
  }

  Push_Node(pNode);

  if(iDepth == 0)
    return;

  for(unsigned int i(0); i < pNode->ChildCount(); i++) {
    Recursive_Push_Node(pNode->Children()[i], iDepth - 1);
  }
}

// FIXME - annoying code duplication below

void CDasherModel::RenderToView(CDasherView *pView) {
  std::vector<CDasherNode *> vNodeList;
  std::vector<CDasherNode *> vDeleteList;

  pView->Render(m_Root, m_Rootmin, m_Rootmax, vNodeList, vDeleteList);

  if(!GetBoolParameter(BP_OLD_STYLE_PUSH)) {
  for(std::vector<CDasherNode *>::iterator it(vNodeList.begin()); it != vNodeList.end(); ++it)
    Push_Node(*it);
  }
  
  for(std::vector<CDasherNode *>::iterator it(vDeleteList.begin()); it != vDeleteList.end(); ++it)
    (*it)->Delete_children();
}

bool CDasherModel::RenderToView(CDasherView *pView, int iMouseX, int iMouseY, bool bRedrawDisplay) {
  std::vector<CDasherNode *> vNodeList;
  std::vector<CDasherNode *> vDeleteList;

  bool bReturnValue;

  bReturnValue = pView->Render(m_Root, m_Rootmin, m_Rootmax, vNodeList, vDeleteList, iMouseX, iMouseY, bRedrawDisplay);

  if(!GetBoolParameter(BP_OLD_STYLE_PUSH)) {
  for(std::vector<CDasherNode *>::iterator it(vNodeList.begin()); it != vNodeList.end(); ++it)
    Push_Node(*it);
  }
  
  for(std::vector<CDasherNode *>::iterator it(vDeleteList.begin()); it != vDeleteList.end(); ++it)
    (*it)->Delete_children();

  return bReturnValue;
}

bool CDasherModel::CheckForNewRoot(CDasherView *pView) {

  if(m_deGotoQueue.size() > 0)
    return false;

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
    if((*i)->Alive()) {
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

void CDasherModel::ClickTo(int x, int y, int width, int height, CDasherView *pView) {
  // FIXME - we should just schedule a path here, and tap on should be responsible for following it

   myint dasherx, dashery;

   pView->ClickTo(x, y, dasherx, dashery);

   ScheduleZoom(dasherx,dashery);
}

void CDasherModel::ScheduleZoom(int dasherx, int dashery) {
   if (dasherx < 2) { dasherx = 100; }
   int iSteps = GetLongParameter(LP_ZOOMSTEPS); 
   myint iRxnew = ((GetLongParameter(LP_OX)/2) * GetLongParameter(LP_OX)) / dasherx;
   myint o1, o2, n1, n2;
   Plan_new_goto_coords(iRxnew, dashery, &iSteps, &o1,&o2,&n1,&n2);
   int s ;
   myint rootMin, rootMax;
   rootMin = n1;  
   rootMax = n2;
   myint zoomstep1, zoomstep2;

   std::vector<CDasherNode *> vNodeList;
   std::vector<CDasherNode *> vDeleteList;

   m_deGotoQueue.clear();

   SGotoItem sNewItem;

   for (s = 1 ; s <= iSteps ; s ++) {
     // use simple linear interpolation. Really should do logarithmic interpolation, but
     // this will probably look fine.
     zoomstep1 = (s * n1 + (iSteps-s) * o1) / iSteps;
     zoomstep2 = (s * n2 + (iSteps-s) * o2) / iSteps;

     sNewItem.iN1 = zoomstep1;
     sNewItem.iN2 = zoomstep2;
     sNewItem.iStyle = 1;

     m_deGotoQueue.push_back(sNewItem);

   //   NewGoTo(zoomstep1, zoomstep2, 1);
//      pView->Render(m_Root, m_Rootmin, m_Rootmax, vNodeList, vDeleteList);
//      pView->Display();
   } 

   sNewItem.iN1 = n1;
   sNewItem.iN2 = n2;
   sNewItem.iStyle = 2;
 
   m_deGotoQueue.push_back(sNewItem);

   // NewGoTo(n1, n2, 2);
//    pView->Render(m_Root, m_Rootmin, m_Rootmax, vNodeList, vDeleteList);
//    pView->Display();

//    CheckForNewRoot(pView);
}
