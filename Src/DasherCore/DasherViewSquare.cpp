// DasherViewSquare.cpp
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

#ifdef _WIN32
#include "..\Win32\Common\WinCommon.h"
#endif

#include "DasherViewSquare.h"
#include "DasherView.h"
#include "DasherTypes.h"
#include "Event.h"
#include "Observable.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdlib.h>

using namespace Dasher;
using namespace Opts;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
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

CDasherViewSquare::CDasherViewSquare(CSettingsUser *pCreateFrom, CDasherScreen *DasherScreen, Opts::ScreenOrientations orient)
: CDasherView(DasherScreen,orient), CSettingsUserObserver(pCreateFrom), m_Y1(4), m_Y2(0.95 * CDasherModel::MAX_Y), m_Y3(0.05 * CDasherModel::MAX_Y), m_bVisibleRegionValid(false) {

  //Note, nonlinearity parameters set in SetScaleFactor
  ScreenResized(DasherScreen);
}

CDasherViewSquare::~CDasherViewSquare() {}

void CDasherViewSquare::SetOrientation(Opts::ScreenOrientations newOrient) {
  if (newOrient == GetOrientation()) return;
  CDasherView::SetOrientation(newOrient);
  m_bVisibleRegionValid=false;
  SetScaleFactor();
}

void CDasherViewSquare::HandleEvent(int iParameter) {
  switch (iParameter) {
    case LP_MARGIN_WIDTH:
    case BP_NONLINEAR_Y:
    case LP_NONLINEAR_X:
    case LP_GEOMETRY:
      m_bVisibleRegionValid = false;
      SetScaleFactor();
  }
}

CDasherNode *CDasherViewSquare::Render(CDasherNode *pRoot, myint iRootMin, myint iRootMax,
				    CExpansionPolicy &policy) {
  DASHER_ASSERT(pRoot != 0);
  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);
  //

  m_iRenderCount = 0;

  CDasherNode *pOutput = pRoot->Parent();

  // Blank the region around the root node:
  if (GetLongParameter(LP_SHAPE_TYPE)==0) { //disjoint rects, so go round root
    if(iRootMin > iDasherMinY)
      DasherDrawRectangle(iDasherMaxX, iDasherMinY, iDasherMinX, iRootMin, 0, -1, 0);

    if(iRootMax < iDasherMaxY)
      DasherDrawRectangle(iDasherMaxX, iRootMax, iDasherMinX, iDasherMaxY, 0, -1, 0);

    //to left (greater Dasher X)
    if (iRootMax - iRootMin < iDasherMaxX)
      DasherDrawRectangle(iDasherMaxX, std::max(iRootMin,iDasherMinY), iRootMax-iRootMin, std::min(iRootMax,iDasherMaxY), 0, -1, 0);

    //to right (margin)
    DasherDrawRectangle(0, iDasherMinY, iDasherMinX, iDasherMaxY, 0, -1, 0);

    //and render root.
    DisjointRender(pRoot, iRootMin, iRootMax, NULL, policy, std::numeric_limits<double>::infinity(), pOutput);
  } else {
    //overlapping rects/shapes
    if (pOutput) {
      //LEFT of Y axis, would be entirely covered by the root node parent (before we render root)
      // (getColour() gives the right colour, even if pOutput is invisible - in that case it gives
      // the colour of its parent)
      DasherDrawRectangle(iDasherMaxX, iDasherMinY, 0, iDasherMaxY, pOutput->getColour(), -1, 0);
      //RIGHT of Y axis, should be white.
      DasherDrawRectangle(0, iDasherMinY, iDasherMinX, iDasherMaxY, 0, -1, 0);
    } else //easy case, whole screen is white (outside root node, e.g. when starting)
      Screen()->DrawRectangle(0, 0, Screen()->GetWidth(), Screen()->GetHeight(), 0, -1, 0);
    NewRender(pRoot, iRootMin, iRootMax, NULL, policy, std::numeric_limits<double>::infinity(), pOutput);
  }

  // Labels are drawn in a second parse to get the overlapping right
  for (vector<CTextString *>::iterator it=m_DelayedTexts.begin(), E=m_DelayedTexts.end(); it!=E; it++)
    DoDelayedText(*it);
  m_DelayedTexts.clear();

  // Finally decorate the view
  Crosshair();
  return pOutput;
}

/// Draw text specified in Dasher co-ordinates. The position is
/// specified as two co-ordinates, intended to the be the corners of
/// the leading edge of the containing box.

CDasherViewSquare::CTextString *CDasherViewSquare::DasherDrawText(myint iDasherMaxX, myint iDasherMidY, CDasherScreen::Label *pLabel, CTextString *pParent, int iColor) {

  screenint x,y;
  Dasher2Screen(iDasherMaxX, iDasherMidY, x, y);

  //compute font size...
  int iSize = GetLongParameter(LP_DASHER_FONTSIZE);
  {
    const myint iMaxY(CDasherModel::MAX_Y);
    if (Screen()->MultiSizeFonts() && iSize>4) {
      //font size maxes out at ((iMaxY*3)/2)+iMaxY)/iMaxY = 3/2*smallest
      // which is reached when iDasherMaxX == iMaxY/2, i.e. the crosshair
      iSize = ((min(iDasherMaxX*3,(iMaxY*3)/2) + iMaxY) * iSize) / iMaxY;
    } else {
      //old style fonts; ignore iSize passed-in.
      myint iLeftTimesFontSize = (iMaxY - iDasherMaxX )*iSize;
      if(iLeftTimesFontSize < iMaxY * 19/ 20)
        iSize *= 20;
      else if(iLeftTimesFontSize < iMaxY * 159 / 160)
        iSize *= 14;
      else
        iSize *= 11;
    }
  }

  CTextString *pRet = new CTextString(pLabel, x, y, iSize, iColor);
  vector<CTextString *> &dest(pParent ? pParent->m_children : m_DelayedTexts);
  dest.push_back(pRet);
  return pRet;
}

