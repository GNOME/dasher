// DasherViewSquare.cpp
//
// Copyright (c) 2001-2004 David Ward

#include "../Common/Common.h"

#include "DasherViewSquare.h"
#include "DasherModel.h"
#include "DasherView.h"
#include "DasherTypes.h"
#include "Event.h"
#include "EventHandler.h"
#include "View/DelayedDraw.h"

#include <algorithm>
#include <limits>

#include <iostream>

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// FIXME - quite a lot of the code here probably should be moved to
// the parent class (DasherView). I think we really should make the
// parent class less general - we're probably not going to implement
// anything which uses radically different co-ordinate transforms, and
// we can always override if necessary.

void CDasherViewSquare::RenderNodes() {
  
  Screen()->Blank();

  DASHER_ASSERT(DasherModel()->Root() != 0);

  //DASHER_TRACEOUTPUT("RenderNodes\n");

  // Render nodes to screen object (should use off screen buffer)

  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;

  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  // FIXME - vNodeList might be redundant - remove if this is the case

  std::vector<CDasherNode *> vNodeList;

  RecursiveRender(DasherModel()->Root(), DasherModel()->Rootmin(), DasherModel()->Rootmax(), iDasherMaxX, vNodeList);
  // DelayDraw the text nodes
  m_pDelayDraw->Draw(Screen());

  //  std::cout << "Nodes to tap on: " << vNodeList.size() << std::endl;

  for(std::vector<CDasherNode *>::iterator it(vNodeList.begin()); it != vNodeList.end(); ++it)
    DasherModel()->Push_Node(*it);

  Crosshair(DasherModel()->DasherOX());  // add crosshair

}

void CDasherViewSquare::HandleEvent(Dasher::CEvent *pEvent) {
  // Let the parent class do its stuff
  CDasherView::HandleEvent(pEvent);

  // And then interpret events for ourself
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    switch (pEvt->m_iParameter) {

    default:
      break;
    }
  }
}

int CDasherViewSquare::RecursiveRender(CDasherNode *pRender, myint y1, myint y2, int mostleft, std::vector<CDasherNode *> &vNodeList) {
  DASHER_ASSERT_VALIDPTR_RW(pRender);

  // Decide which colour to use when rendering the child

  int Color;

  if(GetBoolParameter(BP_COLOUR_MODE) == true) {
    if(pRender->Colour() != -1) {
      Color = pRender->Colour();
    }
    else {
      if(pRender->Symbol() == DasherModel()->GetSpaceSymbol()) {
        Color = 9;
      }
      else if(pRender->Symbol() == DasherModel()->GetControlSymbol()) {
        Color = 8;
      }
      else {
        Color = (pRender->Symbol() % 3) + 10;
      }
    }
  }
  else {
    Color = pRender->Phase() % 3;
  }

  if((pRender->ColorScheme() % 2) == 1 && Color < 130 && GetBoolParameter(BP_COLOUR_MODE) == true) {    // We don't loop on high
    Color += 130;               // colours
  }

  //      DASHER_TRACEOUTPUT("%x ",Render);

  std::string display;
//   if(pRender->GetControlTree() != NULL)
//     display = pRender->GetControlTree()->text;


  display = pRender->m_strDisplayText;

  if(RenderNode(pRender->Symbol(), Color, pRender->ColorScheme(), y1, y2, mostleft, display, pRender->m_bShove)) {
    // yuk
    if(!pRender->ControlChild() && pRender->Symbol() < DasherModel()->GetAlphabet().GetNumberTextSymbols())
      RenderGroups(pRender, y1, y2, mostleft);
  }
  else {
    //    std::cout << "Killing Node" << std::endl;
    pRender->Kill();
    return 0;
  }

  if(pRender->ChildCount() == 0) {
    vNodeList.push_back(pRender);
    return 0;
  }

  int norm = GetLongParameter(LP_NORMALIZATION);
  CDasherNode::ChildMap::const_iterator i;

  for(i = pRender->GetChildren().begin(); i != pRender->GetChildren().end(); i++) {
    CDasherNode *pChild = *i;
    //    if(pChild->Alive()) {
      myint Range = y2 - y1;
      myint newy1 = y1 + (Range * pChild->Lbnd()) / norm;
      myint newy2 = y1 + (Range * pChild->Hbnd()) / norm;

      // FIXME - make the threshold a parameter
      
      if((newy2 - newy1 > 50) || (pChild->Alive())) {
	pChild->Alive(true);
	RecursiveRender(pChild, newy1, newy2, mostleft, vNodeList);
      }
      //    }
  }

  return 1;
}

void CDasherViewSquare::RenderGroups(CDasherNode *Render, myint y1, myint y2, int mostleft) {
  CDasherNode::ChildMap & Children = Render->Children();
  if(Children.size() == 0)
    return;
  int current = 0;
  std::string Label = "";

  myint range = y2 - y1;

  const CAlphabet & alphabet = DasherModel()->GetAlphabet();

  for(int iGroup = 1; iGroup < alphabet.GetGroupCount(); iGroup++) {
    int lower = alphabet.GetGroupStart(iGroup);
    int upper = alphabet.GetGroupEnd(iGroup);

    myint lbnd = Children[lower]->Lbnd();
    myint hbnd = Children[upper - 1]->Hbnd();
    myint newy1 = y1 + (range * lbnd) / (int)GetLongParameter(LP_NORMALIZATION);
    myint newy2 = y1 + (range * hbnd) / (int)GetLongParameter(LP_NORMALIZATION);

    if(GetBoolParameter(BP_COLOUR_MODE) == true) {
      std::string Label = DasherModel()->GroupLabel(iGroup);
      int Colour = DasherModel()->GroupColour(iGroup);

      if(Colour != -1) {
        RenderNode(0, DasherModel()->GroupColour(iGroup), Opts::Groups, newy1, newy2, mostleft, Label, true);
      }
      else {
        RenderNode(0, (current % 3) + 110, Opts::Groups, newy1, newy2, mostleft, Label, true);
      }
    }
    else {
      RenderNode(0, current - 1, Opts::Groups, newy1, newy2, mostleft, Label, true);
    }
  }
}

CDasherViewSquare::Cymap::Cymap(myint iScale) {
  double dY1 = 0.25;            // Amount of acceleration
  double dY2 = 0.95;            // Accelerate Y movement below this point
  double dY3 = 0.05;            // Accelerate Y movement above this point

  m_Y2 = myint(dY2 * iScale);
  m_Y3 = myint(dY3 * iScale);
  m_Y1 = myint(1.0 / dY1);
}

CDasherViewSquare::CDasherViewSquare(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherScreen *DasherScreen, CDasherModel *DasherModel)
:CDasherView(pEventHandler, pSettingsStore, DasherScreen, DasherModel) {

  // Make sure that the auto calibration is set to zero berfore we start

  m_yAutoOffset = 0;

  bInBox = false;
  m_pDelayDraw = new CDelayedDraw();
  ChangeScreen(DasherScreen);

  // tweak these if you know what you are doing
  m_dXmpa = 0.2;                // these are for the x non-linearity
  m_dXmpb = 0.5;
  m_dXmpc = 0.9;
  m_dXmpd = 0.5;                // slow X movement when accelerating Y


  //scale #samples by #samples = m_dSamplesScale / (current bitrate) + m_dSampleOffset
  m_dSampleScale = 1.5;
  m_dSampleOffset = 1.3;
  m_dMinRRate = 80.0;
  m_dSensitivity = GetLongParameter(LP_AUTOSPEED_SENSITIVITY) / 100.0; //param only, no GUI!
  //tolerance for automatic speed control
  m_dTier1 = 0.0005;  //  should be arranged so that tier4 > tier3 > tier2 > tier1 !!!
  m_dTier2 = 0.01;
  m_dTier3 = 0.2;
  m_dTier4 = 0.31;
  //bitrate fractional changes for auto-speed control
  m_dChange1 = 1.1;
  m_dChange2 = 1.02;
  m_dChange3 = 0.97;
  m_dChange4 = 0.94;
  //cap bitrate at...
  m_dSpeedMax = 8.0;
  m_dSpeedMin = 0.1;
  //variance of two-centred-gaussians for adaptive radius
  m_dSigma1 = 0.5; 
  m_dSigma2 = 0.05;
  //Initialise auto-speed control
  m_nSpeedCounter = 0;
  m_dBitrate = double(round(GetLongParameter(LP_MAX_BITRATE) / 100.0));
  UpdateMinRadius();
  UpdateSampleSize(); 
  //KeyControl=false;
  m_ymap = Cymap(DasherModel->DasherY());

  CDasherModel::CRange rActive(m_ymap.unmap(0), m_ymap.unmap(DasherModel->DasherY()));
  DasherModel->SetActive(rActive);
}

