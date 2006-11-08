// DasherViewSquare.cpp
//
// Copyright (c) 2001-2004 David Ward

// TODO - there's no real reason to distinguish between groups and nodes here - they're all just boxes to be rendered, with the distinction being at the level of the alphabet manager

#include "../Common/Common.h"

#ifdef _WIN32
#include "..\Win32\Common\WinCommon.h"
#endif

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
  //  m_yAutoOffset = 0;

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

  // TODO: We need an overhall of the node creation/deletion logic - make sure that we only maintain the minimum number of nodes which are actually needed.
  // This is especially true at the moment in Game mode, which feels very sluggish. Node creation also feels slower in Windows than Linux, especially
  // if many nodes are created at once (eg untrained Hiragana)

  ++m_iRenderCount;

  if(bDraw && !RenderNode(pRender->Colour(), y1, y2, mostleft, pRender->m_strDisplayText, pRender->m_bShove) && !(pRender->GetGame())) {
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
    
    RenderNode(pCurrentGroup->iColour, newy1, newy2, mostleft, pCurrentGroup->strLabel, true);
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


bool CDasherViewSquare::IsNodeVisible(myint y1, myint y2) {
  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;

  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  return ((y2 - y1) < iDasherMaxX) || ((y1 > iDasherMinY) && (y2 < iDasherMaxY));
}


int CDasherViewSquare::RenderNode(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove) {

  // Commenting because click mode occasionally fails this assert.
  // I don't know why.  -- cjb.
  if (!(y2 >= y1)) { return 1; }

  // TODO - Get sensible limits here (to allow for non-linearities)
  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;

  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  screenint iScreenX1;
  screenint iScreenY1;
  screenint iScreenX2;
  screenint iScreenY2;
  
  Dasher2Screen(0, std::max(y1, iDasherMinY), iScreenX1, iScreenY1);
  Dasher2Screen(0, std::min(y2, iDasherMaxY), iScreenX2, iScreenY2);

  Cint32 iHeight = std::max(iScreenY2 - iScreenY1, 0);

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
    DasherDrawRectangle(std::min(iDasherSize,iDasherMaxX), std::min(y2,iDasherMaxY), 0, std::max(y1,iDasherMinY), Color, -1, Nodes1, GetBoolParameter(BP_OUTLINE_MODE), true, 1);
  }
  else {
    // TODO: Reimplement

//     int iDasherY((myint)GetLongParameter(LP_MAX_Y));

//     int iSpacing(iDasherY / 128);       // FIXME - assuming that this is an integer below

//     int iXStart = 0;

//     switch (iTruncationType) {
//     case 1:
//       iXStart = iSize - iSize * iTruncation / 200;
//       break;
//     case 2:
//       iXStart = iSize - iSize * iTruncation / 100;
//       break;
//     }

//     int iTipMin((y2 - y1) * iTruncation / (200) + y1);
//     int iTipMax(y2 - (y2 - y1) * iTruncation / (200));

//     int iLowerMin(((y1 + 1) / iSpacing) * iSpacing);
//     int iLowerMax(((iTipMin - 1) / iSpacing) * iSpacing);

//     int iUpperMin(((iTipMax + 1) / iSpacing) * iSpacing);
//     int iUpperMax(((y2 - 1) / iSpacing) * iSpacing);

//     if(iLowerMin < 0)
//       iLowerMin = 0;

//     if(iLowerMax < 0)
//       iLowerMax = 0;

//     if(iUpperMin < 0)
//       iUpperMin = 0;

//     if(iUpperMax < 0)
//       iUpperMax = 0;

//     if(iLowerMin > iDasherY)
//       iLowerMin = iDasherY;

//     if(iLowerMax > iDasherY)
//       iLowerMax = iDasherY;

//     if(iUpperMin > iDasherY)
//       iUpperMin = iDasherY;

//     if(iUpperMax > iDasherY)
//       iUpperMax = iDasherY;

//     while(iLowerMin < y1)
//       iLowerMin += iSpacing;

//     while(iLowerMax > iTipMin)
//       iLowerMax -= iSpacing;

//     while(iUpperMin < iTipMax)
//       iUpperMin += iSpacing;

//     while(iUpperMax > y2)
//       iUpperMax -= iSpacing;

//     int iLowerCount((iLowerMax - iLowerMin) / iSpacing + 1);
//     int iUpperCount((iUpperMax - iUpperMin) / iSpacing + 1);

//     if(iLowerCount < 0)
//       iLowerCount = 0;

//     if(iUpperCount < 0)
//       iUpperCount = 0;

//     int iTotalCount(iLowerCount + iUpperCount + 6);

//     myint *x = new myint[iTotalCount];
//     myint *y = new myint[iTotalCount];

//     // Weird duplication here is to make truncated squares possible too

//     x[0] = 0;
//     y[0] = y1;
//     x[1] = iXStart;
//     y[1] = y1;

//     x[iLowerCount + 2] = iDasherSize;
//     y[iLowerCount + 2] = iTipMin;
//     x[iLowerCount + 3] = iDasherSize;
//     y[iLowerCount + 3] = iTipMax;

//     x[iTotalCount - 2] = iXStart;
//     y[iTotalCount - 2] = y2;
//     x[iTotalCount - 1] = 0;
//     y[iTotalCount - 1] = y2;

//     for(int i(0); i < iLowerCount; ++i) {
//       x[i + 2] = (iLowerMin + i * iSpacing - y1) * (iDasherSize - iXStart) / (iTipMin - y1) + iXStart;
//       y[i + 2] = iLowerMin + i * iSpacing;
//     }

//     for(int j(0); j < iUpperCount; ++j) {
//       x[j + iLowerCount + 4] = (y2 - (iUpperMin + j * iSpacing)) * (iDasherSize - iXStart) / (y2 - iTipMax) + iXStart;
//       y[j + iLowerCount + 4] = iUpperMin + j * iSpacing;
//     }

//     DasherPolygon(x, y, iTotalCount, Color);

//     delete x;
//     delete y;
  }

  myint iDasherAnchorX(iDasherSize);

  if( sDisplayText.size() > 0 )
    DasherDrawText(iDasherAnchorX, y1, iDasherAnchorX, y2, sDisplayText, mostleft, bShove);

  return 1;
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
  // TODO: Change output parameters to pointers and allow NULL to mean
  // 'I don't care'. Need to be slightly careful about this as it will
  // require a slightly more sophisticated caching mechanism

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

int CDasherViewSquare::GetCoordinates(unsigned long Time, myint &iDasherX, myint &iDasherY) {

  // FIXME - Actually turn autocalibration on and off!
  // FIXME - AutoCalibrate should use Dasher co-ordinates, not raw mouse co-ordinates?
  // FIXME - Have I broken this by moving it before the offset is applied?
  // FIXME - put ymap stuff back in 
  // FIXME - optimise this

  int iCoordinateCount(GetCoordinateCount());

  myint *pCoordinates(new myint[iCoordinateCount]);

  int iType(GetInputCoordinates(iCoordinateCount, pCoordinates));

  screenint mousex;
  screenint mousey;

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
  //  if(GetBoolParameter(BP_AUTOCALIBRATE) && (GetStringParameter(SP_INPUT_FILTER) == "Eyetracker Mode")) {
    //    AutoCalibrate(&mousex, &mousey);
  //  }


  // TODO: Mode probably isn't being used any more

  // Convert the input co-ordinates to dasher co-ordinates

  //  Input2Dasher(mousex, mousey, iDasherX, iDasherY);
  if(iType == 0)
    Screen2Dasher(mousex, mousey, iDasherX, iDasherY, false, true );
  else {
    iDasherX = mousex;
    iDasherY = mousey;
  }

//   m_iDasherXCache = iDasherX;
//   m_iDasherYCache = iDasherY;

  // Request an update at the calculated co-ordinates


  // Cache the Dasher Co-ordinates, so we can use them later for things like drawing the mouse position
#ifndef WITH_MAEMO
  // FIXME
  //  iDasherX = myint(xmap(iDasherX / static_cast < double >(GetLongParameter(LP_MAX_Y))) * GetLongParameter(LP_MAX_Y));
  // iDasherY = m_ymap.map(iDasherY);
#endif

  return iType;
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

// TODO - should be elsewhere

void CDasherViewSquare::DrawGameModePointer(myint iPosition) {
  // TODO: Horrible code duplication here


  // Circular version:

  double dOffset = (iPosition - 2048) / 2048.0;

  double dCos = (1 - pow(dOffset,2.0)) / (1 + pow(dOffset,2.0));
  double dSin = 2 * dOffset / (1 + pow(dOffset,2.0));

  {
    CDasherScreen::point p[2];

    myint iDasherX;
    myint iDasherY;

    iDasherX = 2048;
    iDasherY = 2048;
    Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);

    iDasherX = static_cast<int>(2048 - 500 * dCos);
    iDasherY = static_cast<int>(2048 + 500 * dSin);
    Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);

    Screen()->Polyline(p, 2, 4, 136); 
  }

  int iOtherPosition = static_cast<int>((1 - (1 / dOffset)) * 2048);

  int iRadius = abs((int)(iPosition - iOtherPosition)) / 2;
  int iCentre = (iPosition + iOtherPosition) / 2;

  {
    CDasherScreen::point p[2];

    myint iDasherX;
    myint iDasherY;

    iDasherX = 0;
    iDasherY = iCentre;
    Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);

    iDasherX = 0;
    iDasherY = iCentre + iRadius;
    Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);

    Screen()->DrawCircle(p[0].x, p[0].y, abs(p[1].y - p[0].y), 136, 0, 1, false);
  }

  if(iPosition > (myint)GetLongParameter(LP_MAX_Y)) {
     CDasherScreen::point p[2];
  
  myint iDasherX;
  myint iDasherY;
  
  iDasherX = -400;
  iDasherY = GetLongParameter(LP_MAX_Y);
  
  Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -400;
  iDasherY = GetLongParameter(LP_MAX_Y) - 500;
  
  Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  Screen()->Polyline(p, 2, 1, 2);

   iDasherX = -300;
  iDasherY = GetLongParameter(LP_MAX_Y) - 100;
  
  Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  Screen()->Polyline(p, 2, 1, 2);

   iDasherX = -500;
  iDasherY = GetLongParameter(LP_MAX_Y) - 100;
  
  Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  Screen()->Polyline(p, 2, 1, 2);

  }
  else if(iPosition < 0) {
         CDasherScreen::point p[2];
  
  myint iDasherX;
  myint iDasherY;
  
  iDasherX = -400;
  iDasherY = 0;
  
  Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -400;
  iDasherY = 500;
  
  Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  Screen()->Polyline(p, 2, 1, 2);

   iDasherX = -300;
  iDasherY = 100;
  
  Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  Screen()->Polyline(p, 2, 1, 2);

   iDasherX = -500;
  iDasherY = 100;
  
  Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  Screen()->Polyline(p, 2, 1, 2);
  }
  else {
 CDasherScreen::point p[2];
  
  myint iDasherX;
  myint iDasherY;
  
  iDasherX = -200;
  iDasherY = iPosition;
  
  Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = iPosition;
  
  Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  Screen()->Polyline(p, 2, 1, 2);

   iDasherX = -300;
  iDasherY = iPosition + 100;
  
  Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  Screen()->Polyline(p, 2, 1, 2);

   iDasherX = -300;
  iDasherY = iPosition - 100;
  
  Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  Screen()->Polyline(p, 2, 1, 2);
   }

}