void CDasherViewSquare::DoDelayedText(CTextString *pText) {

  //note that it'd be better to compute old-style font sizes here, or even after shunting
  // across according to the aiMax array, but this needs Dasher co-ordinates, which were
  // more easily available at CTextString creation time. If it really doesn't look as good,
  // can put in extra calls to Screen2Dasher....
  screenint x(pText->m_ix), y(pText->m_iy);
  pair<screenint,screenint> textDims=Screen()->TextSize(pText->m_pLabel, pText->m_iSize);
  switch (GetOrientation()) {
    case Dasher::Opts::LeftToRight: {
      screenint iRight = x + textDims.first;
      if (iRight < Screen()->GetWidth()) {
        Screen()->DrawString(pText->m_pLabel, x, y-textDims.second/2, pText->m_iSize, pText->m_iColor);
        for (vector<CTextString *>::iterator it = pText->m_children.begin(); it!=pText->m_children.end(); it++) {
          CTextString *pChild=*it;
          pChild->m_ix = max(pChild->m_ix, iRight);
          DoDelayedText(pChild);
        }
        pText->m_children.clear();
      }
      break;
    }
    case Dasher::Opts::RightToLeft: {
      screenint iLeft = x-textDims.first;
      if (iLeft>=0) {
        Screen()->DrawString(pText->m_pLabel, iLeft, y-textDims.second/2, pText->m_iSize, pText->m_iColor);
        for (vector<CTextString *>::iterator it = pText->m_children.begin(); it!=pText->m_children.end(); it++) {
          CTextString *pChild=*it;
          pChild->m_ix = min(pChild->m_ix, iLeft);
          DoDelayedText(*it);
        }
        pText->m_children.clear();
      }
      break;
    }
    case Dasher::Opts::TopToBottom: {
      screenint iBottom = y + textDims.second;
      if (iBottom < Screen()->GetHeight()) {
        Screen()->DrawString(pText->m_pLabel, x-textDims.first/2, y, pText->m_iSize, pText->m_iColor);
        for (vector<CTextString *>::iterator it = pText->m_children.begin(); it!=pText->m_children.end(); it++) {
          CTextString *pChild=*it;
          pChild->m_iy = max(pChild->m_iy, iBottom);
          DoDelayedText(*it);
        }
        pText->m_children.clear();
      }
      break;
    }
    case Dasher::Opts::BottomToTop: {
      screenint iTop = y - textDims.second;
      if (y>=0) {
        Screen()->DrawString(pText->m_pLabel, x-textDims.first/2, iTop, pText->m_iSize, pText->m_iColor);
        for (vector<CTextString *>::iterator it = pText->m_children.begin(); it!=pText->m_children.end(); it++) {
          CTextString *pChild=*it;
          pChild->m_iy = min(pChild->m_iy, iTop);
          DoDelayedText(*it);
        }
        pText->m_children.clear();
      }
      break;
    }
    default:
      break;
  }
  delete pText;
}

CDasherViewSquare::CTextString::~CTextString() {
  for (vector<CTextString *>::iterator it = m_children.begin(); it!=m_children.end(); it++)
    delete *it;
}

void CDasherViewSquare::TruncateTri(myint x, myint y1, myint y2, myint midy1, myint midy2, int fillColor, int outlineColor, int lineWidth) {
  DASHER_ASSERT (y1<=midy1 && midy1<=midy2 && midy2<=y2);
  myint iVisibleMinX, iVisibleMaxX, iVisibleMinY, iVisibleMaxY;
  VisibleRegion(iVisibleMinX, iVisibleMinY, iVisibleMaxX, iVisibleMaxY);

  myint x1(x), x2(x); //(max)x-coords of the two lines
  myint tempx1(0),tempx2(0); //& min x-coords
  //intersect y1's diagonal with screen
  if (!ClipLineToVisible(x1,midy1,tempx1,y1)) {
    //entirely offscreen....i.e. off top/bottom
    //DASHER_ASSERT (midy1 < iVisibleMinY);//no, args undefined if returns false!
    midy1 = y1 = iVisibleMinY;
    x1 = min(x1, iVisibleMaxX);
    tempx1=0;
  }
  //intersect y2's diagonal with screen
  if (!ClipLineToVisible(tempx2, y2, x2, midy2)) {
    //entirely offscreen again, i.e. off bottom/top
    midy2 = y2 = iVisibleMaxY;
    x2 = min(x2, iVisibleMaxX);
    tempx2=0;
  }
  if (x1!=x2) {
    //both will be clipped to be <= iVisibleMaxX by above. If they are still
    // unequal, one must have been further clipped by passing off top/bottom
    // (i.e., the point of max x is off the top/off the bottom), in which case
    // the other line is entirely offscreen:
    DASHER_ASSERT(midy1 == midy2); //point/line of max x has been removed

    if (x1<x2) {
      //(0,y1) - (x1,midy1) hit max-y edge of screen
      //(0,y2) - (x2,midy2) entirely offscreen
      DASHER_ASSERT(midy1==iVisibleMaxY && y2 == midy2);
      x2=x1;
    } else {
      // (0,y2) - (x2, midy2) hit min-y edge of screen
      // (0,y1) - (x1,midy1) entirely offscreen
      DASHER_ASSERT(midy2 == iVisibleMinY && y1 == midy1);
      x1=x2;
    }
  }
  // midy1,x1 is now start point
  vector<CDasherScreen::point> pts(1);
  Dasher2Screen(x1, midy1, pts[0].x, pts[0].y);
  DasherLine2Screen(x1, midy1, tempx1, y1, pts);
  if (tempx1) {
    //did not reach y axis
    DASHER_ASSERT(y1 == iVisibleMinY);
    pts.push_back(CDasherScreen::point());
    Dasher2Screen(0, iVisibleMinY, pts.back().x, pts.back().y);
  }
  //that gets us to the min-y (y1) end of the line along the y-axis

  //add line along y-axis...
  pts.push_back(CDasherScreen::point());
  Dasher2Screen(0, y2, pts.back().x, pts.back().y);

  if (tempx2) {
    //y2's diagonal did not reach y-axis
    DASHER_ASSERT(y2 == iVisibleMaxY);
    pts.push_back(CDasherScreen::point());
    Dasher2Screen(tempx2, iVisibleMaxY, pts.back().x, pts.back().y);
  }
  //and the diagonal part...
  DasherLine2Screen(tempx2, y2, x2, midy2, pts);

  if (midy1 != midy2) {
    //is the max-x extent a line, after cropping - i.e. handles both
    // normal triangle (orig midy1==orig midy2) being cropped to screen edge,
    // and trunc tri (orig midy1 < orig midy2, possibly cropped) cases
    DASHER_ASSERT(x1 == x2);
    pts.push_back(CDasherScreen::point());
    Dasher2Screen(x1, midy1, pts.back().x, pts.back().y);
  } else DASHER_ASSERT(pts.back().x == pts[0].x && pts.back().y == pts[0].y);

  CDasherScreen::point *p_array=new CDasherScreen::point[pts.size()];
  for (unsigned int i = 0; i<pts.size(); i++) p_array[i] = pts[i];
  Screen()->Polygon(p_array, pts.size(), fillColor, outlineColor, lineWidth);
  delete[] p_array;
}