CDasherViewSquare::~CDasherViewSquare() {
  if (m_pDelayDraw != NULL) {
    delete m_pDelayDraw;
    m_pDelayDraw = NULL;
  }
}

int CDasherViewSquare::RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme, myint y1, myint y2, int &mostleft, const std::string &displaytext, bool bShove) {

  // Commenting because click mode occasionally fails this assert.
  // I don't know why.  -- cjb.
  if (!(y2 >= y1)) { return 1; }

  // FIXME - Get sensibel limits here (to allow for non-linearities)

  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;

  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  screenint s1, s2;
  Cint32 iSize = dashery2screen(y1, y2, s1, s2);

  // Actual height in pixels
  Cint32 iHeight = Cint32((Cint32) (iSize * CanvasY) / (Cint32) DasherModel()->DasherY());

  if(iHeight <= 1)
    return 0;                   // We're too small to render

  if((y1 > iDasherMaxY) || (y2 < iDasherMinY)){
    return 0;                   // We're entirely off screen, so don't render.
  }

  myint iDasherSize(y2 - y1);

  // FIXME - get rid of pointless assignment below

  int iTruncation(GetLongParameter(LP_TRUNCATION));     // Trucation farction times 100;
  int iTruncationType(GetLongParameter(LP_TRUNCATIONTYPE));

  if(iTruncation == 0) {        // Regular squares
    DasherDrawRectangle(std::min(iDasherSize,iDasherMaxX), std::min(y2,iDasherMaxY), 0, std::max(y1,iDasherMinY), Color, -1, ColorScheme, GetBoolParameter(BP_OUTLINE_MODE), true, 1);
  }
  else {
    int iDasherY(DasherModel()->DasherY());

    int iSpacing(iDasherY / 128);       // FIXME - assuming that this is an integer below

    int iXStart = 0;

    switch (iTruncationType) {
    case 1:
      iXStart = iSize - iSize * iTruncation / 200;
      break;
    case 2:
      iXStart = iSize - iSize * iTruncation / 100;
      break;
    }

    int iTipMin((y2 - y1) * iTruncation / (200) + y1);
    int iTipMax(y2 - (y2 - y1) * iTruncation / (200));

    int iLowerMin(((y1 + 1) / iSpacing) * iSpacing);
    int iLowerMax(((iTipMin - 1) / iSpacing) * iSpacing);

    int iUpperMin(((iTipMax + 1) / iSpacing) * iSpacing);
    int iUpperMax(((y2 - 1) / iSpacing) * iSpacing);

    if(iLowerMin < 0)
      iLowerMin = 0;

    if(iLowerMax < 0)
      iLowerMax = 0;

    if(iUpperMin < 0)
      iUpperMin = 0;

    if(iUpperMax < 0)
      iUpperMax = 0;

    if(iLowerMin > iDasherY)
      iLowerMin = iDasherY;

    if(iLowerMax > iDasherY)
      iLowerMax = iDasherY;

    if(iUpperMin > iDasherY)
      iUpperMin = iDasherY;

    if(iUpperMax > iDasherY)
      iUpperMax = iDasherY;

    while(iLowerMin < y1)
      iLowerMin += iSpacing;

    while(iLowerMax > iTipMin)
      iLowerMax -= iSpacing;

    while(iUpperMin < iTipMax)
      iUpperMin += iSpacing;

    while(iUpperMax > y2)
      iUpperMax -= iSpacing;

    int iLowerCount((iLowerMax - iLowerMin) / iSpacing + 1);
    int iUpperCount((iUpperMax - iUpperMin) / iSpacing + 1);

    if(iLowerCount < 0)
      iLowerCount = 0;

    if(iUpperCount < 0)
      iUpperCount = 0;

    int iTotalCount(iLowerCount + iUpperCount + 6);

    myint *x = new myint[iTotalCount];
    myint *y = new myint[iTotalCount];

    // Weird duplication here is to make truncated squares possible too

    x[0] = 0;
    y[0] = y1;
    x[1] = iXStart;
    y[1] = y1;

    x[iLowerCount + 2] = iDasherSize;
    y[iLowerCount + 2] = iTipMin;
    x[iLowerCount + 3] = iDasherSize;
    y[iLowerCount + 3] = iTipMax;

    x[iTotalCount - 2] = iXStart;
    y[iTotalCount - 2] = y2;
    x[iTotalCount - 1] = 0;
    y[iTotalCount - 1] = y2;

    for(int i(0); i < iLowerCount; ++i) {
      x[i + 2] = (iLowerMin + i * iSpacing - y1) * (iDasherSize - iXStart) / (iTipMin - y1) + iXStart;
      y[i + 2] = iLowerMin + i * iSpacing;
    }

    for(int j(0); j < iUpperCount; ++j) {
      x[j + iLowerCount + 4] = (y2 - (iUpperMin + j * iSpacing)) * (iDasherSize - iXStart) / (y2 - iTipMax) + iXStart;
      y[j + iLowerCount + 4] = iUpperMin + j * iSpacing;
    }

    DasherPolygon(x, y, iTotalCount, Color);

    delete x;
    delete y;

  }

  myint iDasherAnchorX(iDasherSize);

  std::string sDisplayText;

  if(displaytext != std::string(""))
    sDisplayText = displaytext;
  else
    sDisplayText = DasherModel()->GetDisplayText(Character);


  if( sDisplayText.size() > 0 )
    DasherDrawText(iDasherAnchorX, y1, iDasherAnchorX, y2, sDisplayText, mostleft, bShove);

  return 1;
}

