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

//#include "DasherGameMode.h"
#include "DasherViewSquare.h"
#include "DasherModel.h"
#include "DasherView.h"
#include "DasherTypes.h"
#include "Event.h"
#include "EventHandler.h"

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

CDasherViewSquare::CDasherViewSquare(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherScreen *DasherScreen)
: CDasherView(pEventHandler, pSettingsStore, DasherScreen),   m_Y1(4), m_Y2(0.95 * GetLongParameter(LP_MAX_Y)), m_Y3(0.05 * GetLongParameter((LP_MAX_Y))) {

  // TODO - AutoOffset should be part of the eyetracker input filter
  // Make sure that the auto calibration is set to zero berfore we start
  //  m_yAutoOffset = 0;

  ChangeScreen(DasherScreen);

  //Note, nonlinearity parameters set in SetScaleFactor 
  m_bVisibleRegionValid = false;
  
}

CDasherViewSquare::~CDasherViewSquare() {}

void CDasherViewSquare::HandleEvent(Dasher::CEvent *pEvent) {
  // Let the parent class do its stuff
  CDasherView::HandleEvent(pEvent);

  // And then interpret events for ourself
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    switch (pEvt->m_iParameter) {
    case LP_REAL_ORIENTATION:
    case LP_MARGIN_WIDTH:
    case BP_NONLINEAR_Y:
    case LP_NONLINEAR_X:
      m_bVisibleRegionValid = false;
      SetScaleFactor();
      break;
    default:
      break;
    }
  }
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
    
    Screen2Dasher(newleft2, newtop2, iDasherNewLeft, iDasherNewTop);
    Screen2Dasher(newright2, newbottom2, iDasherNewRight, iDasherNewBottom);
    
    mostleft = std::min(iDasherNewRight, iDasherNewLeft);
  }
  
  // Actually draw the text. We use DelayDrawText as the text should
  // be overlayed once all of the boxes have been drawn.
  
  m_DelayDraw.DelayDrawText(sDisplayText, newleft2, newtop2, Size);
}

void CDasherViewSquare::RenderNodes(CDasherNode *pRoot, myint iRootMin, myint iRootMax,
				    CExpansionPolicy &policy) {
  DASHER_ASSERT(pRoot != 0);
  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);
  //

  screenint iScreenLeft;
  screenint iScreenTop;
  screenint iScreenRight;
  screenint iScreenBottom;

  Dasher2Screen(iRootMax-iRootMin, iRootMin, iScreenLeft, iScreenTop);
  Dasher2Screen(0, iRootMax, iScreenRight, iScreenBottom);

  //ifiScreenTop < 0)
  //  iScreenTop = 0;

  //if(iScreenLeft < 0) 
  //  iScreenLeft=0;

  //// TODO: Should these be right on the boundary?
  //if(iScreenBottom > Screen()->GetHeight()) 
  //  iScreenBottom=Screen()->GetHeight();

  //if(iScreenRight > Screen()->GetWidth()) 
  //  iScreenRight=Screen()->GetWidth();

  // Blank the region around the root node:
  
  if(iRootMin > iDasherMinY)
    DasherDrawRectangle(iDasherMaxX, iDasherMinY, iDasherMinX, iRootMin, 0, -1, Nodes1, 0);
  
  //if(iScreenTop > 0)
  //  Screen()->DrawRectangle(0, 0, Screen()->GetWidth(), iScreenTop, 0, -1, Nodes1, false, true, 1);

  if(iRootMax < iDasherMaxY)
    DasherDrawRectangle(iDasherMaxX, iRootMax, iDasherMinX, iDasherMaxY, 0, -1, Nodes1, 0);

  //if(iScreenBottom <= Screen()->GetHeight())
  // Screen()->DrawRectangle(0, iScreenBottom, Screen()->GetWidth(), Screen()->GetHeight(), 0, -1, Nodes1, false, true, 1);

  DasherDrawRectangle(0, iDasherMinY, iDasherMinX, iDasherMaxY, 0, -1, Nodes1, 0);
  //  Screen()->DrawRectangle(iScreenRight, std::max(0, (int)iScreenTop),
  //		  Screen()->GetWidth(), std::min(Screen()->GetHeight(), (int)iScreenBottom), 
  //		  0, -1, Nodes1, false, true, 1);

  // Render the root node (and children)
  RecursiveRender(pRoot, iRootMin, iRootMax, iDasherMaxX, policy, std::numeric_limits<double>::infinity(), iDasherMaxX,0,0);

  // Labels are drawn in a second parse to get the overlapping right
  m_DelayDraw.Draw(Screen());

  // Finally decorate the view
  Crosshair((myint)GetLongParameter(LP_OX));
}