#define sq(X) ((X)*(X))
void CDasherViewSquare::Circle(myint Range, myint y1, myint y2, int fCol, int oCol, int lWidth) {
  std::vector<CDasherScreen::point> pts;
  myint cy((y1+y2)/2),r(Range/2), x1, x2;
  myint iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  CDasherScreen::point p;
  //run along bottom edge...
  if (y1 < iDasherMinY) {
    Dasher2Screen(0, iDasherMinY, p.x, p.y);
    pts.push_back(p);
    //intersect with bottom edge
    x1 = min(iDasherMaxX, myint(sqrt(double(r*r - sq(cy-iDasherMinY)))));
    y1 = iDasherMinY;
  } else {
    x1=0;
  }
  Dasher2Screen(x1,y1,p.x,p.y);
  pts.push_back(p);

  //and along top...
  if (y2 > iDasherMaxY) {
    //intersect...
    x2 = min(iDasherMaxX, myint(sqrt(double(r*r - sq(iDasherMaxY-cy)))));
    Dasher2Screen(x2, y2=iDasherMaxY, p.x, p.y);
    //that's target point for end of curved section.
    if (x2==iDasherMaxX && x1==iDasherMaxX) {
      //circle entirely covers screen
      DASHER_ASSERT(y1==iDasherMinY);
      DasherDrawRectangle(iDasherMaxX, iDasherMinY, 0, iDasherMaxY, fCol, oCol, lWidth);
      return;
    }
    //will also need final point at top-right (0,y2 in dasher coords)....
  } else {
    Dasher2Screen(x2=0,y2,p.x,p.y);
  }
  CircleTo(cy,r,y1,x1,y2,x2,p,pts, 2.0);
  if (iDasherMaxY == y2) {
    Dasher2Screen(0, iDasherMaxX, p.x, p.y);
    pts.push_back(p);
  }
  CDasherScreen::point *p_array = new CDasherScreen::point[pts.size()];
  for (unsigned int i=0; i<pts.size(); i++) p_array[i] = pts[i];
  Screen()->Polygon(p_array, pts.size(), fCol, oCol, lWidth);
  delete[] p_array;
}

void CDasherViewSquare::CircleTo(myint cy, myint r, myint y1, myint x1, myint y3, myint x3, CDasherScreen::point dest, vector<CDasherScreen::point> &pts, double dXMul) {
  myint y2((y1+y3)/2);
  myint x2(sqrt(double(sq(r)-sq(cy-y2)))*dXMul);
  CDasherScreen::point mid; //where midpoint of circle/arc should be
  Dasher2Screen(x2, y2, mid.x, mid.y); //(except "midpoint" measured along y axis)
  int lmx=(pts.back().x + dest.x)/2, lmy = (pts.back().y + dest.y)/2; //midpoint of straight line
  if (abs(dest.y-pts.back().y)<2 || abs(mid.x-lmx) + abs(mid.y-lmy)<2) {
    //okay, use straight line
    pts.push_back(dest);
  } else {
    CircleTo(cy, r, y1, x1, y2, x2, mid, pts, dXMul);
    CircleTo(cy, r, y2, x2, y3, x3, dest, pts, dXMul);
  }
}
#undef sq

void CDasherViewSquare::DasherSpaceArc(myint cy, myint r, myint x1, myint y1, myint x2, myint y2, int iColour, int iLineWidth) {
  CDasherScreen::point p;
  //start point
  Dasher2Screen(x1, y1, p.x, p.y);
  vector<CDasherScreen::point> pts;
  pts.push_back(p);
  //if circle goes behind crosshair and we want the point of max-x, force division into two sections with that point as boundary
  if (r>CDasherModel::ORIGIN_X && ((y1 < cy) ^ (y2 < cy))) {
    Dasher2Screen(r, cy, p.x, p.y);
    CircleTo(cy, r, y1, x1, cy, r, p, pts, 1.0);
    x1=r; y1=cy;
  }
  Dasher2Screen(x2, y2, p.x, p.y);
  CircleTo(cy, r, y1, x1, y2, x2, p, pts, 1.0);
  CDasherScreen::point *p_array = new CDasherScreen::point[pts.size()];
  for (unsigned int i=0; i<pts.size(); i++) p_array[i] = pts[i];
  Screen()->Polyline(p_array, pts.size(), iLineWidth, iColour);
}

void CDasherViewSquare::Quadric(myint Range, myint lowY, myint highY, int fillColor, int outlineColour, int lineWidth) {
  static const double RR2=1.0/sqrt(2.0);
  const int midY=(lowY+highY)/2;
#define NUM_STEPS 40
  CDasherScreen::point p_array[2*NUM_STEPS+2];
  myint minX,maxX,minY,maxY;
  VisibleRegion(minX, minY, maxX, maxY);
  {
    myint x1(0), y1(highY), x2(Range*RR2),y2(highY*RR2 + midY*(1.0-RR2)), x3(Range), y3(midY);
    for (int i=0; i<=NUM_STEPS; i++) {
      double f=i/(double)NUM_STEPS, of = 1.0-f;
      Dasher2Screen(min(maxX,myint(of*of*x1 + 2.0*of*f*x2 + f*f*x3)), max(minY,min(maxY,myint(of*of*y1 + 2.0*of*f*y2 + f*f*y3))), p_array[i].x, p_array[i].y);
    }
  }
  {
    myint x1(Range), y1(midY), x2(Range*RR2), y2(lowY*RR2 + midY*(1.0-RR2)), x3(0), y3(lowY);
    for (int i=0; i<=NUM_STEPS; i++) {
      double f=i/(double)NUM_STEPS, of = 1.0-f;
      Dasher2Screen(min(maxX,myint(of*of*x1 + 2.0*of*f*x2 + f*f*x3)),max(minY,min(maxY,myint(of*of*y1 + 2.0*of*f*y2 + f*f*y3))), p_array[i+NUM_STEPS+1].x, p_array[i+NUM_STEPS+1].y);
    }
  }

  Screen()->Polygon(p_array, 2*NUM_STEPS+2, fillColor, outlineColour, lineWidth);
#undef NUM_STEPS
}