bool CDasherViewSquare::CheckForNewRoot() {
  CDasherNode *const root = DasherModel()->Root();
  CDasherNode::ChildMap & children = root->Children();

  myint y1 = DasherModel()->Rootmin();
  myint y2 = DasherModel()->Rootmax();

  // This says that the root node must enclose everything visible.
  // Tiny probability characters near the bottom will cause a problem
  // with forcing to reparent to the previous one.

  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;

  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  if((y1 > iDasherMinY) || (y2 < iDasherMaxY ) || (y2-y1 < iDasherMaxX)) {
    DasherModel()->Reparent_root(root->Lbnd(), root->Hbnd());
    return true;
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

    y1 = DasherModel()->Rootmin();
    y2 = DasherModel()->Rootmax();
    myint range = y2 - y1;
    
    myint newy1 = y1 + (range * theone->Lbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
    myint newy2 = y1 + (range * theone->Hbnd()) / (int)GetLongParameter(LP_NORMALIZATION);
    if(newy1 < iDasherMinY && newy2 > iDasherMaxY)
      if( (newy2 - newy1) > iDasherMaxX ) {
        DasherModel()->Make_root(theone);
        return false;
    }
  }

  return false;
}

/// Convert screen co-ordinates to dasher co-ordinates. This doesn't
/// include the nonlinear mapping for eyetracking mode etc - it is
/// just the inverse of the mapping used to calculate the screen
/// positions of boxes etc.

void CDasherViewSquare::Screen2Dasher(screenint iInputX, screenint iInputY, myint &iDasherX, myint &iDasherY, bool b1D, bool bNonlinearity) {

  // Things we're likely to need:

  myint iDasherWidth = DasherModel()->DasherY();
  myint iDasherHeight = DasherModel()->DasherY();

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  if( b1D ) { // Special case for 1D mode...
    iDasherX = iInputX * iDasherWidth / iScreenWidth;
    iDasherY = iInputY * iDasherHeight / iScreenHeight;
    return;
  }

  int eOrientation(GetLongParameter(LP_REAL_ORIENTATION));

  myint iScaleFactorX;
  myint iScaleFactorY;
  
  GetScaleFactor(eOrientation, &iScaleFactorX, &iScaleFactorY);

  switch(eOrientation) {
  case Dasher::Opts::LeftToRight:
    iDasherX = iDasherWidth / 2 - ( iInputX - iScreenWidth / 2 ) * m_iScalingFactor / iScaleFactorX;
    iDasherY = iDasherHeight / 2 + ( iInputY - iScreenHeight / 2 ) * m_iScalingFactor / iScaleFactorY;
    break;
  case Dasher::Opts::RightToLeft:
    iDasherX = myint(iDasherWidth / 2 + ( iInputX - iScreenWidth / 2 ) * m_iScalingFactor/ iScaleFactorX);
    iDasherY = myint(iDasherHeight / 2 + ( iInputY - iScreenHeight / 2 ) * m_iScalingFactor/ iScaleFactorY);
    break;
  case Dasher::Opts::TopToBottom:
    iDasherX = myint(iDasherWidth / 2 - ( iInputY - iScreenHeight / 2 ) * m_iScalingFactor/ iScaleFactorY);
    iDasherY = myint(iDasherHeight / 2 + ( iInputX - iScreenWidth / 2 ) * m_iScalingFactor/ iScaleFactorX);
    break;
  case Dasher::Opts::BottomToTop:
    iDasherX = myint(iDasherWidth / 2 + ( iInputY - iScreenHeight / 2 ) * m_iScalingFactor/ iScaleFactorY);
    iDasherY = myint(iDasherHeight / 2 + ( iInputX - iScreenWidth / 2 ) * m_iScalingFactor/ iScaleFactorX);
    break;
  }

#ifndef WITH_MAEMO
  // FIXME - disabled to avoid floating point
  if( bNonlinearity ) {
  iDasherX = myint(ixmap(iDasherX / static_cast < double >(DasherModel()->DasherY())) * DasherModel()->DasherY());
  iDasherY = m_ymap.unmap(iDasherY);
  }
#endif
}

void CDasherViewSquare::SetScaleFactor( void )
{
  myint iDasherWidth = DasherModel()->DasherY();
  myint iDasherHeight = iDasherWidth;

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  // Try doing this a different way:

  myint iDasherMargin( 300 ); // Make this a parameter

  myint iMinX( 0-iDasherMargin );
  myint iMaxX( iDasherWidth + iDasherMargin );
  myint iMinY( 0 );
  myint iMaxY( iDasherHeight );

  double dLRHScaleFactor;
  double dLRVScaleFactor;
  double dTBHScaleFactor;
  double dTBVScaleFactor;

  dLRHScaleFactor = iScreenWidth / static_cast<double>( iMaxX - iMinX );
  dLRVScaleFactor = iScreenHeight / static_cast<double>( iMaxY - iMinY );
  dTBHScaleFactor = iScreenWidth / static_cast<double>( iMaxY - iMinY );
  dTBVScaleFactor = iScreenHeight / static_cast<double>( iMaxX - iMinX );

  iLRScaleFactorX = std::max(std::min(dLRHScaleFactor, dLRVScaleFactor), dLRHScaleFactor / 4.0) * m_iScalingFactor;
  iLRScaleFactorY = std::max(std::min(dLRHScaleFactor, dLRVScaleFactor), dLRVScaleFactor / 4.0) * m_iScalingFactor;
  iTBScaleFactorX = std::max(std::min(dTBHScaleFactor, dTBVScaleFactor), dTBVScaleFactor / 4.0) * m_iScalingFactor;
  iTBScaleFactorY = std::max(std::min(dTBHScaleFactor, dTBVScaleFactor), dTBHScaleFactor / 4.0) * m_iScalingFactor;
}

void CDasherViewSquare::GetScaleFactor( int eOrientation, myint *iScaleFactorX, myint *iScaleFactorY ) {
  if(( eOrientation == Dasher::Opts::LeftToRight ) || ( eOrientation == Dasher::Opts::RightToLeft )) {
    *iScaleFactorX = iLRScaleFactorX;
    *iScaleFactorY = iLRScaleFactorY;
  } else {
    *iScaleFactorX = iTBScaleFactorX;
    *iScaleFactorY = iTBScaleFactorY;
  }
}

/// Convert dasher co-ordinates to screen co-ordinates
  
void CDasherViewSquare::Dasher2Screen(myint iDasherX, myint iDasherY, screenint &iScreenX, screenint &iScreenY) {

  // Apply the nonlinearities

#ifndef WITH_MAEMO
  // FIXME
  iDasherX = myint(xmap(iDasherX / static_cast < double >(DasherModel()->DasherY())) * DasherModel()->DasherY());
  iDasherY = m_ymap.map(iDasherY);
#endif

  // Things we're likely to need:

  myint iDasherWidth = DasherModel()->DasherY();
  myint iDasherHeight = DasherModel()->DasherY();

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  int eOrientation( GetLongParameter(LP_REAL_ORIENTATION) );

  myint iScaleFactorX;
  myint iScaleFactorY;

  GetScaleFactor( eOrientation, &iScaleFactorX, &iScaleFactorY);

  switch( eOrientation ) {
  case Dasher::Opts::LeftToRight:
    iScreenX = screenint(iScreenWidth / 2 - ( iDasherX - iDasherWidth / 2 ) * iScaleFactorX / m_iScalingFactor);
    iScreenY = screenint(iScreenHeight / 2 + ( iDasherY - iDasherHeight / 2 ) * iScaleFactorY / m_iScalingFactor);
    break;
  case Dasher::Opts::RightToLeft:
    iScreenX = screenint(iScreenWidth / 2 + ( iDasherX - iDasherWidth / 2 ) * iScaleFactorX / m_iScalingFactor);
    iScreenY = screenint(iScreenHeight / 2 + ( iDasherY - iDasherHeight / 2 ) * iScaleFactorY / m_iScalingFactor);
    break;
  case Dasher::Opts::TopToBottom:
    iScreenX = screenint(iScreenWidth / 2 + ( iDasherY - iDasherHeight / 2 ) * iScaleFactorX / m_iScalingFactor);
    iScreenY = screenint(iScreenHeight / 2 - ( iDasherX - iDasherWidth / 2 ) * iScaleFactorY / m_iScalingFactor);
    break;
  case Dasher::Opts::BottomToTop:
    iScreenX = screenint(iScreenWidth / 2 + ( iDasherY - iDasherHeight / 2 ) * iScaleFactorX / m_iScalingFactor);
    iScreenY = screenint(iScreenHeight / 2 + ( iDasherX - iDasherWidth / 2 ) * iScaleFactorY / m_iScalingFactor);
    break;
  }

  //  std::cout << iScaleFactorX << " " << iScaleFactorY  << " " << iScreenWidth << " " << iScreenHeight << std::endl;
  //std::cout << iDasherX << " -> " << iScreenX << ", " << iDasherY  << " -> " << iScreenY << std::endl;

}




void CDasherViewSquare::VisibleRegion( myint &iDasherMinX, myint &iDasherMinY, myint &iDasherMaxX, myint &iDasherMaxY ) {
  int eOrientation( GetLongParameter(LP_REAL_ORIENTATION) );
  
  switch( eOrientation ) {
  case Dasher::Opts::LeftToRight:
    Screen2Dasher(Screen()->GetWidth(),0,iDasherMinX,iDasherMinY,false,true);
    Screen2Dasher(0,Screen()->GetHeight(),iDasherMaxX,iDasherMaxY,false,true);
    break;
  case Dasher::Opts::RightToLeft:
    Screen2Dasher(0,0,iDasherMinX,iDasherMinY,false,true);
    Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),iDasherMaxX,iDasherMaxY,false,true);
    break;
  case Dasher::Opts::TopToBottom:
    Screen2Dasher(0,Screen()->GetHeight(),iDasherMinX,iDasherMinY,false,true);
    Screen2Dasher(Screen()->GetWidth(),0,iDasherMaxX,iDasherMaxY,false,true);
    break;
  case Dasher::Opts::BottomToTop:
    Screen2Dasher(0,0,iDasherMinX,iDasherMinY,false,true);
    Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),iDasherMaxX,iDasherMaxY,false,true);
    break;
  }
}

