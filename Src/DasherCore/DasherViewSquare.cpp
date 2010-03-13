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
#include "DasherScreen.h"
#include "DasherView.h"
#include "DasherTypes.h"
#include "Event.h"
#include "EventHandler.h"
#include "View/DelayedDraw.h"

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
    case LP_MARGIN_WIDTH:
        m_bVisibleRegionValid = false;
        SetScaleFactor();
        break;
    default:
      break;
    }
  }
}

void CDasherViewSquare::RenderNodes(CDasherNode *pRoot, myint iRootMin, myint iRootMax,
				    CExpansionPolicy &policy,
				    std::vector<std::pair<myint,bool> > *pvGamePointer) {
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
    DasherDrawRectangle(iDasherMaxX, iDasherMinY, iDasherMinX, iRootMin, 0, 0, Nodes1, false,true, 1);
  
  //if(iScreenTop > 0)
  //  Screen()->DrawRectangle(0, 0, Screen()->GetWidth(), iScreenTop, 0, -1, Nodes1, false, true, 1);

  if(iRootMax < iDasherMaxY)
    DasherDrawRectangle(iDasherMaxX, iRootMax, iDasherMinX, iDasherMaxY, 0, 0, Nodes1, false,true, 1);

  //if(iScreenBottom <= Screen()->GetHeight())
  // Screen()->DrawRectangle(0, iScreenBottom, Screen()->GetWidth(), Screen()->GetHeight(), 0, -1, Nodes1, false, true, 1);

  DasherDrawRectangle(0, iDasherMinY, iDasherMinX, iDasherMaxY, 0, 4, Nodes1, false,true, 1);
  //  Screen()->DrawRectangle(iScreenRight, std::max(0, (int)iScreenTop),
  //		  Screen()->GetWidth(), std::min(Screen()->GetHeight(), (int)iScreenBottom), 
  //		  0, -1, Nodes1, false, true, 1);

  // Render the root node (and children)
  RecursiveRender(pRoot, iRootMin, iRootMax, iDasherMaxX, policy, std::numeric_limits<double>::infinity(), pvGamePointer,iDasherMaxX,0,0);

  // Labels are drawn in a second parse to get the overlapping right
  m_pDelayDraw->Draw(Screen());

  // Finally decorate the view
  Crosshair((myint)GetLongParameter(LP_OX));
}

//min size in *Dasher co-ordinates* to consider rendering a node
#define QUICK_REJECT 50
//min size in *screen* (pixel) co-ordinates to render a node
#define MIN_SIZE 2