//min size in *Dasher co-ordinates* to consider rendering a node
#define QUICK_REJECT 50
//min size in *screen* (pixel) co-ordinates to render a node
#define MIN_SIZE 2

bool CDasherViewSquare::CheckRender(CDasherNode *pRender, myint y1, myint y2,
									int mostleft, CExpansionPolicy &policy, double dMaxCost,
									myint parent_width, int parent_color, int iDepth)
{
  if (y2-y1 >= QUICK_REJECT)
  {
    myint iDasherMinX;
    myint iDasherMinY;
    myint iDasherMaxX;
    myint iDasherMaxY;
    VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);
	
    if (y1 <= iDasherMaxY && y2 >= iDasherMinY)
	{
      screenint iScreenX1;
      screenint iScreenY1;
      screenint iScreenX2;
      screenint iScreenY2;
	
      Dasher2Screen(0, std::max(y1, iDasherMinY), iScreenX1, iScreenY1);
      Dasher2Screen(0, std::min(y2, iDasherMaxY), iScreenX2, iScreenY2);
	
      Cint32 iHeight = std::max(myint(iScreenY2 - iScreenY1),myint( 0));
  
      if (iHeight >= MIN_SIZE)
	  {
		  //node should be rendered!
		  
		  RecursiveRender(pRender, y1, y2, mostleft, policy, dMaxCost, parent_width, parent_color, iDepth);
		  return true;
	  }
	}
  }
  // We get here if the node is too small to render or is off-screen.
  // So, collapse it immediately.
  // 
  // In game mode, we get here if the child is too small to draw, but we need the
  // coordinates - if this is the case then we shouldn't delete any children.
  //
  // TODO: Should probably render the parent segment here anyway (or
  // in the above)
  if(!pRender->GetFlag(NF_GAME))
    pRender->Delete_children();
  return false;
}