/// The minimum Dasher Y co-ordinate which will be visible

myint CDasherViewSquare::DasherVisibleMinY() {

  // Todo - convert all these to a single 'get visible extent' function

  myint iDasherX;
  myint iDasherY;

  int eOrientation( GetLongParameter(LP_REAL_ORIENTATION) );

  switch( eOrientation ) {
  case Dasher::Opts::LeftToRight:
    Screen2Dasher(Screen()->GetWidth(),0,iDasherX,iDasherY,false,true);
    break;
  case Dasher::Opts::RightToLeft:
    Screen2Dasher(0,0,iDasherX,iDasherY,false,true);
    break;
  case Dasher::Opts::TopToBottom:
    Screen2Dasher(0,Screen()->GetHeight(),iDasherX,iDasherY,false,true);
    break;
  case Dasher::Opts::BottomToTop:
    Screen2Dasher(0,0,iDasherX,iDasherY,false,true);
    break;
  }

  return iDasherY;
}

/// The maximum Dasher Y co-ordinate which will be visible

myint CDasherViewSquare::DasherVisibleMaxY() {
  // Todo - convert all these to a single 'get visible extent' function

  myint iDasherX;
  myint iDasherY;

  int eOrientation( GetLongParameter(LP_REAL_ORIENTATION) );

  switch( eOrientation ) {
  case Dasher::Opts::LeftToRight:
    Screen2Dasher(0,Screen()->GetHeight(),iDasherX,iDasherY,false,true);
    break;
  case Dasher::Opts::RightToLeft:
    Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),iDasherX,iDasherY,false,true);
    break;
  case Dasher::Opts::TopToBottom:
    Screen2Dasher(Screen()->GetWidth(),0,iDasherX,iDasherY,false,true);
    break;
  case Dasher::Opts::BottomToTop:
    Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),iDasherX,iDasherY,false,true);
    break;
  }

  return iDasherY;
 }

/// The maximum Dasher X co-ordinate which will be visible

myint CDasherViewSquare::DasherVisibleMaxX() {
   // Todo - convert all these to a single 'get visible extent' function

  myint iDasherX;
  myint iDasherY;

  int eOrientation( GetLongParameter(LP_REAL_ORIENTATION) );

  switch( eOrientation ) {
  case Dasher::Opts::LeftToRight:
    Screen2Dasher(0,Screen()->GetHeight(),iDasherX,iDasherY,false,true);
    break;
  case Dasher::Opts::RightToLeft:
    Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),iDasherX,iDasherY,false,true);
    break;
  case Dasher::Opts::TopToBottom:
    Screen2Dasher(Screen()->GetWidth(),0,iDasherX,iDasherY,false,true);
    break;
  case Dasher::Opts::BottomToTop:
    Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),iDasherX,iDasherY,false,true);
    break;
  }

  return iDasherX;
}

/// Convert raw Dasher co-ordinates to the equivalent 1D mode position

void CDasherViewSquare::Dasher2OneD(myint &iDasherX, myint &iDasherY) {
  
    double disty,circlesize,yfullrange,yforwardrange,angle,ellipse_eccentricity,ybackrange,yb,x;	
    
    // The distance between the Y coordinate and the centreline in pixels
    disty=DasherModel()->DasherOY()-iDasherY;
        
    circlesize=    DasherModel()->DasherY()/2.5;
    yforwardrange= DasherModel()->DasherY()/3.2; // Was 1.6
    yfullrange=    yforwardrange*1.6;
    ybackrange=    yfullrange-yforwardrange;
    ellipse_eccentricity=6;
 
    if (disty>yforwardrange) {
      // If the distance between y-coord and centreline is > radius,
      // we should be going backwards, off the top.
      yb=(disty-yforwardrange)/ybackrange;
      
      if (yb>1) {
	x=0;
	iDasherY=myint(DasherModel()->DasherOY());
      }
      else { 
	angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));

	x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
	iDasherY=myint(-(1+cos(angle))*circlesize/2+DasherModel()->DasherOY());
      }
    }
    else if (disty <-(yforwardrange)) {
      // Backwards, off the bottom.
      yb=-(disty+yforwardrange)/ybackrange;
      
      if (yb>1) {
	x=0;
	iDasherY=myint(DasherModel()->DasherOY());
      }   
      else {
	angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));
	
	x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
	iDasherY=myint((1+cos(angle))*circlesize/2+DasherModel()->DasherOY());
      }   
    }
    
    else {
      angle=((disty*3.14159/2)/yforwardrange);
      x=cos(angle)*circlesize;
      iDasherY=myint(-sin(angle)*circlesize+DasherModel()->DasherOY());
    }
    x=DasherModel()->DasherOX()-x;

    iDasherX = myint(x);
}

/// Convert raw Dasher co-ordinates to eyetracker position

void CDasherViewSquare::Dasher2Eyetracker(myint &iDasherX, myint &iDasherY) {

  double disty=DasherModel()->DasherOY()-iDasherY;

  myint x( iDasherX );

  myint dasherOX=DasherModel()->DasherOX(); 

  //  if( iDasherX < dasherOX ) {

      //cout << "dasherOX: " << dasherOX << endl; 
      myint dasherOY=DasherModel()->DasherOY(); 
         
      // X co-ordinate changes. 
      double double_x = (x/dasherOX);  // Fraction of way over to crosshair
      double double_y = -((iDasherY-dasherOY)/(double)(dasherOY) ); // Fraction above the crosshair
             
      // FIXME - I have *no* idea how this is supposed to work - someone else fix it and comment the code please!

      double xmax_y = xmax(double_x, double_y); 
                 
      if(double_x < xmax_y) { 
        double_x = xmax_y; 
      } 

      //      std::cout << xmax_y << std::endl;

      x = myint(dasherOX*double_x);

      // Finished x-coord changes.

      double repulsionparameter=0.5;
      iDasherY = myint(dasherOY - (1.0+ double_y*double_y* repulsionparameter ) * disty);


      if( iDasherX < x )
      iDasherX = x;
}

/// Convert abstract 'input coordinates', which may or may not
/// correspond to actual screen positions, depending on the settings,
/// into dasher co-ordinates. Modes are:
///
/// 0 = Direct (ie mouse)
/// 1 = 1D
/// 2 = Eyetracker
///
/// This should be done once initially, then we work in Dasher
/// co-ordinates for everything else. Input co-ordinates will be
/// assumed to range over the extent of the screen.
///
/// TODO: Abstract out modes into an enum

