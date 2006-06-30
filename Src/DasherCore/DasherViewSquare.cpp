// DasherViewSquare.cpp
//
// Copyright (c) 2001-2004 David Ward

// TODO - there's no real reason to distinguish between groups and nodes here - they're all just boxes to be rendered, with the distinction being at the level of the alphabet manager

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

// FIXME - duplicated 'mode' code throught - needs to be fixed (actually, mode related stuff, Input2Dasher etc should probably be at least partially in some other class)

CDasherViewSquare::CDasherViewSquare(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherScreen *DasherScreen)
 : CDasherView(pEventHandler, pSettingsStore, DasherScreen) {

  // TODO - AutoOffset should be part of the eyetracker input filter
  // Make sure that the auto calibration is set to zero berfore we start
  m_yAutoOffset = 0;

  m_pDelayDraw = new CDelayedDraw();
  ChangeScreen(DasherScreen);

  // TODO - Make these parameters
  // tweak these if you know what you are doing
  m_dXmpa = 0.2;                // these are for the x non-linearity
  m_dXmpb = 0.5;
  m_dXmpc = 0.9;
  m_dXmpd = 0.5;                // slow X movement when accelerating Y

  m_ymap = Cymap((myint)GetLongParameter(LP_MAX_Y));

  m_bVisibleRegionValid = false;
}

CDasherViewSquare::~CDasherViewSquare() {
  if (m_pDelayDraw != NULL) {
    delete m_pDelayDraw;
    m_pDelayDraw = NULL;
  }
}

void CDasherViewSquare::HandleEvent(Dasher::CEvent *pEvent) {
  // Let the parent class do its stuff
  CDasherView::HandleEvent(pEvent);

  // And then interpret events for ourself
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    switch (pEvt->m_iParameter) {
    case LP_REAL_ORIENTATION:
      m_bVisibleRegionValid = false;
      break;
    default:
      break;
    }
  }
}

void CDasherViewSquare::RenderNodes(CDasherNode *pRoot, myint iRootMin, myint iRootMax, std::vector<CDasherNode *> &vNodeList, std::vector<CDasherNode *> &vDeleteList, myint *iGamePointer) {

  DASHER_ASSERT(pRoot != 0);

  Screen()->Blank();

  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);
 
  RecursiveRender(pRoot, iRootMin, iRootMax, iDasherMaxX, vNodeList, vDeleteList, iGamePointer, true);

  // DelayDraw the text nodes
  m_pDelayDraw->Draw(Screen());

  Crosshair((myint)GetLongParameter(LP_OX));  // add crosshair
}


int CDasherViewSquare::RecursiveRender(CDasherNode *pRender, myint y1, myint y2, int mostleft, std::vector<CDasherNode *> &vNodeList, std::vector<CDasherNode *> &vDeleteList, myint *iGamePointer, bool bDraw) {
  DASHER_ASSERT_VALIDPTR_RW(pRender);

  if(bDraw && !RenderNode(pRender->Colour(), pRender->ColorScheme(), y1, y2, mostleft, pRender->m_strDisplayText, pRender->m_bShove)) {
    vDeleteList.push_back(pRender);
    pRender->Kill();
    return 0;
  }
  
  if(pRender->ChildCount() == 0) {
    vNodeList.push_back(pRender);
    return 0;
  }

  if(pRender->GetGame())
    *iGamePointer = (y1 + y2) / 2;
  
  // Render groups
  RenderGroups(pRender, y1, y2, mostleft);

  // Render children  
  int norm = (myint)GetLongParameter(LP_NORMALIZATION);

  CDasherNode::ChildMap::const_iterator i;
  for(i = pRender->GetChildren().begin(); i != pRender->GetChildren().end(); i++) {
    CDasherNode *pChild = *i;
    
    myint Range = y2 - y1;
    myint newy1 = y1 + (Range * pChild->Lbnd()) / norm;
    myint newy2 = y1 + (Range * pChild->Hbnd()) / norm;
    
    // FIXME - make the threshold a parameter
    
    if((newy2 - newy1 > 50) || (pChild->Alive())) {
      pChild->Alive(true);
      RecursiveRender(pChild, newy1, newy2, mostleft, vNodeList, vDeleteList, iGamePointer, true);
    }
    else if(pRender->GetGame()) {
      RecursiveRender(pChild, newy1, newy2, mostleft, vNodeList, vDeleteList, iGamePointer, false);
    }
  }
  
  return 1;
}

