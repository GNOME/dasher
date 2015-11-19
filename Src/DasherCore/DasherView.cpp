// DasherView.cpp
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

#include "DasherInput.h"
#include "DasherView.h"

using namespace Dasher;
using std::vector;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////

CDasherView::CDasherView(CDasherScreen *DasherScreen, Opts::ScreenOrientations orient)
 : m_Orientation(orient), m_pScreen(DasherScreen) {
}

/////////////////////////////////////////////////////////////////////////////

void CDasherView::ChangeScreen(CDasherScreen *NewScreen) {
  m_pScreen = NewScreen;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherView::DasherSpaceLine(myint x1, myint y1, myint x2, myint y2, int iWidth, int iColor) {
  if (!ClipLineToVisible(x1, y1, x2, y2)) return;
  vector<CDasherScreen::point> vPoints;
  CDasherScreen::point p;
  Dasher2Screen(x1, y1, p.x, p.y);
  vPoints.push_back(p);
  DasherLine2Screen(x1,y1,x2,y2,vPoints);
  CDasherScreen::point *pts = new CDasherScreen::point[vPoints.size()];
  for (int i = vPoints.size(); i-->0; ) pts[i] = vPoints[i];
  Screen()->Polyline(pts, vPoints.size(), iWidth, iColor);
}

bool CDasherView::ClipLineToVisible(myint &x1, myint &y1, myint &x2, myint &y2) {
  if (x1 > x2) return ClipLineToVisible(x2,y2,x1,y1);
  //ok. have x1 <= x2...
  myint iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);
  if (x1 > iDasherMaxX) {
    DASHER_ASSERT(x2>iDasherMaxX);
    return false; //entirely offscreen!
  }
  if (x2 < iDasherMinX) {
    DASHER_ASSERT(x1<iDasherMinX);
    return false;
  }
  if (x1 < iDasherMinX) {
    y1 = y2+((y1-y2)*(iDasherMinX-x2)/(x1 - x2));
    x1 = iDasherMinX;
  }
  if (x2 > iDasherMaxX) {
    y2 = y1 + (y2-y1)*(iDasherMaxX-x1)/(x2-x1);
    x2 = iDasherMaxX;
  }
  if (y1 < iDasherMinY && y2 < iDasherMinY) return false;
  if (y1 > iDasherMaxY && y2 > iDasherMaxY) return false;
  for (int i=0; i<2; i++) {
    myint &y(i ? y2 : y1), &oy(i ? y1 : y2);
    myint &x(i ? x2 : x1), &ox(i ? x1 : x2);
    if (y<iDasherMinY) {
      x = ox- (ox-x)*(oy-iDasherMinY)/(oy-y);
      y = iDasherMinY;
    } else if (y>iDasherMaxY) {
      x = ox-(ox-x)*(oy-iDasherMaxY)/(oy-y);
      y = iDasherMaxY;
    }
  }
  return true;
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
    Screen()->Polyline(ScreenPoints, n, iWidth,0);//no color given
  }
  delete[]ScreenPoints;
}

// Draw a polyline with an arrow on the end
void CDasherView::DasherPolyarrow(myint *x, myint *y, int n, int iWidth, int iColour, double dArrowSizeFactor) {

  CDasherScreen::point * ScreenPoints = new CDasherScreen::point[n+3];

  for(int i(0); i < n; ++i)
    Dasher2Screen(x[i], y[i], ScreenPoints[i].x, ScreenPoints[i].y);

  int iXvec = (int)((ScreenPoints[n-2].x - ScreenPoints[n-1].x)*dArrowSizeFactor);
  int iYvec = (int)((ScreenPoints[n-2].y - ScreenPoints[n-1].y)*dArrowSizeFactor);

  ScreenPoints[n].x   = ScreenPoints[n-1].x + iXvec + iYvec;
  ScreenPoints[n].y   = ScreenPoints[n-1].y - iXvec + iYvec;
  ScreenPoints[n+1].x = ScreenPoints[n-1].x ;
  ScreenPoints[n+1].y = ScreenPoints[n-1].y ;
  ScreenPoints[n+2].x = ScreenPoints[n-1].x + iXvec - iYvec;
  ScreenPoints[n+2].y = ScreenPoints[n-1].y + iXvec + iYvec;

  Screen()->Polyline(ScreenPoints, n+3, iWidth, (iColour==-1) ? 0 : iColour);

  delete[]ScreenPoints;
}

// Draw a box specified in Dasher co-ordinates

void CDasherView::DasherDrawRectangle(myint iDasherMaxX, myint iDasherMinY, myint iDasherMinX, myint iDasherMaxY, const int Color, int iOutlineColour, int iThickness) {
  //This assertion is more aggressive than necessary (Dasher has been working
  // in many cases where it would fail, with only occassional display glitches)
  // so if it causes trouble, it should be safe to remove...
  DASHER_ASSERT(iDasherMinX <= iDasherMaxX && iDasherMinY <= iDasherMaxY);
  //TODO Parameter names correspond to the values passed in,
  // but the below will only match up with screen coords for LR orientation...
  screenint iScreenLeft;
  screenint iScreenTop;
  screenint iScreenRight;
  screenint iScreenBottom;

  Dasher2Screen(iDasherMaxX, iDasherMinY, iScreenLeft, iScreenTop);
  Dasher2Screen(iDasherMinX, iDasherMaxY, iScreenRight, iScreenBottom);

  Screen()->DrawRectangle(iScreenLeft, iScreenTop, iScreenRight, iScreenBottom, Color, iOutlineColour, iThickness);
}

/// Draw a rectangle centred on a given dasher co-ordinate, but with a size specified in screen co-ordinates (used for drawing the mouse blob)

void CDasherView::DasherDrawCentredRectangle(myint iDasherX, myint iDasherY, screenint iSize, const int Color, bool bDrawOutline) {
  screenint iScreenX;
  screenint iScreenY;

  Dasher2Screen(iDasherX, iDasherY, iScreenX, iScreenY);

  Screen()->DrawRectangle(iScreenX - iSize, iScreenY - iSize, iScreenX + iSize, iScreenY + iSize, Color, -1, bDrawOutline ? 1 : 0);
}