void CDasherViewSquare::Input2Dasher(screenint iInputX, screenint iInputY, myint &iDasherX, myint &iDasherY, int iType, int iMode) {

  // FIXME - need to incorporate one-button mode?

  // First convert the supplied co-ordinates to 'linear' Dasher co-ordinates

  switch (iType) {
  case 0:
    // Raw secreen coordinates
    if(iMode == 2) {
      // First apply the autocalibration offset
      iInputY += int (m_yAutoOffset);   // FIXME - we need more flexible autocalibration to work with orientations other than left-to-right
    }

    if( iMode == 0 )
      Screen2Dasher( iInputX, iInputY, iDasherX, iDasherY, false, true );
    else if( iMode == 1 )
      Screen2Dasher( iInputX, iInputY, iDasherX, iDasherY, true, false );
    else
      Screen2Dasher( iInputX, iInputY, iDasherX, iDasherY, false, true );
    break;
  case 1:
    // Raw dasher coordinates

    iDasherX = iInputX;
    iDasherY = iInputY;
    break;
  default:
    // ERROR
    break;
  }

  // Apply y scaling

  if(iMode == 1 ) {
    if( GetLongParameter(LP_YSCALE) > 0 ) {
      
      double dYScale;
      
      int eOrientation(GetLongParameter(LP_REAL_ORIENTATION));
      
      if(( eOrientation == Dasher::Opts::LeftToRight ) || ( eOrientation == Dasher::Opts::RightToLeft ))
	dYScale = Screen()->GetHeight() / static_cast<double>(GetLongParameter(LP_YSCALE));
      else
	dYScale = Screen()->GetWidth() / static_cast<double>(GetLongParameter(LP_YSCALE));
      
      iDasherY = myint((iDasherY - DasherModel()->DasherY()/2) * dYScale + DasherModel()->DasherY()/2);
    }
  }

  // Then apply any non-linearity

  switch (iMode) {
  case 0:                      // Direct mode
    // Simply get the dasher co-ordinate under the mouse cursor
    //    Screen2Dasher( iInputX, iInputY, iDasherX, iDasherY );

    // Don't go off the canvas - FIXME - is this always needed, or just in direct mode?
//     if(iDasherY > DasherModel()->DasherY())
//       iDasherY = DasherModel()->DasherY();
//     if(iDasherY < 0)
//       iDasherY = 0;

    break;
  case 1:                      // 1D mode
    // Ignore orientation - iInputY maps directly to the single dimension in this case
    //    iDasherY = iInputY * DasherModel()->DasherY() / Screen()->GetHeight();

    // Apply non-linear mapping
    Dasher2OneD(iDasherX, iDasherY);

    break;
  case 2:                      // Eyetracker mode

    // Then find the dasher co-ordinate under the offset mouse position

    // Finally apply the non-linear transformation
    Dasher2Eyetracker(iDasherX, iDasherY);

    break;
  default:
    // Oops!
    break;
  }
}

/// Draw a polyline specified in Dasher co-ordinates

void CDasherViewSquare::DasherPolyline(myint *x, myint *y, int n, int iWidth, int iColour) {

  CDasherScreen::point * ScreenPoints = new CDasherScreen::point[n];

  for(int i(0); i < n; ++i)
    Dasher2Screen(x[i], y[i], ScreenPoints[i].x, ScreenPoints[i].y);

  if(iColour != -1) {
    Screen()->Polyline(ScreenPoints, n, iWidth, iColour);
  }
  else {
    Screen()->Polyline(ScreenPoints, n, iWidth);
  }
  delete[]ScreenPoints;
}

// Draw a filled polygon specified in Dasher co-ordinates

void CDasherViewSquare::DasherPolygon(myint *x, myint *y, int n, int iColour) {

  CDasherScreen::point * ScreenPoints = new CDasherScreen::point[n];

  for(int i(0); i < n; ++i)
    Dasher2Screen(x[i], y[i], ScreenPoints[i].x, ScreenPoints[i].y);

  Screen()->Polygon(ScreenPoints, n, iColour);
  delete[]ScreenPoints;
}

// Draw a box specified in Dasher co-ordinates

void CDasherViewSquare::DasherDrawRectangle(myint iLeft, myint iTop, myint iRight, myint iBottom, const int Color, int iOutlineColour, Opts::ColorSchemes ColorScheme, bool bDrawOutline, bool bFill, int iThickness) {

  screenint iScreenLeft;
  screenint iScreenTop;
  screenint iScreenRight;
  screenint iScreenBottom;

  Dasher2Screen(iLeft, iTop, iScreenLeft, iScreenTop);
  Dasher2Screen(iRight, iBottom, iScreenRight, iScreenBottom);

  Screen()->DrawRectangle(iScreenLeft, iScreenTop, iScreenRight, iScreenBottom, Color, iOutlineColour, ColorScheme, bDrawOutline, bFill, iThickness);
}

/// Draw a rectangle centred on a given dasher co-ordinate, but with a size specified in screen co-ordinates (used for drawing the mouse blob)

void CDasherViewSquare::DasherDrawCentredRectangle(myint iDasherX, myint iDasherY, screenint iSize, const int Color, Opts::ColorSchemes ColorScheme, bool bDrawOutline) {

  screenint iScreenX;
  screenint iScreenY;

  Dasher2Screen(iDasherX, iDasherY, iScreenX, iScreenY);

  Screen()->DrawRectangle(iScreenX - iSize, iScreenY - iSize, iScreenX + iSize, iScreenY + iSize, Color, -1, ColorScheme, bDrawOutline, true, 1);
}

/// Draw text specified in Dasher co-ordinates. The position is
/// specified as two co-ordinates, intended to the be the corners of
/// the leading edge of the containing box.

void CDasherViewSquare::DasherDrawText(myint iAnchorX1, myint iAnchorY1, myint iAnchorX2, myint iAnchorY2, const std::string &sDisplayText, int &mostleft, bool bShove) {

  // Don't draw text which will overlap with text in an ancestor.

  if(iAnchorX1 > mostleft)
    iAnchorX1 = mostleft;

  if(iAnchorX2 > mostleft)
    iAnchorX2 = mostleft;

  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;

  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  iAnchorY1 = std::min( iDasherMaxY, std::max( iDasherMinY, iAnchorY1 ) );
  iAnchorY2 = std::min( iDasherMaxY, std::max( iDasherMinY, iAnchorY2 ) );

  screenint iScreenAnchorX1;
  screenint iScreenAnchorY1;
  screenint iScreenAnchorX2;
  screenint iScreenAnchorY2;

  // FIXME - Truncate here before converting - otherwise we risk integer overflow in screen coordinates

  Dasher2Screen(iAnchorX1, iAnchorY1, iScreenAnchorX1, iScreenAnchorY1);
  Dasher2Screen(iAnchorX2, iAnchorY2, iScreenAnchorX2, iScreenAnchorY2);

  // Truncate the ends of the anchor line to be on the screen - this
  // prevents us from loosing characters off the top and bottom of the
  // screen

  // TruncateToScreen(iScreenAnchorX1, iScreenAnchorY1);
  // TruncateToScreen(iScreenAnchorX2, iScreenAnchorY2);

  // Actual anchor point is the midpoint of the anchor line

  screenint iScreenAnchorX((iScreenAnchorX1 + iScreenAnchorX2) / 2);
  screenint iScreenAnchorY((iScreenAnchorY1 + iScreenAnchorY2) / 2);

  // Compute font size based on position
  int Size = GetLongParameter( LP_DASHER_FONTSIZE );

  // FIXME - this could be much more elegant, and probably needs a
  // rethink anyway - behvaiour here is too dependent on screen size

  screenint iLeftTimesFontSize = (DasherModel()->DasherY() - (iAnchorX1 + iAnchorX2)/ 2 )*Size;
  if(iLeftTimesFontSize < DasherModel()->DasherY() * 19/ 20)
    Size *= 20;
  else if(iLeftTimesFontSize < DasherModel()->DasherY() * 159 / 160)
    Size *= 14;
  else
    Size *= 11;
  

  screenint TextWidth, TextHeight;

  Screen()->TextSize(sDisplayText, &TextWidth, &TextHeight, Size);

  // Poistion of text box relative to anchor depends on orientation

  screenint newleft2 = 0;
  screenint newtop2 = 0;
  screenint newright2 = 0;
  screenint newbottom2 = 0;

  switch (Dasher::Opts::ScreenOrientations(GetLongParameter(LP_REAL_ORIENTATION))) {
  case (Dasher::Opts::LeftToRight):
    newleft2 = iScreenAnchorX;
    newtop2 = iScreenAnchorY - TextHeight / 2;
    newright2 = iScreenAnchorX + TextWidth;
    newbottom2 = iScreenAnchorY + TextHeight / 2;
    break;
  case (Dasher::Opts::RightToLeft):
    newleft2 = iScreenAnchorX - TextWidth;
    newtop2 = iScreenAnchorY - TextHeight / 2;
    newright2 = iScreenAnchorX;
    newbottom2 = iScreenAnchorY + TextHeight / 2;
    break;
  case (Dasher::Opts::TopToBottom):
    newleft2 = iScreenAnchorX - TextWidth / 2;
    newtop2 = iScreenAnchorY;
    newright2 = iScreenAnchorX + TextWidth / 2;
    newbottom2 = iScreenAnchorY + TextHeight;
    break;
  case (Dasher::Opts::BottomToTop):
    newleft2 = iScreenAnchorX - TextWidth / 2;
    newtop2 = iScreenAnchorY - TextHeight;
    newright2 = iScreenAnchorX + TextWidth / 2;
    newbottom2 = iScreenAnchorY;
    break;
  default:
    break;
  }

  // Update the value of mostleft to take into account the new text

  if(bShove) {
    myint iDasherNewLeft;
    myint iDasherNewTop;
    myint iDasherNewRight;
    myint iDasherNewBottom;
    
    Screen2Dasher(newleft2, newtop2, iDasherNewLeft, iDasherNewTop,false,true);
    Screen2Dasher(newright2, newbottom2, iDasherNewRight, iDasherNewBottom,false,true);
    
    mostleft = std::min(iDasherNewRight, iDasherNewLeft);
  }

  // Actually draw the text. We use DelayDrawText as the text should
  // be overlayed once all of the boxes have been drawn.

  m_pDelayDraw->DelayDrawText(sDisplayText, newleft2, newtop2, Size);

}