void CDasherViewSquare::RenderGroups(CDasherNode *Render, myint y1, myint y2, int mostleft) {
  SGroupInfo *pCurrentGroup(Render->m_pBaseGroup);

  while(pCurrentGroup) {
    RecursiveRenderGroups(pCurrentGroup, Render, y1, y2, mostleft);
    pCurrentGroup = pCurrentGroup->pNext;
  }
}

void CDasherViewSquare::RecursiveRenderGroups(SGroupInfo *pCurrentGroup, CDasherNode *pNode, myint y1, myint y2, int mostleft) {
  
  if(pCurrentGroup->bVisible) {
    myint range = y2 - y1;
    
    int lower(pCurrentGroup->iStart);
    int upper(pCurrentGroup->iEnd);
    
    myint lbnd = pNode->Children()[lower]->Lbnd();
    myint hbnd = pNode->Children()[upper - 1]->Hbnd();
    
    myint newy1 = y1 + (range * lbnd) / (int)GetLongParameter(LP_NORMALIZATION);
    myint newy2 = y1 + (range * hbnd) / (int)GetLongParameter(LP_NORMALIZATION);
    
    RenderNode(pCurrentGroup->iColour, Opts::Groups, newy1, newy2, mostleft, pCurrentGroup->strLabel, true);
  }
  
  // Iterate through child groups
  SGroupInfo *pCurrentChild(pCurrentGroup->pChild);

  while(pCurrentChild) {
    RecursiveRenderGroups(pCurrentChild, pNode, y1, y2, mostleft);
    pCurrentChild = pCurrentChild->pNext;
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



int CDasherViewSquare::RenderNode(const int Color, Opts::ColorSchemes ColorScheme, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove) {

  // Commenting because click mode occasionally fails this assert.
  // I don't know why.  -- cjb.
  if (!(y2 >= y1)) { return 1; }

  // TODO - Get sensibel limits here (to allow for non-linearities)
  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  screenint s1, s2;
  // TODO - use new versions of functions
  Cint32 iSize = dashery2screen(y1, y2, s1, s2);

  // Actual height in pixels
  Cint32 iHeight = Cint32((Cint32) (iSize * CanvasY) / (Cint32) GetLongParameter(LP_MAX_Y));

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
    int iDasherY((myint)GetLongParameter(LP_MAX_Y));

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

  if( sDisplayText.size() > 0 )
    DasherDrawText(iDasherAnchorX, y1, iDasherAnchorX, y2, sDisplayText, mostleft, bShove);

  return 1;
}

bool CDasherViewSquare::IsNodeVisible(myint y1, myint y2) {
 
  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;

  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  return (y1 > iDasherMinY) || (y2 < iDasherMaxY ) || (y2-y1 < iDasherMaxX);
}

/// Convert screen co-ordinates to dasher co-ordinates. This doesn't
/// include the nonlinear mapping for eyetracking mode etc - it is
/// just the inverse of the mapping used to calculate the screen
/// positions of boxes etc.

void CDasherViewSquare::Screen2Dasher(screenint iInputX, screenint iInputY, myint &iDasherX, myint &iDasherY, bool b1D, bool bNonlinearity) {

  // Things we're likely to need:

  myint iDasherWidth = (myint)GetLongParameter(LP_MAX_Y);
  myint iDasherHeight = (myint)GetLongParameter(LP_MAX_Y);

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
  iDasherX = myint(ixmap(iDasherX / static_cast < double >(GetLongParameter(LP_MAX_Y))) * (myint)GetLongParameter(LP_MAX_Y));
  iDasherY = m_ymap.unmap(iDasherY);
  }
#endif
}

void CDasherViewSquare::SetScaleFactor( void )
{
  myint iDasherWidth = (myint)GetLongParameter(LP_MAX_Y);
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

  iLRScaleFactorX = myint(std::max(std::min(dLRHScaleFactor, dLRVScaleFactor), dLRHScaleFactor / 4.0) * m_iScalingFactor);
  iLRScaleFactorY = myint(std::max(std::min(dLRHScaleFactor, dLRVScaleFactor), dLRVScaleFactor / 4.0) * m_iScalingFactor);
  iTBScaleFactorX = myint(std::max(std::min(dTBHScaleFactor, dTBVScaleFactor), dTBVScaleFactor / 4.0) * m_iScalingFactor);
  iTBScaleFactorY = myint(std::max(std::min(dTBHScaleFactor, dTBVScaleFactor), dTBHScaleFactor / 4.0) * m_iScalingFactor);
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
  iDasherX = myint(xmap(iDasherX / static_cast < double >(GetLongParameter(LP_MAX_Y))) * (myint)GetLongParameter(LP_MAX_Y));
  iDasherY = m_ymap.map(iDasherY);
#endif

  // Things we're likely to need:

  myint iDasherWidth = (myint)GetLongParameter(LP_MAX_Y);
  myint iDasherHeight = (myint)GetLongParameter(LP_MAX_Y);

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
}

void CDasherViewSquare::VisibleRegion( myint &iDasherMinX, myint &iDasherMinY, myint &iDasherMaxX, myint &iDasherMaxY ) {

  if(!m_bVisibleRegionValid) {

    int eOrientation( GetLongParameter(LP_REAL_ORIENTATION) );
    
    switch( eOrientation ) {
    case Dasher::Opts::LeftToRight:
      Screen2Dasher(Screen()->GetWidth(),0,m_iDasherMinX,m_iDasherMinY,false,true);
      Screen2Dasher(0,Screen()->GetHeight(),m_iDasherMaxX,m_iDasherMaxY,false,true);
      break;
    case Dasher::Opts::RightToLeft:
      Screen2Dasher(0,0,m_iDasherMinX,m_iDasherMinY,false,true);
      Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),m_iDasherMaxX,m_iDasherMaxY,false,true);
      break;
    case Dasher::Opts::TopToBottom:
      Screen2Dasher(0,Screen()->GetHeight(),m_iDasherMinX,m_iDasherMinY,false,true);
      Screen2Dasher(Screen()->GetWidth(),0,m_iDasherMaxX,m_iDasherMaxY,false,true);
      break;
    case Dasher::Opts::BottomToTop:
      Screen2Dasher(0,0,m_iDasherMinX,m_iDasherMinY,false,true);
      Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),m_iDasherMaxX,m_iDasherMaxY,false,true);
      break;
    }
    
    m_bVisibleRegionValid = true;
  }

  iDasherMinX = m_iDasherMinX;
  iDasherMaxX = m_iDasherMaxX;
  iDasherMinY = m_iDasherMinY;
  iDasherMaxY = m_iDasherMaxY;
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

  //  std::cout << "iType: " << iType << " iMode: " << iMode << std::endl;

  switch (iType) {
  case 0:
    // Raw secreen coordinates

    // TODO - autocalibration should be at the level of the eyetracker filter
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

  // TODO: Check that this is still doing something vaguely sensible - I think it isn't

  if(iMode == 1 ) {
    if( GetLongParameter(LP_YSCALE) > 0 ) {
      
      double dYScale;
      
      int eOrientation(GetLongParameter(LP_REAL_ORIENTATION));
      
      if(( eOrientation == Dasher::Opts::LeftToRight ) || ( eOrientation == Dasher::Opts::RightToLeft ))
	dYScale = Screen()->GetHeight() / static_cast<double>(GetLongParameter(LP_YSCALE));
      else
	dYScale = Screen()->GetWidth() / static_cast<double>(GetLongParameter(LP_YSCALE));
      
      iDasherY = myint((iDasherY - (myint)GetLongParameter(LP_MAX_Y)/2) * dYScale + (myint)GetLongParameter(LP_MAX_Y)/2);
    }
  }
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
				     myint &iDasherX, myint &iDasherY,
                                     Dasher::VECTOR_SYMBOL_PROB* pAdded, 
                                     int* pNumDeleted) {

  // FIXME - rename this something more appropriate (all this really should do is convert the coordinates)

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
  if(GetBoolParameter(BP_AUTOCALIBRATE) && (GetStringParameter(SP_INPUT_FILTER) == "Eyetracker Mode")) {
    AutoCalibrate(&mousex, &mousey);
  }


  // TODO: Mode probably isn't being used any more

  // Convert the input co-ordinates to dasher co-ordinates

  int mode;
  
  if(GetBoolParameter(BP_NUMBER_DIMENSIONS))
    mode = 1;
  else if(GetBoolParameter(BP_EYETRACKER_MODE))
    mode = 2;
  else
    mode = 0;
 
  Input2Dasher(mousex, mousey, iDasherX, iDasherY, iType, mode);
  m_iDasherXCache = iDasherX;
  m_iDasherYCache = iDasherY;

  // Request an update at the calculated co-ordinates


  // Cache the Dasher Co-ordinates, so we can use them later for things like drawing the mouse position
#ifndef WITH_MAEMO
  // FIXME
  //  iDasherX = myint(xmap(iDasherX / static_cast < double >(GetLongParameter(LP_MAX_Y))) * GetLongParameter(LP_MAX_Y));
  // iDasherY = m_ymap.map(iDasherY);
#endif
}