void CDasherViewSquare::RecursiveRender(CDasherNode *pRender, myint y1, myint y2,
					int mostleft, CExpansionPolicy &policy, double dMaxCost,
					myint parent_width,int parent_color, int iDepth)
{
  DASHER_ASSERT_VALIDPTR_RW(pRender);

//   if(iDepth == 2)
//     std::cout << pRender->GetDisplayInfo()->strDisplayText << std::endl;

  // TODO: We need an overhall of the node creation/deletion logic -
  // make sure that we only maintain the minimum number of nodes which
  // are actually needed.  This is especially true at the moment in
  // Game mode, which feels very sluggish. Node creation also feels
  // slower in Windows than Linux, especially if many nodes are
  // created at once (eg untrained Hiragana)

  ++m_iRenderCount;
 
  //  myint trange = y2 - y1;

  // Attempt to draw the region to the left of this node inside its parent. 
  

//   if(iDepth == 2) {
//     std::cout << "y1: " << y1 << " y2: " << y2 << std::endl;

  // Set the NF_SUPER flag if this node entirely frames the visual
  // area.
  
  // TODO: too slow?
  // TODO: use flags more rather than delete/reparent lists
  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  DasherDrawRectangle(std::min(parent_width,iDasherMaxX), std::max(y1,iDasherMinY), std::min(y2-y1,iDasherMaxX), std::min(y2,iDasherMaxY), parent_color, -1, Nodes1, 0);
  
  const std::string &sDisplayText(pRender->GetDisplayInfo()->strDisplayText);
  if( sDisplayText.size() > 0 )
  {  
    DasherDrawText(y2-y1, y1, y2-y1, y2, sDisplayText, mostleft, pRender->GetDisplayInfo()->bShove);
  }
	
	
  pRender->SetFlag(NF_SUPER, (y1 < iDasherMinY) && (y2 > iDasherMaxY) && ((y2 - y1) > iDasherMaxX));

  // If there are no children then we still need to render the parent
  if(pRender->ChildCount() == 0) {
    DasherDrawRectangle(std::min(y2-y1,iDasherMaxX), std::min(y2,iDasherMaxY),0, std::max(y1,iDasherMinY), pRender->GetDisplayInfo()->iColour, -1, Nodes1, 0);
	  //also allow it to be expanded, it's big enough.
	  policy.pushNode(pRender, y1, y2, true, dMaxCost);
	  return;
  }

  //Node has children. It can therefore be collapsed...however,
  // we don't allow a node covering the crosshair to be collapsed
  // (at best this'll mean there's nowhere useful to go forwards;
  // at worst, all kinds of crashes trying to do text output!)
  if (!pRender->GetFlag(NF_GAME) && !pRender->GetFlag(NF_SEEN))
    dMaxCost = policy.pushNode(pRender, y1, y2, false, dMaxCost);
	
  // Render children  
  int norm = (myint)GetLongParameter(LP_NORMALIZATION);

  myint lasty=y1;
  
  
  int id=-1;
  //  int lower=-1,upper=-1;
  myint temp_parentwidth=y2-y1;
  int temp_parentcolor = pRender->GetDisplayInfo()->iColour;

  const myint Range(y2 - y1);
  
  if (CDasherNode *pChild = pRender->onlyChildRendered)
  {
    //if child still covers screen, render _just_ it and return
    myint newy1 = y1 + (Range * (myint)pChild->Lbnd()) / (myint)norm;
    myint newy2 = y1 + (Range * (myint)pChild->Hbnd()) / (myint)norm;
    if (newy1 < iDasherMinY && newy2 > iDasherMaxY) {
      //still covers entire screen. Parent should too...
      DASHER_ASSERT(dMaxCost == std::numeric_limits<double>::infinity());
      
      //don't inc iDepth, meaningless when covers the screen
      RecursiveRender(pChild, newy1, newy2, mostleft, 
						policy, dMaxCost,
						temp_parentwidth, temp_parentcolor, iDepth);
      //leave pRender->onlyChildRendered set, so remaining children are skipped
    }
    else
      pRender->onlyChildRendered = NULL;
  }
	
  if (!pRender->onlyChildRendered)
  { //render all children...
	  for(CDasherNode::ChildMap::const_iterator i = pRender->GetChildren().begin();
		  i != pRender->GetChildren().end(); i++) {
		id++;
		CDasherNode *pChild = *i;
		
		myint newy1 = y1 + (Range * (myint)pChild->Lbnd()) / (myint)norm;/// norm and lbnd are simple ints
		myint newy2 = y1 + (Range * (myint)pChild->Hbnd()) / (myint)norm;
    if (newy1 < iDasherMinY && newy2 > iDasherMaxY) {
      DASHER_ASSERT(dMaxCost == std::numeric_limits<double>::infinity());
      pRender->onlyChildRendered = pChild;
      RecursiveRender(pChild, newy1, newy2, mostleft, policy, dMaxCost, temp_parentwidth, temp_parentcolor, iDepth);
      //ensure we don't blank over this child in "finishing off" the parent (!)
      lasty=newy2;
      break; //no need to render any more children!
    }
		if (CheckRender(pChild, newy1, newy2, mostleft, policy, dMaxCost,
						temp_parentwidth, temp_parentcolor, iDepth+1))
		{
		
          if (lasty<newy1) {
			//if child has been drawn then the interval between him and the
			//last drawn child should be drawn too.
			//std::cout << "Fill in: " << lasty << " " << newy1 << std::endl;
		  
			RenderNodePartFast(temp_parentcolor, lasty, newy1, mostleft, 
					 pRender->GetDisplayInfo()->strDisplayText, 
					 pRender->GetDisplayInfo()->bShove,
					 temp_parentwidth);
          }
		
		  lasty = newy2;
		}
	  }
  
	  // Finish off the drawing process

	  //  if(iDepth == 1) {
		// Theres a chance that we haven't yet filled the entire parent, so finish things off if necessary.
		if(lasty<y2) {
		  RenderNodePartFast(temp_parentcolor, lasty, y2, mostleft, 
				 pRender->GetDisplayInfo()->strDisplayText, 
				 pRender->GetDisplayInfo()->bShove,
				 temp_parentwidth);
		}
  }  
    // Draw the outline
    if(pRender->GetDisplayInfo()->bVisible) {
      RenderNodeOutlineFast(pRender->GetDisplayInfo()->iColour, 
			    y1, y2, mostleft, 
			    pRender->GetDisplayInfo()->strDisplayText, 
			    pRender->GetDisplayInfo()->bShove);
    }
    //  }
}