bool CDasherViewSquare::IsSpaceAroundNode(myint y1, myint y2) {
  myint iVisibleMinX;
  myint iVisibleMinY;
  myint iVisibleMaxX;
  myint iVisibleMaxY;

  VisibleRegion(iVisibleMinX, iVisibleMinY, iVisibleMaxX, iVisibleMaxY);
  const myint maxX(y2-y1);
  if ((maxX < iVisibleMaxX) || (y1 > iVisibleMinY) || (y2 < iVisibleMaxY))
    return true; //space around sq => space around anything smaller!

  //in theory, even if the crosshair is off-screen (!), anything spanning y1-y2 should cover it...
  DASHER_ASSERT (CoversCrosshair(y2-y1, y1, y2));

  switch (GetLongParameter(LP_SHAPE_TYPE)) {
    case 0: //non-overlapping rects
    case 1: //overlapping rects
      return false;
    case 2: { //simple triangles
      const myint iMidY((y1+y2)/2);
      return (iMidY < iVisibleMaxY && (y2-iVisibleMaxY)*maxX < iVisibleMaxX*(y2-iMidY))
          || (iMidY > iVisibleMinY && (iVisibleMinY-y1)*maxX < iVisibleMaxX*(iMidY-y1));
    }
    case 3: { //truncated triangles
      const myint y113((y1+y1+y2)/3), y123((y1+y2+y2)/3);
      return (y123 < iVisibleMaxY && (y2-iVisibleMaxY)*maxX < iVisibleMaxX*(y2-y123))
          || (y113 > iVisibleMinY && (iVisibleMinY-y1)*maxX < iVisibleMaxX*(y123-y1));
    }
    case 4: //quadric.
      //erm. seems hard. fall-through to circle, as it isn't far out -
      // unfortunately it's not a conservative approximation, the circle
      // covers the quadric not the other way around, so we'll say the
      // circle covers the screen when the quadric doesn't :-(. However
      // atm circles seem better generally so fixing quadrics is a low priority!
    case 5: { //circle - or rather ellipse, x diameter is twice y diam, hence the *2 to normalize
      const myint iMidY((y1+y2)/2); //centerX=0, radius = maxX
      const myint maxYDiff(max(iVisibleMaxY-iMidY,iMidY-iVisibleMinY)*2);
      return maxYDiff*maxYDiff + iVisibleMaxX*iVisibleMaxX > maxX*maxX;
    }
  }
  /* NOTREACHED */
  return false;
}

void CDasherViewSquare::DisjointRender(CDasherNode *pRender, myint y1, myint y2,
					CTextString *pPrevText, CExpansionPolicy &policy, double dMaxCost,
					CDasherNode *&pOutput)
{
  DASHER_ASSERT_VALIDPTR_RW(pRender);

  ++m_iRenderCount;

  // Set the NF_SUPER flag if this node entirely frames the visual
  // area.

  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);
  pRender->SetFlag(NF_SUPER, (y2-y1 >= iDasherMaxX) && (y1 <= iDasherMinY) && (y2 >= iDasherMaxY));

  const int myColor = pRender->getColour();

  if( pRender->getLabel() )
  {
    const int textColor = GetLongParameter(LP_OUTLINE_WIDTH)<0 ? myColor : 4;
    myint ny1 = std::min(iDasherMaxY, std::max(iDasherMinY, y1)),
          ny2 = std::min(iDasherMaxY, std::max(iDasherMinY, y2));
    CTextString *pText = DasherDrawText(y2-y1, (ny1+ny2)/2, pRender->getLabel(), pPrevText, textColor);
    if (pRender->bShove()) pPrevText = pText;
  }

  const myint Range(y2-y1);

  //Does node cover crosshair?
  if (pOutput == pRender->Parent() && Range > CDasherModel::ORIGIN_X && y1 < CDasherModel::ORIGIN_Y && y2 > CDasherModel::ORIGIN_Y) {
    pOutput=pRender;
    if (pRender->ChildCount()==0) {
      //covers crosshair! forcibly populate, now!
      policy.ExpandNode(pRender);
    }
  }
  if (pRender->ChildCount() == 0) {
    //allow empty node to be expanded, it's big enough.
    policy.pushNode(pRender, y1, y2, true, dMaxCost);
    //and render whole node in one go
    DasherDrawRectangle(std::min(Range,iDasherMaxX), std::max(y1,iDasherMinY),0, std::min(y2,iDasherMaxY), myColor, -1, 0);
    //fall through to draw outline
  } else {
    //Node has children. It can therefore be collapsed...however,
    // we don't allow a node covering the crosshair to be collapsed
    // (at best this'll mean there's nowhere useful to go forwards;
    // at worst, all kinds of crashes trying to do text output!)

    //No reason why we can't collapse a game mode node that's too small/offscreen
    // - we've got its coordinates, and can recreate its children and set their
    // NF_GAME flags appropriately when it becomes renderable again...
    if (pRender != pOutput)
      dMaxCost = policy.pushNode(pRender, y1, y2, false, dMaxCost);

    // Render children

    int id=-1;

    if (CDasherNode *pChild = pRender->onlyChildRendered)
    {
      //if child still covers screen, render _just_ it and return
      myint newy1 = y1 + (Range * pChild->Lbnd()) / CDasherModel::NORMALIZATION;
      myint newy2 = y1 + (Range * pChild->Hbnd()) / CDasherModel::NORMALIZATION;
      if (newy1 < iDasherMinY && newy2 > iDasherMaxY) {
        //still covers entire screen. Parent should too...
        DASHER_ASSERT(dMaxCost == std::numeric_limits<double>::infinity());

        if (newy2-newy1 < iDasherMaxX) //fill in to it's left...
          DasherDrawRectangle(std::min(Range,iDasherMaxX), std::max(y1,iDasherMinY), newy2-newy1, std::min(y2,iDasherMaxY), myColor, -1, 0);
        DisjointRender(pChild, newy1, newy2, pPrevText,
                        policy, dMaxCost, pOutput);
        //leave pRender->onlyChildRendered set, so remaining children are skipped
      }
      else
        pRender->onlyChildRendered = NULL;
    }

    if (!pRender->onlyChildRendered) {
      //render all children...
      myint lasty=y1;
      for(CDasherNode::ChildMap::const_iterator i = pRender->GetChildren().begin();
        i != pRender->GetChildren().end(); i++) {
        id++;
        CDasherNode *pChild = *i;

        myint newy1 = y1 + (Range * pChild->Lbnd()) / CDasherModel::NORMALIZATION;
        myint newy2 = y1 + (Range * pChild->Hbnd()) / CDasherModel::NORMALIZATION;

        if (pChild->GetFlag(NF_GAME)) {
          CGameNodeDrawEvent evt(pChild, newy1, newy2);
          Observable<CGameNodeDrawEvent*>::DispatchEvent(&evt);
        }
        //switch to "render just one child" mode if all others are off the screen,
        //and if this _won't_ cause us to avoid rendering a game node...
        if (newy1 < iDasherMinY && newy2 > iDasherMaxY && (!pRender->GetFlag(NF_GAME) || pChild->GetFlag(NF_GAME)))  {
          DASHER_ASSERT(dMaxCost == std::numeric_limits<double>::infinity());
          pRender->onlyChildRendered = pChild;
          if (newy2-newy1 < iDasherMaxX)
            DasherDrawRectangle(std::min(Range,iDasherMaxX), std::max(y1,iDasherMinY), newy2-newy1, std::min(y2,iDasherMaxY), myColor, -1, 0);
          DisjointRender(pChild, newy1, newy2, pPrevText, policy, dMaxCost, pOutput);
          //ensure we don't blank over this child in "finishing off" the parent (!)
          lasty=newy2;
          //all remaining children are offscreen. quickly delete, avoid recomputing ranges...
          while ((++i)!=pRender->GetChildren().end())
            if (!(*i)->GetFlag(NF_SEEN)) (*i)->Delete_children();
          break;
        } else if (newy2-newy1 >= GetLongParameter(LP_MIN_NODE_SIZE) //simple test if big enough
            && newy1 <= iDasherMaxY && newy2 >= iDasherMinY) //at least partly on screen
        {
          //child should be rendered!
          //fill in to its left
          DasherDrawRectangle(std::min(y2-y1,iDasherMaxX), std::max(newy1,iDasherMinY), std::min(newy2-newy1,iDasherMaxX), std::min(newy2,iDasherMaxY), myColor, -1, 0);

          if (std::max(lasty,iDasherMinY)<newy1) //fill in interval above child up to the last drawn child
            DasherDrawRectangle(std::min(Range,iDasherMaxX), std::max(lasty,iDasherMinY),0, std::min(newy1,iDasherMaxY), myColor, -1, 0);
          lasty = newy2;
          DisjointRender(pChild, newy1, newy2, pPrevText, policy, dMaxCost, pOutput);
        } else {
          // We get here if the node is too small to render or is off-screen.
          // So, collapse it immediately.
          if(!pChild->GetFlag(NF_SEEN))
            pChild->Delete_children();
        }
      }
      //all children rendered.
      if (lasty<min(y2,iDasherMaxY)) {
        // Finish off the drawing process, filling in any part of the parent below the last-rendered child
        DasherDrawRectangle(std::min(Range,iDasherMaxX), std::max(lasty, iDasherMinY), 0, std::min(y2, iDasherMaxY), myColor, -1, 0);
      }
    }
    //end rendering children, fall through to outline
  }
  // Lastly, draw the outline
  if(GetLongParameter(LP_OUTLINE_WIDTH) && pRender->GetFlag(NF_VISIBLE)) {
    DasherDrawRectangle(std::min(Range,iDasherMaxX), std::max(y1,iDasherMinY),0, std::min(y2,iDasherMaxY), -1, -1, abs(GetLongParameter(LP_OUTLINE_WIDTH)));
  }
}

