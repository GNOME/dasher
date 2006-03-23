// DasherView.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherView.h"
#include "Event.h"
#include "EventHandler.h"
#include "DasherModel.h"
#include "DasherInput.h"

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

/////////////////////////////////////////////////////////////////////////////

CDasherView::CDasherView(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherScreen *DasherScreen)
:CDasherComponent(pEventHandler, pSettingsStore), m_pScreen(DasherScreen), m_pInput(0) {
}

void CDasherView::HandleEvent(Dasher::CEvent *pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    switch (pEvt->m_iParameter) {
    default:
      break;
    }
  }
};

/////////////////////////////////////////////////////////////////////////////

void CDasherView::ChangeScreen(CDasherScreen *NewScreen) {
  m_pScreen = NewScreen;
}

/////////////////////////////////////////////////////////////////////////////

bool CDasherView::Render(CDasherNode *pRoot, myint iRootMin, myint iRootMax, std::vector<CDasherNode *> &vNodeList, std::vector<CDasherNode *> &vDeleteList, bool bRedrawDisplay) {

  bool bDidSomething(false);    // Have we actually done any drawing

  if(bRedrawDisplay) {
    Screen()->SendMarker(0);     // Start of 'dasher field'
    RenderNodes(pRoot, iRootMin, iRootMax, vNodeList, vDeleteList);
    bDidSomething = true;
  }

  Screen()->SendMarker(1);       // Start of 'decoration'
  return bDidSomething;
}

int CDasherView::GetCoordinateCount() {
  // TODO: Do we really need support for co-ordinate counts other than 2?
  if(m_pInput)
    return m_pInput->GetCoordinateCount();
  else
    return 0;
}

int CDasherView::GetCoordinates(int iN, myint * pCoordinates) {
  if(m_pInput)
    return m_pInput->GetCoordinates(iN, pCoordinates);
  else
    return 0;
}

void CDasherView::SetInput(CDasherInput * _pInput) {
  // TODO: Is it sensible to make this responsible for the input
  // device - I guess it makes sense for now

  DASHER_ASSERT_VALIDPTR_RW(_pInput);

  // Don't delete the old input class; whoever is calling this method
  // might want to keep several Input class instances around and
  // change which one is currently driving dasher without deleting any

  m_pInput = _pInput;

  // Tell the new object about maximum values

  myint iMaxCoordinates[2];

  iMaxCoordinates[0] = GetLongParameter(LP_MAX_Y);
  iMaxCoordinates[1] = GetLongParameter(LP_MAX_Y);

  m_pInput->SetMaxCoordinates(2, iMaxCoordinates);
}

void CDasherView::Display() {
  m_pScreen->Display();
}

/// Draw a polyline specified in Dasher co-ordinates

void CDasherView::DasherPolyline(myint *x, myint *y, int n, int iWidth, int iColour) {

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

void CDasherView::DasherPolygon(myint *x, myint *y, int n, int iColour) {

  CDasherScreen::point * ScreenPoints = new CDasherScreen::point[n];

  for(int i(0); i < n; ++i)
    Dasher2Screen(x[i], y[i], ScreenPoints[i].x, ScreenPoints[i].y);

  Screen()->Polygon(ScreenPoints, n, iColour);
  delete[]ScreenPoints;
}

// Draw a box specified in Dasher co-ordinates

void CDasherView::DasherDrawRectangle(myint iLeft, myint iTop, myint iRight, myint iBottom, const int Color, int iOutlineColour, Opts::ColorSchemes ColorScheme, bool bDrawOutline, bool bFill, int iThickness) {

  screenint iScreenLeft;
  screenint iScreenTop;
  screenint iScreenRight;
  screenint iScreenBottom;

  Dasher2Screen(iLeft, iTop, iScreenLeft, iScreenTop);
  Dasher2Screen(iRight, iBottom, iScreenRight, iScreenBottom);

  Screen()->DrawRectangle(iScreenLeft, iScreenTop, iScreenRight, iScreenBottom, Color, iOutlineColour, ColorScheme, bDrawOutline, bFill, iThickness);
}

/// Draw a rectangle centred on a given dasher co-ordinate, but with a size specified in screen co-ordinates (used for drawing the mouse blob)

void CDasherView::DasherDrawCentredRectangle(myint iDasherX, myint iDasherY, screenint iSize, const int Color, Opts::ColorSchemes ColorScheme, bool bDrawOutline) {

  screenint iScreenX;
  screenint iScreenY;

  Dasher2Screen(iDasherX, iDasherY, iScreenX, iScreenY);

  Screen()->DrawRectangle(iScreenX - iSize, iScreenY - iSize, iScreenX + iSize, iScreenY + iSize, Color, -1, ColorScheme, bDrawOutline, true, 1);
}

/// Draw text specified in Dasher co-ordinates. The position is
/// specified as two co-ordinates, intended to the be the corners of
/// the leading edge of the containing box.

void CDasherView::DasherDrawText(myint iAnchorX1, myint iAnchorY1, myint iAnchorX2, myint iAnchorY2, const std::string &sDisplayText, int &mostleft, bool bShove) {

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

  screenint iLeftTimesFontSize = ((myint)GetLongParameter(LP_MAX_Y) - (iAnchorX1 + iAnchorX2)/ 2 )*Size;
  if(iLeftTimesFontSize < (myint)GetLongParameter(LP_MAX_Y) * 19/ 20)
    Size *= 20;
  else if(iLeftTimesFontSize < (myint)GetLongParameter(LP_MAX_Y) * 159 / 160)
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