bool CDasherViewSquare::IsNodeVisible(myint y1, myint y2) {
  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;

  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  return ((y2 - y1) < iDasherMaxX) || ((y1 > iDasherMinY) && (y2 < iDasherMaxY));
}

// Draw the outline of a node
int CDasherViewSquare::RenderNodeOutlineFast(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove) {

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

  Cint32 iHeight = std::max(myint(iScreenY2 - iScreenY1),myint( 0));
  Cint32 iWidth = std::max(myint(iScreenX2 - iScreenX1),myint( 0));

  if((iHeight <= 1) && (iWidth <= 1))
    return 0;                   // We're too small to render

  if((y1 > iDasherMaxY) || (y2 < iDasherMinY)){
    return 0;                   // We're entirely off screen, so don't render.
  }

  // TODO: This should be earlier?
  if(!GetBoolParameter(BP_OUTLINE_MODE))
    return 1;

  myint iDasherSize(y2 - y1);
 


  //  std::cout << std::min(iDasherSize,iDasherMaxX) << " " << std::min(y2,iDasherMaxY) << " 0 " << std::max(y1,iDasherMinY) << std::endl;

  DasherDrawRectangle(0, std::min(y1,iDasherMaxY),std::min(iDasherSize,iDasherMaxX), std::max(y2,iDasherMinY), -1, Color, Nodes1, 1);

//   // FIXME - get rid of pointless assignment below

//   int iTruncation(GetLongParameter(LP_TRUNCATION));     // Trucation farction times 100;

//   if(iTruncation == 0) {        // Regular squares

//   }
//   else { 
//     // TODO: Put something back here?
//   }

  return 1;
}


// Draw a filled block of a node right down to the baseline (intended
// for the case when you have no visible child)
int CDasherViewSquare::RenderNodePartFast(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove,myint iParentWidth ) {

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

  // std::cout << "Fill in components: " <<  iScreenY1 << " " << iScreenY2 << std::endl;

  Cint32 iHeight = std::max(myint(iScreenY2 - iScreenY1),myint( 0));
  Cint32 iWidth = std::max(myint(iScreenX2 - iScreenX1),myint( 0));

  if((iHeight < 1) && (iWidth < 1)) {
    // std::cout << "b" << std::endl;
    return 0;                   // We're too small to render
  }

  if((y1 > iDasherMaxY) || (y2 < iDasherMinY)){
    //std::cout << "a" << std::endl;
    return 0;                   // We're entirely off screen, so don't render.
  }

  DasherDrawRectangle(std::min(iParentWidth,iDasherMaxX), std::min(y2,iDasherMaxY),0, std::max(y1,iDasherMinY), Color, -1, Nodes1, 0);
  
  return 1;
}