bool CDasherViewSquare::CoversCrosshair(myint Range, myint y1, myint y2) {
  if (Range > CDasherModel::ORIGIN_X && y1 < CDasherModel::ORIGIN_Y && y2 > CDasherModel::ORIGIN_Y) {
    switch (GetLongParameter(LP_SHAPE_TYPE)) {
      case 0: //Disjoint rectangles
      case 1: //Rectangles
        return true;
      case 2: {       //Triangles
        myint iMidY((y1+y2)/2);
        return (iMidY > CDasherModel::ORIGIN_Y)
        ? ((CDasherModel::ORIGIN_Y-y1)*Range) > (iMidY - y1) * CDasherModel::ORIGIN_X
        : ((y2-CDasherModel::ORIGIN_Y)*Range) > (y2 - iMidY) * CDasherModel::ORIGIN_X;
      }
      case 3: {       //Truncated tris
        myint midy1((y1+y1+y2)/3), midy2((y1+y2+y2)/3);
        if (midy1 > CDasherModel::ORIGIN_Y) //(0,y1) - (Range,midy1)
          return (CDasherModel::ORIGIN_Y-y1)*Range > (midy1 - y1) * CDasherModel::ORIGIN_X;
        if (midy2 > CDasherModel::ORIGIN_Y) // (Range,midy1) - (Range,midy2)
          return true;
        return (y2 - CDasherModel::ORIGIN_Y)*Range > (y2 - midy2) * CDasherModel::ORIGIN_X;
        break;
      }
      case 4: //quadrics. We'll approximate with circles, as they're easier...
        // however, note that the circle is bigger, so this'll output things
        // too soon/aggressively :-(.
        // (hence, fallthrough to:)
      case 5: { //circles - actually ellipses, as x diameter is twice y diameter, hence the *4
        const myint y_dist(CDasherModel::ORIGIN_Y - (y1+y2)/2);
        return CDasherModel::ORIGIN_X * CDasherModel::ORIGIN_X + y_dist*y_dist*4 < Range*Range;
      }
    }
  }
  return false;
}