bool CDasherViewSquare::CheckRender(CDasherNode *pRender, myint y1, myint y2,
									int mostleft, CExpansionPolicy &policy, double dMaxCost,
									std::vector<std::pair<myint,bool> > *pvGamePointer,
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
		  
		  RecursiveRender(pRender, y1, y2, mostleft, policy, dMaxCost, pvGamePointer, parent_width, parent_color, iDepth);
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
					std::vector<std::pair<myint,bool> > *pvGamePointer,
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

  if(GetLongParameter(LP_TRUNCATION) == 0) {        // Regular squares
    DasherDrawRectangle(std::min(parent_width,iDasherMaxX), std::max(y1,iDasherMinY), std::min(y2-y1,iDasherMaxX), std::min(y2,iDasherMaxY), parent_color, -1, Nodes1, false, true, 1);
  }
	
  const std::string &sDisplayText(pRender->GetDisplayInfo()->strDisplayText);
  if( sDisplayText.size() > 0 )
  {  
    DasherDrawText(y2-y1, y1, y2-y1, y2, sDisplayText, mostleft, pRender->GetDisplayInfo()->bShove);
  }
	
	
  pRender->SetFlag(NF_SUPER, (y1 < iDasherMinY) && (y2 > iDasherMaxY) && ((y2 - y1) > iDasherMaxX));

  // If there are no children then we still need to render the parent
  if(pRender->ChildCount() == 0) {
	  int iTruncation(GetLongParameter(LP_TRUNCATION));     // Trucation farction times -x100;
	  //  int iTruncationType(GetLongParameter(LP_TRUNCATIONTYPE));
	  
	  if(iTruncation == 0) {        // Regular squares
		  DasherDrawRectangle(std::min(y2-y1,iDasherMaxX), std::min(y2,iDasherMaxY),0, std::max(y1,iDasherMinY), pRender->GetDisplayInfo()->iColour, -1, Nodes1, false, true, 1);
	  }
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
						policy, dMaxCost, pvGamePointer, 
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
      RecursiveRender(pChild, newy1, newy2, mostleft, policy, dMaxCost, pvGamePointer, temp_parentwidth, temp_parentcolor, iDepth);
      //ensure we don't blank over this child in "finishing off" the parent (!)
      lasty=newy2;
      break; //no need to render any more children!
    }
		if (CheckRender(pChild, newy1, newy2, mostleft, policy, dMaxCost,
						pvGamePointer, temp_parentwidth, temp_parentcolor, iDepth+1))
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

  DasherDrawRectangle(0, std::min(y1,iDasherMaxY),std::min(iDasherSize,iDasherMaxX), std::max(y2,iDasherMinY), Color, -1, Nodes1, true,false, 1);

//   // FIXME - get rid of pointless assignment below

//   int iTruncation(GetLongParameter(LP_TRUNCATION));     // Trucation farction times 100;
//   //  int iTruncationType(GetLongParameter(LP_TRUNCATIONTYPE));

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

  //  myint iDasherSize(y2 - y1);

  // FIXME - get rid of pointless assignment below

  int iTruncation(GetLongParameter(LP_TRUNCATION));     // Trucation farction times -x100;
  //  int iTruncationType(GetLongParameter(LP_TRUNCATIONTYPE));

  if(iTruncation == 0) {        // Regular squares
    DasherDrawRectangle(std::min(iParentWidth,iDasherMaxX), std::min(y2,iDasherMaxY),0, std::max(y1,iDasherMinY), Color, -1, Nodes1, false, true, 1);
  }
  else {
   
  }
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

  myint iScaleFactorX;
  myint iScaleFactorY;
  
  GetScaleFactor(eOrientation, &iScaleFactorX, &iScaleFactorY);

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

  if (GetBoolParameter(BP_NONLINEAR_Y)) {
    iDasherX = myint(ixmap(iDasherX / static_cast < double >(GetLongParameter(LP_MAX_Y))) * (myint)GetLongParameter(LP_MAX_Y));
    iDasherY = m_ymap.unmap(iDasherY);
  }
  
}

void CDasherViewSquare::SetScaleFactor( void )
{
  myint iDasherWidth = (myint)GetLongParameter(LP_MAX_Y);
  myint iDasherHeight = iDasherWidth;

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  // Try doing this a different way:

  myint iDasherMargin( GetLongParameter(LP_MARGIN_WIDTH) ); // Make this a parameter

  myint iMinX( 0-iDasherMargin );
  myint iMaxX( iDasherWidth - 2*iDasherMargin );
  iCenterX = (iMinX + iMaxX)/2;
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

  if (GetBoolParameter(BP_NONLINEAR_Y)) {
    iDasherX = myint(xmap(iDasherX / static_cast < double >(GetLongParameter(LP_MAX_Y))) * (myint)GetLongParameter(LP_MAX_Y));
    iDasherY = m_ymap.map(iDasherY);
  }

  // Things we're likely to need:

  //myint iDasherWidth = (myint)GetLongParameter(LP_MAX_Y);
  myint iDasherHeight = (myint)GetLongParameter(LP_MAX_Y);

  screenint iScreenWidth = Screen()->GetWidth();
  screenint iScreenHeight = Screen()->GetHeight();

  int eOrientation( GetLongParameter(LP_REAL_ORIENTATION) );

  myint iScaleFactorX;
  myint iScaleFactorY;

  GetScaleFactor( eOrientation, &iScaleFactorX, &iScaleFactorY);


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
	iDasherX = myint(xmap(iDasherX / static_cast < double >(GetLongParameter(LP_MAX_Y))) * GetLongParameter(LP_MAX_Y));
    iDasherY = myint(m_ymap.map(iDasherY));
	
    myint iDasherOX = myint(xmap(GetLongParameter(LP_OX) / static_cast < double >(GetLongParameter(LP_MAX_Y))) * GetLongParameter(LP_MAX_Y));
    myint iDasherOY = myint(m_ymap.map(GetLongParameter(LP_OY)));
	
    double x = -(iDasherX - iDasherOX) / double(iDasherOX); //Use normalised coords so min r works 
    double y = -(iDasherY - iDasherOY) / double(iDasherOY); 
    theta = atan2(y, x);
    r = sqrt(x * x + y * y);
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
  screenint Width = Screen()->GetWidth();
  screenint Height = Screen()->GetHeight();
  CanvasX = 9 * Width / 10;
  CanvasBorder = Width - CanvasX;
  CanvasY = Height;
  m_iScalingFactor = 100000000;
  SetScaleFactor();
}