/// Convert screen co-ordinates to dasher co-ordinates. This doesn't
/// include the nonlinear mapping for eyetracking mode etc - it is
/// just the inverse of the mapping used to calculate the screen
/// positions of boxes etc.

void CDasherViewSquare::Screen2Dasher(screenint iInputX, screenint iInputY, myint &iDasherX, myint &iDasherY) {

  // Things we're likely to need:

  //myint iDasherWidth = (myint)GetLongParameter(LP_MAX_Y);
  myint iDasherHeight = (myint)GetLongParameter(LP_MAX_Y);

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  int eOrientation(GetLongParameter(LP_REAL_ORIENTATION));

  switch(eOrientation) {
  case Dasher::Opts::LeftToRight:
    iDasherX = iCenterX - ( iInputX - iScreenWidth / 2 ) * m_iScalingFactor / iScaleFactorX;
    iDasherY = iDasherHeight / 2 + ( iInputY - iScreenHeight / 2 ) * m_iScalingFactor / iScaleFactorY;
    break;
  case Dasher::Opts::RightToLeft:
    iDasherX = myint(iCenterX + ( iInputX - iScreenWidth / 2 ) * m_iScalingFactor/ iScaleFactorX);
    iDasherY = myint(iDasherHeight / 2 + ( iInputY - iScreenHeight / 2 ) * m_iScalingFactor/ iScaleFactorY);
    break;
  case Dasher::Opts::TopToBottom:
    iDasherX = myint(iCenterX - ( iInputY - iScreenHeight / 2 ) * m_iScalingFactor/ iScaleFactorY);
    iDasherY = myint(iDasherHeight / 2 + ( iInputX - iScreenWidth / 2 ) * m_iScalingFactor/ iScaleFactorX);
    break;
  case Dasher::Opts::BottomToTop:
    iDasherX = myint(iCenterX + ( iInputY - iScreenHeight / 2 ) * m_iScalingFactor/ iScaleFactorY);
    iDasherY = myint(iDasherHeight / 2 + ( iInputX - iScreenWidth / 2 ) * m_iScalingFactor/ iScaleFactorX);
    break;
  }

  iDasherX = ixmap(iDasherX);
  iDasherY = iymap(iDasherY);
  
}