void CDasherViewSquare::NewRender(CDasherNode *pRender, myint y1, myint y2,
                                  CTextString *pPrevText, CExpansionPolicy &policy, double dMaxCost,
                                  CDasherNode *&pOutput)
{
	//when we have only one child node to render, which'll be the last thing we
	// do before returning, we make a tail call by jumping here, rather than
	// pushing another stack frame:
beginning:
  DASHER_ASSERT_VALIDPTR_RW(pRender);

  ++m_iRenderCount;

  // Set the NF_SUPER flag if this node entirely frames the visual
  // area.

  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);
  pRender->SetFlag(NF_SUPER, !IsSpaceAroundNode(y1, y2));

  const int myColor = pRender->getColour();

  if( pRender->getLabel() )
  {
    const int textColor = GetLongParameter(LP_OUTLINE_WIDTH)<0 ? myColor : 4;
    myint ny1 = std::min(iDasherMaxY, std::max(iDasherMinY, y1)),
    ny2 = std::min(iDasherMaxY, std::max(iDasherMinY, y2));
    CTextString *pText = DasherDrawText(y2-y1, (ny1+ny2)/2, pRender->getLabel(), pPrevText, textColor);
    if (pRender->bShove()) pPrevText = pText;
  }

  const myint Range(y2-y1);
  // Draw node...we can both fill & outline in one go, since
  // we're drawing the whole node at once (unlike disjoint-rects),
  // as any part of the outline which is obscured by a child node,
  // will have the outline of the child node painted over it,
  // and all outlines are the same colour.

  //"invisible" nodes are given same colour as parent, so we neither fill
  // nor outline them. TODO this isn't quite right, as nodes that are
  // _supposed_ to be the same colour as their parent, will have no outlines...
  // (thankfully having 2 "phases" means this doesn't happen in standard
  // colour schemes)
  if (pRender->GetFlag(NF_VISIBLE)) {
	//outline width 0 = fill only; >0 = fill + outline; <0 = outline only
	int fillColour = GetLongParameter(LP_OUTLINE_WIDTH)>=0 ? myColor : -1;
	int lineWidth = abs(GetLongParameter(LP_OUTLINE_WIDTH));
    switch (GetLongParameter(LP_SHAPE_TYPE)) {
      case 1: //overlapping rects
        DasherDrawRectangle(std::min(Range,iDasherMaxX), std::max(y1,iDasherMinY), 0, std::min(y2,iDasherMaxY), fillColour, -1, lineWidth);
        break;
      case 2: //simple triangles
        TruncateTri(Range, y1, y2, (y1+y2)/2, (y1+y2)/2, fillColour, -1, lineWidth);
        break;
      case 3: //truncated triangles
        TruncateTri(Range, y1, y2, (y1+y1+y2)/3, (y1+y2+y2)/3, fillColour, -1, lineWidth);
        break;
      case 4:
        Quadric(Range, y1, y2, fillColour, -1, lineWidth);
        break;
      case 5:
        Circle(Range, y1, y2, fillColour, -1, lineWidth);
        break;
    }
  }

  //Does node cover crosshair?
  if (pOutput == pRender->Parent() && CoversCrosshair(Range, y1, y2))
    pOutput = pRender;

  if (pRender->ChildCount() == 0) {
    if (pOutput==pRender) {
      //covers crosshair! forcibly populate, now!
      policy.ExpandNode(pRender);
    } else {
      //allow empty node to be expanded, it's big enough.
      policy.pushNode(pRender, y1, y2, true, dMaxCost);
      return; //no children atm => nothing more to do
    }
  } else {
    //Node has children. It can therefore be collapsed...however,
    // we don't allow a node covering the crosshair to be collapsed
    // (at best this'll mean there's nowhere useful to go forwards;
    // at worst, all kinds of crashes trying to do text output!)

    //No reason why we can't collapse a game mode node that's too small/offscreen
    // - we've got its coordinates, and can recreate its children and set their
    // NF_GAME flags appropriately when it becomes renderable again...
    if (pRender != pOutput)
      dMaxCost = policy.pushNode(pRender, y1, y2, false, dMaxCost);
  }
  //Node has children - either it already did, or else it covers the crosshair,
  // and we've just made them...so render them.

  //first check if there's only one child we need to render
  if (CDasherNode *pChild = pRender->onlyChildRendered) {
    //if child still covers screen, render _just_ it and return
    myint newy1 = y1 + (Range * pChild->Lbnd()) / CDasherModel::NORMALIZATION;
    myint newy2 = y1 + (Range * pChild->Hbnd()) / CDasherModel::NORMALIZATION;
    if (
	    (newy1 < iDasherMinY && newy2 > iDasherMaxY)) { //covers entire y-axis!
         //render just that child; nothing more to do for this node => tail call to beginning
         pRender = pChild; y1=newy1; y2=newy2;
         goto beginning;
    }
    pRender->onlyChildRendered = NULL;
  }

  //ok, need to render all children...
  myint newy1=y1,newy2;
  CDasherNode::ChildMap::const_iterator I = pRender->GetChildren().begin(), E = pRender->GetChildren().end();
  while (I!=E) {
    CDasherNode *pChild(*I);

    newy2 = y1 + (Range * pChild->Hbnd()) / CDasherModel::NORMALIZATION;
    if (pChild->GetFlag(NF_GAME)) {
      CGameNodeDrawEvent evt(pChild, newy1, newy2);
      Observable<CGameNodeDrawEvent*>::DispatchEvent(&evt);
    }
    if (newy1<=iDasherMaxY && newy2 >= iDasherMinY) { //onscreen
      if (newy2-newy1 > GetLongParameter(LP_MIN_NODE_SIZE)) {
        //definitely big enough to render.
        NewRender(pChild, newy1, newy2, pPrevText, policy, dMaxCost, pOutput);
      } else if (!pChild->GetFlag(NF_SEEN)) pChild->Delete_children();
      if (newy2>iDasherMaxY && !pRender->GetFlag(NF_GAME)) {
        //remaining children offscreen and no game-mode child we might skip
        // (among the remainder, or any previous off the top of the screen)
        if (newy1 < iDasherMinY) pRender->onlyChildRendered = pChild; //previous children also offscreen!
        break; //skip remaining children
      }
    }
    I++;
    newy1=newy2;
  }
  if (I!=E) {
    //broke out of loop. Possibly more to delete...
    while (++I!=E) if (!(*I)->GetFlag(NF_SEEN)) (*I)->Delete_children();
  }
  //all children rendered.
}

/// Convert screen co-ordinates to dasher co-ordinates. This doesn't
/// include the nonlinear mapping for eyetracking mode etc - it is
/// just the inverse of the mapping used to calculate the screen
/// positions of boxes etc.

void CDasherViewSquare::Screen2Dasher(screenint iInputX, screenint iInputY, myint &iDasherX, myint &iDasherY) {

  // Things we're likely to need:

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  switch(GetOrientation()) {
  case Dasher::Opts::LeftToRight:
    iDasherX = ( iScreenWidth - iInputX ) * SCALE_FACTOR / iScaleFactorX;
    iDasherY = CDasherModel::MAX_Y / 2 + ( iInputY - iScreenHeight / 2 ) * SCALE_FACTOR / iScaleFactorY;
    break;
  case Dasher::Opts::RightToLeft:
    iDasherX = myint( ( iInputX ) * SCALE_FACTOR / iScaleFactorX);
    iDasherY = myint(CDasherModel::MAX_Y / 2 + ( iInputY - iScreenHeight / 2 ) * SCALE_FACTOR / iScaleFactorY);
    break;
  case Dasher::Opts::TopToBottom:
    iDasherX = myint( ( iScreenHeight - iInputY ) * SCALE_FACTOR / iScaleFactorX);
    iDasherY = myint(CDasherModel::MAX_Y / 2 + ( iInputX - iScreenWidth / 2 ) * SCALE_FACTOR / iScaleFactorY);
    break;
  case Dasher::Opts::BottomToTop:
    iDasherX = myint( ( iInputY  ) * SCALE_FACTOR / iScaleFactorX);
    iDasherY = myint(CDasherModel::MAX_Y / 2 + ( iInputX - iScreenWidth / 2 ) * SCALE_FACTOR / iScaleFactorY);
    break;
    default:
      break;
  }

  iDasherX = ixmap(iDasherX);
  iDasherY = iymap(iDasherY);

}