/// Truncate a set of co-ordinates so that they are on the screen

void CDasherViewSquare::TruncateToScreen(screenint &iX, screenint &iY) {

  // I think that this function is now obsolete

  if(iX < 0)
    iX = 0;
  if(iX > Screen()->GetWidth())
    iX = Screen()->GetWidth();

  if(iY < 0)
    iY = 0;
  if(iY > Screen()->GetHeight())
    iY = Screen()->GetHeight();
}

// work out the next viewpoint
// move the rectangles accordingly
void CDasherViewSquare::TapOnDisplay(screenint mousex,
                                     screenint mousey, 
                                     unsigned long Time, 
                                     Dasher::VECTOR_SYMBOL_PROB* pAdded, 
                                     int* pNumDeleted) {

  // NOTE - we now ignore the values which are actually passed to the display

  // FIXME - Actually turn autocalibration on and off!
  // FIXME - AutoCalibrate should use Dasher co-ordinates, not raw mouse co-ordinates?
  // FIXME - Have I broken this by moving it before the offset is applied?
  // FIXME - put ymap stuff back in 

  // FIXME - optimise this

  int iCoordinateCount(GetCoordinateCount());

  myint *pCoordinates(new myint[iCoordinateCount]);

  int iType(GetCoordinates(iCoordinateCount, pCoordinates));

  if(iCoordinateCount == 1) {
    mousex = 0;
    mousey = pCoordinates[0];
  }
  else {
    mousex = pCoordinates[0];
    mousey = pCoordinates[1];
  }

  delete[]pCoordinates;

  //  bool autocalibrate = GetBoolParameter(BP_AUTOCALIBRATE);
  if(GetBoolParameter(BP_AUTOCALIBRATE) && GetBoolParameter(BP_EYETRACKER_MODE)) {
    AutoCalibrate(&mousex, &mousey);
  }

  myint iDasherX;
  myint iDasherY;

  // Convert the input co-ordinates to dasher co-ordinates

  Input2Dasher(mousex, mousey, iDasherX, iDasherY, iType, DasherModel()->GetMode());
  m_iDasherXCache = iDasherX;
  m_iDasherYCache = iDasherY;

  // Request an update at the calculated co-ordinates

  DasherModel()->Tap_on_display(iDasherX,iDasherY, Time, pAdded, pNumDeleted);

  SpeedControl(iDasherX, iDasherY);

  CheckForNewRoot();
  // Cache the Dasher Co-ordinates, so we can use them later for things like drawing the mouse position
#ifndef WITH_MAEMO
  // FIXME
  iDasherX = myint(xmap(iDasherX / static_cast < double >(DasherModel()->DasherY())) * DasherModel()->DasherY());
  iDasherY = m_ymap.map(iDasherY);
#endif
}

void CDasherViewSquare::ClickTo(int x, int y, int width, int height)
{
  myint dasherx, dashery;
  Screen2Dasher(x,y,dasherx,dashery,false,false);
  if (dasherx < 2) { dasherx = 100; }
  int iSteps = GetLongParameter(LP_ZOOMSTEPS); 
  myint iRxnew = ((DasherModel()->DasherOX()/2) * DasherModel()->DasherOX()) / dasherx;
  myint o1, o2, n1, n2;
  DasherModel()->Plan_new_goto_coords(iRxnew, dashery, &iSteps, &o1,&o2,&n1,&n2);
  int s ;
  myint rootMin, rootMax;
  rootMin = n1;  
  rootMax = n2;
  myint zoomstep1, zoomstep2;
  for (s = 1 ; s <= iSteps ; s ++) {
    // use simple linear interpolation. Really should do logarithmic interpolation, but
    // this will probably look fine.
    zoomstep1 = (s * n1 + (iSteps-s) * o1) / iSteps;
    zoomstep2 = (s * n2 + (iSteps-s) * o2) / iSteps;
    DasherModel()->NewGoTo(zoomstep1, zoomstep2, 1);
    Render();
    Display();
  }
  DasherModel()->NewGoTo(n1, n2, 2);
  Render();
  Display();
}


// move to the specified point

void CDasherViewSquare::GoTo(screenint mousex, screenint mousey) {
  // convert mouse (screen) coords into dasher coords

  UnMapScreen(&mousex, &mousey);
  myint idasherx, idashery;
  Screen2Dasher(mousex, mousey, idasherx, idashery, false, false);
  DasherModel()->GoTo(idasherx, idashery);
  CheckForNewRoot();
}

void CDasherViewSquare::DrawGoTo(screenint mousex, screenint mousey) {
  // Draw a box surrounding the area of the screen that will be zoomed into
  UnMapScreen(&mousex, &mousey);
  myint idasherx, idashery;
  screen2dasher(mousex, mousey, &idasherx, &idashery);
  // So, we have a set of coordinates. We need a bunch of points back.
  myint height = DasherModel()->PlotGoTo(idasherx, idashery);
  myint top, bottom, left, right;

  // Convert back to screen coordinates?
  top = mousey - height / 2;
  bottom = mousey + height / 2;
  left = mousex + height / 2;
  right = mousex - height / 2;
  top = dashery2screen(top);
  bottom = dashery2screen(bottom);
  left = dasherx2screen(left);
  right = dasherx2screen(right);

  // Draw the lines
  Screen()->DrawRectangle(left, top + 5, right, top - 5, 1, -1, Opts::ColorSchemes(Objects), false, true, 1);
  Screen()->DrawRectangle(left + 5, top + 5, left, bottom - 5, 1, -1, Opts::ColorSchemes(Objects), false, true, 1);
  Screen()->DrawRectangle(left, bottom + 5, right, bottom - 5, 1, -1, Opts::ColorSchemes(Objects), false, true, 1);
}