void CDasherViewSquare::SetScaleFactor( void )
{
  //Parameters for X non-linearity.
  // Set some defaults here, in case we change(d) them later...
  m_dXmpb = 0.5; //threshold: DasherX's less than (m_dXmpb * MAX_Y) are linear...
  m_dXmpc = 0.9; //...but multiplied by m_dXmpc; DasherX's above that, are logarithmic...

  //set log scaling coefficient (unused if LP_NONLINEAR_X==0)
  // note previous value of m_dXmpa = 0.2, i.e. a value of LP_NONLINEAR_X =~= 4.8
  m_dXmpa = exp(GetLongParameter(LP_NONLINEAR_X)/-3.0); 
  
  myint iDasherWidth = (myint)GetLongParameter(LP_MAX_Y);
  myint iDasherHeight = iDasherWidth;

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  // Try doing this a different way:

  myint iDasherMargin( GetLongParameter(LP_MARGIN_WIDTH) ); // Make this a parameter

  myint iMinX( 0-iDasherMargin );
  myint iMaxX( iDasherWidth );
  iCenterX = (iMinX + iMaxX)/2;
  myint iMinY( 0 );
  myint iMaxY( iDasherHeight );

  Dasher::Opts::ScreenOrientations eOrientation(Dasher::Opts::ScreenOrientations(GetLongParameter(LP_REAL_ORIENTATION)));
  
  double dScaleFactorX, dScaleFactorY;
  
  if (eOrientation == Dasher::Opts::LeftToRight || eOrientation == Dasher::Opts::RightToLeft) {
    dScaleFactorX = iScreenWidth / static_cast<double>( iMaxX - iMinX );
    dScaleFactorY = iScreenHeight / static_cast<double>( iMaxY - iMinY );
  } else {
    dScaleFactorX = iScreenHeight / static_cast<double>( iMaxX - iMinX );
    dScaleFactorY = iScreenWidth / static_cast<double>( iMaxY - iMinY );
  }
  
  if (dScaleFactorX < dScaleFactorY) {
    //fewer (pixels per dasher coord) in X direction - i.e., X is more compressed.
    //So, use X scale for Y too...except first, we'll _try_ to reduce the difference
    // by changing the relative scaling of X and Y (by at most 20%):
    double dMul = max(0.8, dScaleFactorX / dScaleFactorY);
    m_dXmpc *= dMul;
    dScaleFactorX /= dMul;

    iScaleFactorX = myint(dScaleFactorX * m_iScalingFactor);
    iScaleFactorY = myint(std::max(dScaleFactorX, dScaleFactorY / 4.0) * m_iScalingFactor);
  } else {
    //X has more room; use Y scale for both -> will get lots history
    iScaleFactorX = myint(std::max(dScaleFactorY, dScaleFactorX / 4.0) * m_iScalingFactor);
    iScaleFactorY = myint(dScaleFactorY * m_iScalingFactor);
    // however, "compensate" by relaxing the default "relative scaling" of X
    // (normally only 90% of Y) towards 1...
    m_dXmpc = std::min(1.0,0.9 * dScaleFactorX / dScaleFactorY);
  }
  iCenterX *= m_dXmpc;
}


inline myint CDasherViewSquare::CustomIDiv(myint iNumerator, myint iDenominator) { 
  // Integer division rounding away from zero

  long long int num, denom, quot, rem;
  myint res;

  num   = iNumerator;
  denom = iDenominator;

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

  //myint iDasherWidth = (myint)GetLongParameter(LP_MAX_Y);
  myint iDasherHeight = (myint)GetLongParameter(LP_MAX_Y);

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  int eOrientation( GetLongParameter(LP_REAL_ORIENTATION) );

  // Note that integer division is rounded *away* from zero here to
  // ensure that this really is the inverse of the map the other way
  // around.

  switch( eOrientation ) {
  case Dasher::Opts::LeftToRight:
    iScreenX = screenint(iScreenWidth / 2 - 
			 CustomIDiv((( iDasherX - iCenterX ) * iScaleFactorX), m_iScalingFactor));
    iScreenY = screenint(iScreenHeight / 2 +
			 CustomIDiv(( iDasherY - iDasherHeight / 2 ) * iScaleFactorY, m_iScalingFactor));
    break;
  case Dasher::Opts::RightToLeft:
    iScreenX = screenint(iScreenWidth / 2 + 
			 CustomIDiv(( iDasherX - iCenterX ) * iScaleFactorX, m_iScalingFactor));
    iScreenY = screenint(iScreenHeight / 2 + 
			 CustomIDiv(( iDasherY - iDasherHeight / 2 ) * iScaleFactorY, m_iScalingFactor));
    break;
  case Dasher::Opts::TopToBottom:
    iScreenX = screenint(iScreenWidth / 2 + 
			 CustomIDiv(( iDasherY - iDasherHeight / 2 ) * iScaleFactorX, m_iScalingFactor));
    iScreenY = screenint(iScreenHeight / 2 - 
			 CustomIDiv(( iDasherX - iCenterX ) * iScaleFactorY, m_iScalingFactor));
    break;
  case Dasher::Opts::BottomToTop:
    iScreenX = screenint(iScreenWidth / 2 + 
			 CustomIDiv(( iDasherY - iDasherHeight / 2 ) * iScaleFactorX, m_iScalingFactor));
    iScreenY = screenint(iScreenHeight / 2 + 
			 CustomIDiv(( iDasherX - iCenterX ) * iScaleFactorY, m_iScalingFactor));
    break;
  }
}