void CDasherViewSquare::SetScaleFactor( void )
{
  //Parameters for X non-linearity.
  // Set some defaults here, in case we change(d) them later...
  m_iXlogThres=CDasherModel::MAX_Y/2; //threshold: DasherX's less than this are linear; those greater are logarithmic

  //set log scaling coefficient (unused if LP_NONLINEAR_X==0)
  // note previous value = 1/0.2, i.e. a value of LP_NONLINEAR_X =~= 4.8
  m_dXlogCoeff = exp(GetLongParameter(LP_NONLINEAR_X)/3.0);

  const bool bHoriz(GetOrientation() == Dasher::Opts::LeftToRight || GetOrientation() == Dasher::Opts::RightToLeft);
  const screenint iScreenWidth(Screen()->GetWidth()), iScreenHeight(Screen()->GetHeight());
  const double dPixelsX(bHoriz ? iScreenWidth : iScreenHeight), dPixelsY(bHoriz ? iScreenHeight : iScreenWidth);

  //Defaults/starting values, will be modified later according to scheme in use...
  iMarginWidth = GetLongParameter(LP_MARGIN_WIDTH);
  double dScaleFactorY(dPixelsY / CDasherModel::MAX_Y );
  double dScaleFactorX(dPixelsX / static_cast<double>(CDasherModel::MAX_Y + iMarginWidth) );

  switch (GetLongParameter(LP_GEOMETRY)) {
    case 0: {
      //old style
      if (dScaleFactorX < dScaleFactorY) {
        //fewer (pixels per dasher coord) in X direction - i.e., X is more compressed.
        //So, use X scale for Y too...except first, we'll _try_ to reduce the difference
        // by changing the relative scaling of X and Y (by at most 20%):
        double dMul = max(0.8, dScaleFactorX / dScaleFactorY);
        dScaleFactorY = std::max(dScaleFactorX/dMul, dScaleFactorY / 4.0);
        dScaleFactorX *= 0.9;
        iMarginWidth = (CDasherModel::MAX_Y/20.0 + iMarginWidth*0.95)/0.9;
      } else {
        //X has more room; use Y scale for both -> will get lots history
        // however, "compensate" by relaxing the default "relative scaling" of X
        // (normally only 90% of Y) towards 1...
        double dXmpc = std::min(1.0,0.9 * dScaleFactorX / dScaleFactorY);
        dScaleFactorX = max(dScaleFactorY, dScaleFactorX / 4.0)*dXmpc;
        iMarginWidth = (iMarginWidth + dPixelsX/dScaleFactorX - CDasherModel::MAX_Y)/2;
      }
      break;
    }
    //all new styles fix the y axis the way we want it (i.e. leave as above),
    // and just do different things with x...
    case 1:
      //square with x-hair possibly offscreen
      dScaleFactorX = dScaleFactorY;
      break;
    case 2:
    case 3: {
      //2 or 3 => squish x (so xhair always visible)
      const double dDesiredXPerPixel( (CDasherModel::MAX_Y + iMarginWidth) / dPixelsX), dMinXPerPixel((CDasherModel::ORIGIN_X+iMarginWidth)/dPixelsX);
      const double dAspect(1.0/dScaleFactorY/dDesiredXPerPixel);
      double dDasherXPerPixel( (dAspect<1.0)
                              ? (dMinXPerPixel+pow(dAspect,3.0)*(dDesiredXPerPixel-dMinXPerPixel)) //tall+thin
                              : (1.0/dScaleFactorY)); //square or wide+low
      iMarginWidth /= 0.9; //this comes from the old scaling by m_dXmpc=0.9. Drop in new scheme?
      if (GetLongParameter(LP_GEOMETRY)==3) {
        //make whole screen logarithmic (but keep xhair in same place)
        myint crosshair(xmap(2048)); //should be 2048...
        m_iXlogThres=0;
        dDasherXPerPixel *= xmap(2048)/static_cast<double>(crosshair);
      }
      dScaleFactorX = 0.9 / dDasherXPerPixel;
    }
  }
  iScaleFactorX = myint(dScaleFactorX * SCALE_FACTOR);
  iScaleFactorY = myint(dScaleFactorY * SCALE_FACTOR);

#ifdef DEBUG
  //test...
  for (screenint x=0; x<iScreenWidth; x++) {
    dasherint dx, dy;
    Screen2Dasher(x, 0, dx, dy);
    screenint fx, fy;
    Dasher2Screen(dx, dy, fx, fy);
    if (fx!=x)
      std::cout << "ERROR ScreenX " << x << " becomes " << dx << " back to " << fx << std::endl;;
  }
  for (screenint y=0; y<iScreenHeight; y++) {
    dasherint dx,dy;
    Screen2Dasher(0, y, dx, dy);
    screenint fx,fy;
    Dasher2Screen(dx, dy, fx, fy);
    if (fy!=y)
      std::cout << "ERROR ScreenY " << y << " becomes " << dy << " back to " << fy << std::endl;
  }
#endif

  //notify listeners that coordinates have changed...
  Observable<CDasherView*>::DispatchEvent(this);
}


inline myint CDasherViewSquare::CustomIDivScaleFactor(myint iNumerator) {
  // Integer division rounding away from zero

  long long int num, denom, quot, rem;
  myint res;

  num   = iNumerator;
  denom = SCALE_FACTOR;

  DASHER_ASSERT(denom != 0);

#ifdef HAVE_LLDIV
  lldiv_t ans = ::lldiv(num, denom);

  quot = ans.quot;
  rem  = ans.rem;
#else
  quot = num / denom;
  rem  = num % denom;
#endif

  if (rem < 0)
    res = quot - 1;
  else if (rem > 0)
    res = quot + 1;
  else
    res = quot;

  return res;

  // return (iNumerator + iDenominator - 1) / iDenominator;
}

void CDasherViewSquare::Dasher2Screen(myint iDasherX, myint iDasherY, screenint &iScreenX, screenint &iScreenY) {

  // Apply the nonlinearities

  iDasherX = xmap(iDasherX);
  iDasherY = ymap(iDasherY);

  // Things we're likely to need:

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  // Note that integer division is rounded *away* from zero here to
  // ensure that this really is the inverse of the map the other way
  // around.

  switch( GetOrientation() ) {
  case Dasher::Opts::LeftToRight:
    iScreenX = screenint(iScreenWidth -
			 CustomIDivScaleFactor(iDasherX  * iScaleFactorX));
    iScreenY = screenint(iScreenHeight / 2 +
			 CustomIDivScaleFactor(( iDasherY - CDasherModel::MAX_Y / 2 ) * iScaleFactorY));
    break;
  case Dasher::Opts::RightToLeft:
    iScreenX = screenint(CustomIDivScaleFactor(iDasherX * iScaleFactorX));
    iScreenY = screenint(iScreenHeight / 2 +
			 CustomIDivScaleFactor( (iDasherY - CDasherModel::MAX_Y/2) * iScaleFactorY));
    break;
  case Dasher::Opts::TopToBottom:
    iScreenX = screenint(iScreenWidth / 2 +
			 CustomIDivScaleFactor( (iDasherY - CDasherModel::MAX_Y/2) * iScaleFactorY));
    iScreenY = screenint(iScreenHeight -
			 CustomIDivScaleFactor( iDasherX * iScaleFactorX ));
    break;
  case Dasher::Opts::BottomToTop:
    iScreenX = screenint(iScreenWidth / 2 +
			 CustomIDivScaleFactor(( iDasherY - CDasherModel::MAX_Y/2 ) * iScaleFactorY));
    iScreenY = screenint(CustomIDivScaleFactor( iDasherX  * iScaleFactorX ));
    break;
    default:
      break;
  }
}

void CDasherViewSquare::Dasher2Polar(myint iDasherX, myint iDasherY, double &r, double &theta) {
	iDasherX = xmap(iDasherX);
    iDasherY = ymap(iDasherY);

  myint iDasherOX = xmap(CDasherModel::ORIGIN_X);
    myint iDasherOY = ymap(CDasherModel::ORIGIN_Y);

    double x = -(iDasherX - iDasherOX) / double(iDasherOX); //Use normalised coords so min r works
    double y = -(iDasherY - iDasherOY) / double(iDasherOY);
    theta = atan2(y, x);
    r = sqrt(x * x + y * y);
}