void CDasherViewSquare::NewDrawGoTo(myint iDasherMin, myint iDasherMax, bool bActive) {
  myint iHeight(iDasherMax - iDasherMin);

  int iColour;
  int iWidth;

  if(bActive) {
    iColour = 1;
    iWidth = 3;
  }
  else {
    iColour = 2;
    iWidth = 1;
  }

  CDasherScreen::point p[4];

  Dasher2Screen( 0, iDasherMin, p[0].x, p[0].y);
  Dasher2Screen( iHeight, iDasherMin, p[1].x, p[1].y);
  Dasher2Screen( iHeight, iDasherMax, p[2].x, p[2].y);
  Dasher2Screen( 0, iDasherMax, p[3].x, p[3].y);

  Screen()->Polyline(p, 4, iWidth, iColour);
}

// TODO: Autocalibration should be in the eyetracker filter class

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
  m_bVisibleRegionValid = false;
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

void CDasherViewSquare::AutoCalibrate(screenint *mousex, screenint *mousey) {
  return;
  // Y value in dasher coordinates
  double dashery = double (*mousey) * double ((myint)GetLongParameter(LP_MAX_Y)) / double (CanvasY);

  // Origin in dasher coordinates
  myint dasherOY = (myint)GetLongParameter(LP_OY);

  // Distance above origin in dasher coordinates
  double disty = double (dasherOY) - dashery;

  // Whether we're paused or not (sensible choice of name!)
  bool DasherRunning = GetBoolParameter(BP_DASHER_PAUSED);

  if(!DasherRunning == true) {
    // Only update every this number of timesteps
    m_yFilterTimescale = 20;
    m_ySum += (int)disty;
    m_ySumCounter++;

    m_ySigBiasPercentage = 50;

    // Despite the name, this is actually measured in dasher coordinates
    m_ySigBiasPixels = m_ySigBiasPercentage * (myint)GetLongParameter(LP_MAX_Y) / 100;


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

// TODO - should be elsewhere

void CDasherViewSquare::DrawGameModePointer(myint iPosition) {

  // FIXME - reimplement

//   myint loc = DasherModel()->GetGameModePointerLoc();

//   if(loc == myint(INT64_MIN))
//     return;

//   if(loc > GetLongParameter(LP_MAX_Y))
//     DasherDrawCentredRectangle(-50, GetLongParameter(LP_MAX_Y), 5, 135, Opts::ColorSchemes(Objects), false);

//   else if(loc < 0)
//     DasherDrawCentredRectangle(-50, 0, 5, 135, Opts::ColorSchemes(Objects), false);

//   else
     DasherDrawCentredRectangle(-50, iPosition, 7, 135, Opts::ColorSchemes(Objects), false);

}