void CDasherViewSquare::Dasher2Polar(myint iDasherX, myint iDasherY, double &r, double &theta) {
	iDasherX = xmap(iDasherX);
    iDasherY = ymap(iDasherY);
	
  myint iDasherOX = xmap(GetLongParameter(LP_OX));
    myint iDasherOY = ymap(GetLongParameter(LP_OY));
	
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
        int x_mid = x1+(x2-x1) * (m_Y3-y1)/(y2-y1);
        DasherLine2Screen(x1, y1, x_mid, m_Y3, vPoints);
        x1=x_mid; y1=m_Y3;
      }//else //no, a single line might cross _both_ borders!
      if ((y1 > m_Y2 && y2 < m_Y2) || (y2 > m_Y2 && y1 < m_Y2)) {
        //crosses top non-linearity border
        int x_mid = x1 + (x2-x1) * (m_Y2-y1)/(y2-y1);
        DasherLine2Screen(x1, y1, x_mid, m_Y2, vPoints);
        x1=x_mid; y1=m_Y2;
      }
    }
    double dMax(static_cast<double>(GetLongParameter(LP_MAX_Y)));
    if (GetLongParameter(LP_NONLINEAR_X) && (x1 / dMax > m_dXmpb || x2 / dMax > m_dXmpb)) {
      //into logarithmic section
      CDasherScreen::point pStart, pScreenMid, pEnd;
      Dasher2Screen(x2, y2, pEnd.x, pEnd.y);
      for(;;) {
        Dasher2Screen(x1, y1, pStart.x, pStart.y);
        //a straight line on the screen between pStart and pEnd passes through pScreenMid:
        pScreenMid.x = (pStart.x + pEnd.x)/2;
        pScreenMid.y = (pStart.y + pEnd.y)/2;
        //whereas a straight line _in_Dasher_space_ passes through pDasherMid:
        int xMid=(x1+x2)/2, yMid=(y1+y2)/2;
        CDasherScreen::point pDasherMid;
        Dasher2Screen(xMid, yMid, pDasherMid.x, pDasherMid.y);
        
        //since we know both endpoints are in the same section of the screen wrt. Y nonlinearity,
        //the midpoint along the DasherY axis of both lines should be the same.
        const Dasher::Opts::ScreenOrientations orient(Dasher::Opts::ScreenOrientations(GetLongParameter(LP_REAL_ORIENTATION)));
        if (orient==Dasher::Opts::LeftToRight || orient==Dasher::Opts::RightToLeft) {
          DASHER_ASSERT(abs(pDasherMid.y - pScreenMid.y)<=1);//allow for rounding error
          if (abs(pDasherMid.x - pScreenMid.x)<=1) break; //call a straight line accurate enough
        } else {
          DASHER_ASSERT(abs(pDasherMid.x - pScreenMid.x)<=1);
          if (abs(pDasherMid.y - pScreenMid.y)<=1) break;
        }
        //line should appear bent. Subdivide!
        DasherLine2Screen(x1,y1,xMid,yMid,vPoints); //recurse for first half (to Dasher-space midpoint)
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

    int eOrientation( GetLongParameter(LP_REAL_ORIENTATION) );
    
    switch( eOrientation ) {
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

void CDasherViewSquare::ChangeScreen(CDasherScreen *NewScreen) {
  CDasherView::ChangeScreen(NewScreen);
  m_bVisibleRegionValid = false;
  m_iScalingFactor = 100000000;
  SetScaleFactor();
}