void CDasherViewSquare::DasherLine2Screen(myint x1, myint y1, myint x2, myint y2, vector<CDasherScreen::point> &vPoints) {
  if (x1!=x2 && y1!=y2) { //only diagonal lines ever get changed...
    if (GetBoolParameter(BP_NONLINEAR_Y)) {
      if ((y1 < m_Y3 && y2 > m_Y3) ||(y2 < m_Y3 && y1 > m_Y3)) {
        //crosses bottom non-linearity border
        myint x_mid = x1+(x2-x1) * (m_Y3-y1)/(y2-y1);
        DasherLine2Screen(x1, y1, x_mid, m_Y3, vPoints);
        x1=x_mid; y1=m_Y3;
      }//else //no, a single line might cross _both_ borders!
      if ((y1 > m_Y2 && y2 < m_Y2) || (y2 > m_Y2 && y1 < m_Y2)) {
        //crosses top non-linearity border
        myint x_mid = x1 + (x2-x1) * (m_Y2-y1)/(y2-y1);
        DasherLine2Screen(x1, y1, x_mid, m_Y2, vPoints);
        x1=x_mid; y1=m_Y2;
      }
    }
    if (GetLongParameter(LP_NONLINEAR_X) && (x1 > m_iXlogThres || x2 > m_iXlogThres)) {
      //into logarithmic section
      CDasherScreen::point pStart, pScreenMid, pEnd;
      Dasher2Screen(x2, y2, pEnd.x, pEnd.y);
      for(;;) {
        Dasher2Screen(x1, y1, pStart.x, pStart.y);
        //a straight line on the screen between pStart and pEnd passes through pScreenMid:
        pScreenMid.x = (pStart.x + pEnd.x)/2;
        pScreenMid.y = (pStart.y + pEnd.y)/2;
        //whereas a straight line _in_Dasher_space_ passes through pDasherMid:
        myint xMid=(x1+x2)/2, yMid=(y1+y2)/2;
        CDasherScreen::point pDasherMid;
        Dasher2Screen(xMid, yMid, pDasherMid.x, pDasherMid.y);

        //since we know both endpoints are in the same section of the screen wrt. Y nonlinearity,
        //the midpoint along the DasherY axis of both lines should be the same.
        if (GetOrientation()==Dasher::Opts::LeftToRight || GetOrientation()==Dasher::Opts::RightToLeft) {
          DASHER_ASSERT(abs(pDasherMid.y - pScreenMid.y)<=1);//allow for rounding error
          if (abs(pDasherMid.x - pScreenMid.x)<=1) break; //call a straight line accurate enough
        } else {
          DASHER_ASSERT(abs(pDasherMid.x - pScreenMid.x)<=1);
          if (abs(pDasherMid.y - pScreenMid.y)<=1) break;
        }
        //line should appear bent. Subdivide!
        DasherLine2Screen(x1,y1,xMid,yMid,vPoints); //recurse for first half (to Dasher-space midpoint)
        if (x1==xMid || y1 == yMid) break; // as test on entry, only diagonal lines need to be bent...
        x1=xMid; y1=yMid; //& loop round for second half
      }
      //broke out of loop. a straight line (x1,y1)-(x2,y2) on the screen is an accurate portrayal of a straight line in Dasher-space.
      vPoints.push_back(pEnd);
      return;
    }
    //ok, not in x nonlinear section; fall through.
  }
#ifdef DEBUG
  CDasherScreen::point pTest;
  Dasher2Screen(x1, y1, pTest.x, pTest.y);
  DASHER_ASSERT(vPoints.back().x == pTest.x && vPoints.back().y == pTest.y);
#endif
  CDasherScreen::point p;
  Dasher2Screen(x2, y2, p.x, p.y);
  vPoints.push_back(p);
}

void CDasherViewSquare::VisibleRegion( myint &iDasherMinX, myint &iDasherMinY, myint &iDasherMaxX, myint &iDasherMaxY ) {
  // TODO: Change output parameters to pointers and allow NULL to mean
  // 'I don't care'. Need to be slightly careful about this as it will
  // require a slightly more sophisticated caching mechanism

  if(!m_bVisibleRegionValid) {

    switch( GetOrientation() ) {
    case Dasher::Opts::LeftToRight:
      Screen2Dasher(Screen()->GetWidth(),0,m_iDasherMinX,m_iDasherMinY);
      Screen2Dasher(0,Screen()->GetHeight(),m_iDasherMaxX,m_iDasherMaxY);
      break;
    case Dasher::Opts::RightToLeft:
      Screen2Dasher(0,0,m_iDasherMinX,m_iDasherMinY);
      Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),m_iDasherMaxX,m_iDasherMaxY);
      break;
    case Dasher::Opts::TopToBottom:
      Screen2Dasher(0,Screen()->GetHeight(),m_iDasherMinX,m_iDasherMinY);
      Screen2Dasher(Screen()->GetWidth(),0,m_iDasherMaxX,m_iDasherMaxY);
      break;
    case Dasher::Opts::BottomToTop:
      Screen2Dasher(0,0,m_iDasherMinX,m_iDasherMinY);
      Screen2Dasher(Screen()->GetWidth(),Screen()->GetHeight(),m_iDasherMaxX,m_iDasherMaxY);
      break;
    default:
      break;
    }

    m_bVisibleRegionValid = true;
  }

  iDasherMinX = m_iDasherMinX;
  iDasherMaxX = m_iDasherMaxX;
  iDasherMinY = m_iDasherMinY;
  iDasherMaxY = m_iDasherMaxY;
}

// void CDasherViewSquare::NewDrawGoTo(myint iDasherMin, myint iDasherMax, bool bActive) {
//   myint iHeight(iDasherMax - iDasherMin);

//   int iColour;
//   int iWidth;

//   if(bActive) {
//     iColour = 1;
//     iWidth = 3;
//   }
//   else {
//     iColour = 2;
//     iWidth = 1;
//   }

//   CDasherScreen::point p[4];

//   Dasher2Screen( 0, iDasherMin, p[0].x, p[0].y);
//   Dasher2Screen( iHeight, iDasherMin, p[1].x, p[1].y);
//   Dasher2Screen( iHeight, iDasherMax, p[2].x, p[2].y);
//   Dasher2Screen( 0, iDasherMax, p[3].x, p[3].y);

//   Screen()->Polyline(p, 4, iWidth, iColour);
// }

void CDasherViewSquare::ScreenResized(CDasherScreen *NewScreen) {
  m_bVisibleRegionValid = false;
  SetScaleFactor();
}