void CDasherViewSquare::DrawMouse(screenint mousex, screenint mousey) {

  int iCoordinateCount(GetCoordinateCount());

  myint *pCoordinates(new myint[iCoordinateCount]);

  int iType(GetCoordinates(iCoordinateCount, pCoordinates));

  if(iCoordinateCount == 1) {
    mousex = 0;
    mousey = pCoordinates[0];
  }
  else {
    mousex = pCoordinates[0];
    mousey = pCoordinates[1];
  }

  delete[]pCoordinates;
  myint iDasherX;
  myint iDasherY;

  Input2Dasher(mousex, mousey, iDasherX, iDasherY, iType, DasherModel()->GetMode());

  if(GetBoolParameter(BP_COLOUR_MODE) == true) {
    DasherDrawCentredRectangle(iDasherX, iDasherY, 5, 2, Opts::ColorSchemes(Objects), false);
  }
  else {
    DasherDrawCentredRectangle(iDasherX, iDasherY, 5, 1, Opts::ColorSchemes(Objects), false);
  }

}

/// Draw a line from the crosshair to the mouse position

void CDasherViewSquare::DrawMouseLine(screenint mousex, screenint mousey) {

  int iCoordinateCount(GetCoordinateCount());

  myint *pCoordinates(new myint[iCoordinateCount]);

  int iType(GetCoordinates(iCoordinateCount, pCoordinates));

  if(iCoordinateCount == 1) {
    mousex = 0;
    mousey = pCoordinates[0];
  }
  else {
    mousex = pCoordinates[0];
    mousey = pCoordinates[1];
  }

  delete[] pCoordinates;

  myint x[2];
  myint y[2];

  // Start of line is the crosshair location

  x[0] = DasherModel()->DasherOX();
  y[0] = DasherModel()->DasherOY();

  // End of line is the mouse cursor location - note that we should
  // probably be using a chached value rather than computing this
  // separately to TapOn

  Input2Dasher(mousex, mousey, x[1], y[1], iType, DasherModel()->GetMode());

  // Actually plot the line

  if(GetBoolParameter(BP_COLOUR_MODE)) {
    DasherPolyline(x, y, 2, GetLongParameter(LP_LINE_WIDTH), 1);
  }
  else {
    DasherPolyline(x, y, 2, GetLongParameter(LP_LINE_WIDTH), -1);
  }
}

void CDasherViewSquare::DrawKeyboard() {
  CDasherScreen::point line[2];
  line[0].x = 200;
  line[0].y = 0;
  line[1].x = 200;
  line[1].y = CanvasY / 2;

  if(GetBoolParameter(BP_COLOUR_MODE)) {
    Screen()->Polyline(line, 2, 1, 6);
  }
  else {
    Screen()->Polyline(line, 2, 1);
  }

  line[0].x = 200;
  line[0].y = CanvasY / 2;
  line[1].x = 0;
  line[1].y = CanvasY / 2;

  if(GetBoolParameter(BP_COLOUR_MODE)) {
    Screen()->Polyline(line, 2, 1, 6);
  }
  else {
    Screen()->Polyline(line, 2, 1);
  }

  line[0].x = 200;
  line[0].y = CanvasY / 2;
  line[1].x = 200;
  line[1].y = CanvasY;

  if(GetBoolParameter(BP_COLOUR_MODE)) {
    Screen()->Polyline(line, 2, 1, 6);
  }
  else {
    Screen()->Polyline(line, 2, 1);
  }
}

void CDasherViewSquare::ResetSum() {
  m_ySum = 0;
}

void CDasherViewSquare::ResetSumCounter() {
  m_ySumCounter = 0;
}

void CDasherViewSquare::ResetYAutoOffset() {
  m_yAutoOffset = 0;
}

void CDasherViewSquare::ChangeScreen(CDasherScreen *NewScreen) {
  CDasherView::ChangeScreen(NewScreen);
  screenint Width = Screen()->GetWidth();
  screenint Height = Screen()->GetHeight();
  CanvasX = 9 * Width / 10;
  CanvasBorder = Width - CanvasX;
  CanvasY = Height;
  m_iScalingFactor = 100000000;
  SetScaleFactor();
}

int CDasherViewSquare::GetAutoOffset() const {
  return m_yAutoOffset;
}

/// Convert screen co-ordinates to dasher co-ordinates, possibly
/// involving a non-linear transformation for 1D mode, eyetracking
/// mode etc.
///
/// FIXME - lots of floating point arithmetic here

void CDasherViewSquare::screen2dasher(screenint imousex, screenint imousey, myint *idasherx, myint *idashery) const {
  bool eyetracker = GetBoolParameter(BP_EYETRACKER_MODE);
  // bool DasherRunning = DasherModel()->Paused();

  // Add the eyetracker autocalibration offset if necessary

  if(eyetracker)
    imousey += int (m_yAutoOffset);

  // Maybe this mousey tweak should take place earlier, elsewhere, and 
  // have a permanent effect on mousey rather than just local.

  //SUMMARY OF  Y autocallibrate additions: 
  // If autocallibrate  {
  //    tweak mousey right now before anyone looks at it.
  //    If dasher running {
  //        Adjust our tweak estimate
  //    }
  // }
  // end summary

  // If autocallibrate  {
  //    tweak mousey right now before anyone looks at it.
  //           NOTE: yAutoOffset should be set to zero ONCE when Dasher 
  //                 first started, then left alone. In principle, if 
  //                 someone moves their Dasher window from one locn to another
  //  then it might be reasonable to re-zero the offset. But don't.

  // Convert the Y mouse coordinate to one that's based on the canvas size
  double dashery = double (imousey * DasherModel()->DasherY() / CanvasY);

  // Convert the X mouse coordinate to one that's based on the canvas size 
  // - we want this the opposite way round to the mouse coordinate system, 
  // hence the fudging. ixmap gives us the X nonlinearity.        
  double x = ixmap(1.0 * (CanvasX - imousex) / CanvasX) * DasherModel()->DasherY();

  // Disable one-button mode for now.
  // if (eyetracker==true) { dashery=onebutton; }

  // If we're in standard mode, fudge things for the vertical acceleration
  if(GetBoolParameter(BP_NUMBER_DIMENSIONS) == false &&  GetBoolParameter(BP_EYETRACKER_MODE) == false) {
    dashery = m_ymap.unmap(myint(dashery));
    if(dashery > DasherModel()->DasherY()) {
      dashery = DasherModel()->DasherY();
    }
    if(dashery < 0) {
      dashery = 0;
    }
  }

  // The X and Y origins.
  myint dasherOX = DasherModel()->DasherOX();
  //cout << "dasherOX: " << dasherOX << endl;
  myint dasherOY = DasherModel()->DasherOY();
  // For Y co-ordinate changes. 
  // disty is the distance between y and centreline. 
  double disty = double (dasherOY) - dashery;
  //cout << "disty: " << disty << endl;

  // If we're in one-dimensional mode, make new x,y
  if(GetBoolParameter(BP_NUMBER_DIMENSIONS) == true) {
    //if (eyetracker==true && !(x<DasherModel()->DasherOX() && pow(pow(DasherModel()->DasherY()/2-dashery,2)+pow(x-DasherModel()->DasherOX(),2),0.5)>DasherModel()->DasherY()/2.5)) {
    //      *mousex=int(x);
    //      *mousey=int(dashery);
    //      return;
    //}

    double disty, circlesize, yfullrange, yforwardrange, angle, ellipse_eccentricity, ybackrange, yb;

    // The distance between the Y coordinate and the centreline in pixels
    disty = dasherOY - dashery;

    //              double rel_dashery=dashery+1726;
    //      double rel_dasherOY=dasherOY+1726;
    //cout << "x: " << x << endl;
    //cout << "dashery: " << rel_dashery << endl << endl;

    // The radius of the circle transcribed by the one-dimensional mapping
    circlesize = DasherModel()->DasherY() / 2.5;
    yforwardrange = DasherModel()->DasherY() / 1.6;
    yfullrange = yforwardrange * 1.6;
    ybackrange = yfullrange - yforwardrange;
    ellipse_eccentricity = 6;

    if(disty > yforwardrange) {
      // If the distance between y-coord and centreline is > radius,
      // we should be going backwards, off the top.
      yb = (disty - yforwardrange) / ybackrange;

      if(yb > 1) {
        x = 0;
        dashery = double (dasherOY);
      }
      else {
        angle = (yb * 3.14159) * (yb + (1 - yb) * (ybackrange / yforwardrange / ellipse_eccentricity));
        x = (-sin(angle) * circlesize / 2) * ellipse_eccentricity;
        dashery = myint(-(1 + cos(angle)) * circlesize / 2 + dasherOY);
      }
    }
    else if(disty < -(yforwardrange)) {
      // Backwards, off the bottom.
      yb = -(disty + yforwardrange) / ybackrange;

      if(yb > 1) {
        x = 0;
        dashery = double (dasherOY);
      }
      else {
        angle = (yb * 3.14159) * (yb + (1 - yb) * (ybackrange / yforwardrange / ellipse_eccentricity));

        x = (-sin(angle) * circlesize / 2) * ellipse_eccentricity;
        dashery = myint((1 + cos(angle)) * circlesize / 2 + dasherOY);
      }
    }

    else {
      angle = ((disty * 3.14159 / 2) / yforwardrange);
      x = cos(angle) * circlesize;
      dashery = myint(-sin(angle) * circlesize + dasherOY);
    }
    x = dasherOX - x;
  }
  else if(eyetracker == true) {

    myint dasherOX = DasherModel()->DasherOX();
    //cout << "dasherOX: " << dasherOX << endl; 
    myint dasherOY = DasherModel()->DasherOY();

    // X co-ordinate changes. 
    double double_x = (x / dasherOX);
    double double_y = -((dashery - dasherOY) / (double)(dasherOY));

    double xmax_y = xmax(double_x, double_y);

    if(double_x < xmax_y) {
      double_x = xmax_y;
    }

    x = dasherOX * double_x;

    // Finished x-coord changes.

    double repulsionparameter = 0.5;
    dashery = dasherOY - (1.0 + double_y * double_y * repulsionparameter) * disty;
  }
  /* 
     // Finish the yautocallibrate
     //    If dasher running, adjust our tweak estimate
     if(!DasherRunning==true) {
     CDasherView::yFilterTimescale = 60;
     CDasherView::ySum += disty; 
     CDasherView::ySumCounter++; 

     CDasherView::ySigBiasPercentage=50;
     CDasherView::ySigBiasPixels = CDasherView::ySigBiasPercentage * DasherModel()->DasherY() / 100;

     // FIXME: screen2dasher appears to be being called thrice per frame.
     // I don't know why.  
     CDasherView::ySigBiasPixels*=3;

     cout <<"ySum: " << CDasherView::ySum << " | ySigBiasPixels: " << CDasherView::ySigBiasPixels << " | disty: " << disty << " | yAutoOffset: " << CDasherView::yAutoOffset << endl;

     if (CDasherView::ySumCounter > CDasherView::yFilterTimescale) {
     CDasherView::ySumCounter = 0;

     // 'Conditions A', as specified by DJCM.  Only make the auto-offset
     // change if we're past the significance boundary.

     if (CDasherView::ySum > CDasherView::ySigBiasPixels || CDasherView::ySum < -CDasherView::ySigBiasPixels) {
     if (CDasherView::ySum > CDasherView::yFilterTimescale)
     CDasherView::yAutoOffset--; 
     else if (CDasherView::ySum < -CDasherView::yFilterTimescale)
     CDasherView::yAutoOffset++;
     CDasherView::ySum = 0;
     }
     }
     }
   */
  *idasherx = myint(x);

  *idashery = myint(dashery);
}

void CDasherViewSquare::AutoCalibrate(screenint *mousex, screenint *mousey) {
  double dashery = double (*mousey) * double (DasherModel()->DasherY()) / double (CanvasY);
  myint dasherOY = DasherModel()->DasherOY();
  double disty = double (dasherOY) - dashery;
  bool DasherRunning = GetBoolParameter(BP_DASHER_PAUSED);

  if(!DasherRunning == true) {
    m_yFilterTimescale = 20;
    m_ySum += (int)disty;
    m_ySumCounter++;

    m_ySigBiasPercentage = 50;
    m_ySigBiasPixels = m_ySigBiasPercentage * DasherModel()->DasherY() / 100;

    //cout << "yAutoOffset: " << CDasherView::yAutoOffset << endl;

    if(m_ySumCounter > m_yFilterTimescale) {
      m_ySumCounter = 0;

      // 'Conditions A', as specified by DJCM.  Only make the auto-offset
      // change if we're past the significance boundary.

      if(m_ySum > m_ySigBiasPixels || m_ySum < -m_ySigBiasPixels) {
        if(m_ySum > m_yFilterTimescale) {
          m_yAutoOffset--;
        }
        else if(m_ySum < -m_yFilterTimescale)
          m_yAutoOffset++;

        m_ySum = 0;
      }
    }

    //*mousey=int(dashery);
  }
}

void CDasherViewSquare::DrawGameModePointer() {

  myint loc = DasherModel()->GetGameModePointerLoc();

  if(loc == myint(INT64_MIN))
    return;

  if(loc > DasherModel()->DasherY())
    DasherDrawCentredRectangle(-50, DasherModel()->DasherY(), 5, 135, Opts::ColorSchemes(Objects), false);

  else if(loc < 0)
    DasherDrawCentredRectangle(-50, 0, 5, 135, Opts::ColorSchemes(Objects), false);

  else
    DasherDrawCentredRectangle(-50, loc, 7, 135, Opts::ColorSchemes(Objects), false);

}

bool CDasherViewSquare::HandleStartOnMouse(int iTime) {

  screenint mousex, mousey;

  int iCoordinateCount(GetCoordinateCount());

  myint *pCoordinates(new myint[iCoordinateCount]);

  int iType(GetCoordinates(iCoordinateCount, pCoordinates));

  if(iCoordinateCount == 1) {
    mousex = 0;
    mousey = pCoordinates[0];
  }
  else {
    mousex = pCoordinates[0];
    mousey = pCoordinates[1];
  }

  delete pCoordinates;

//   if(GetBoolParameter(BP_AUTOCALIBRATE)) {
//     AutoCalibrate(&mousex, &mousey);
//   }

  myint iDasherX;
  myint iDasherY;

  // Convert the input co-ordinates to dasher co-ordinates

  Input2Dasher(mousex, mousey, iDasherX, iDasherY, iType, DasherModel()->GetMode());

  screenint iNewScreenX;
  screenint iNewScreenY;

  Dasher2Screen( iDasherX, iDasherY, iNewScreenX, iNewScreenY );

  int iBoxMax(-1);
  int iBoxMin(0);

  if(GetLongParameter(LP_MOUSE_POS_BOX) == 1) {
    iBoxMax = Screen()->GetHeight() / 2 - (int)GetLongParameter(LP_MOUSEPOSDIST) + 50;
    iBoxMin = iBoxMax - 100;
  }
  else if(GetLongParameter(LP_MOUSE_POS_BOX) == 2) {
    iBoxMin = Screen()->GetHeight() / 2 + (int)GetLongParameter(LP_MOUSEPOSDIST) - 50;
    iBoxMax = iBoxMin + 100;
  }

  if((iNewScreenY >= iBoxMin) && (iNewScreenY <= iBoxMax)) {
    if(!bInBox) {
      iBoxEntered = iTime;
    }
    else {
      if(iTime - iBoxEntered > 2000) {

        iBoxStart = iTime;

        if(GetLongParameter(LP_MOUSE_POS_BOX) == 1)
          SetLongParameter(LP_MOUSE_POS_BOX, 2);
        else if(GetLongParameter(LP_MOUSE_POS_BOX) == 2) {
          SetLongParameter(LP_MOUSE_POS_BOX, -1);
          return true;
        }
      }
    }

    bInBox = true;
  }
  else {
    if((GetLongParameter(LP_MOUSE_POS_BOX) == 2) && (iTime - iBoxStart > 2000))
      SetLongParameter(LP_MOUSE_POS_BOX, 1);

    bInBox = false;
  }

  return false;

}

