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
#include "View/DelayedDraw.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdlib.h>

using namespace Dasher;

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
    default:
      break;
    }
  }
}

void CDasherViewSquare::RenderNodes(CDasherNode *pRoot, myint iRootMin, myint iRootMax,
				    std::vector<CDasherNode *> &vNodeList, std::vector<CDasherNode *> &vDeleteList,
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

  // Blank the regoin around the root node:
  
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
  RecursiveRender(pRoot, iRootMin, iRootMax, iDasherMaxX, vNodeList, vDeleteList, pvGamePointer,true,iDasherMaxX,0,0);

  // Labels are drawn in a second parse to get the overlapping right
  m_pDelayDraw->Draw(Screen());

  // Finally decorate the view
  Crosshair((myint)GetLongParameter(LP_OX));
}

bool CDasherViewSquare::RecursiveRender(CDasherNode *pRender, myint y1, myint y2,
					int mostleft, std::vector<CDasherNode *> &vNodeList,
					std::vector<CDasherNode *> &vDeleteList,
					std::vector<std::pair<myint,bool> > *pvGamePointer,
					bool bDraw,myint parent_width,int parent_color, int iDepth)
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
 
  myint temp_parentwidth=parent_width;
  //  myint trange = y2 - y1;
  int temp_parentcolor=parent_color;

  // Attempt to draw the region to the left of this node inside its parent. 

  // TODO: Pass displayinfo directly here
  bool bDrewFather = false;

  

//   if(iDepth == 2) {
//     std::cout << "y1: " << y1 << " y2: " << y2 << std::endl;
    
  bDrewFather = RenderNodeFatherFast(parent_color, y1, y2, mostleft,
				     pRender->GetDisplayInfo()->strDisplayText,
				     pRender->GetDisplayInfo()->bShove,parent_width,
				     pRender->GetDisplayInfo()->bVisible);

  // TODO: This should be elsewhere
  
  //if(pRender->GetFlag(NF_GAME))// && !pRender->GetFlag(NF_SUBNODE))
  //  {

  //    pvGamePointer->push_back(std::pair<myint,bool>((y1 + y2) / 2, (bDrewFather&&bDraw)));
  //  }

  
  if(!bDraw || !bDrewFather)
    {
      // We get here if the node is too small to render, or shouldn't be
      // drawn for some other reason.
      // 
      // In game mode, we get here if the child is too small to draw, but we need the
      // coordinates - if this is the case then we shouldn't delete any children.
      //
      // TODO: Should probably render the parent segment here anyway (or
      // in the above)
      if(!pRender->GetFlag(NF_GAME))
	{
	  pRender->SetFlag(NF_ALIVE, false);
	  vDeleteList.push_back(pRender);
	}
      return false;
    }

  // Set the NF_SUPER flag if this node entirely frames the visual
  // area.
  
  // TODO: too slow?
  // TODO: use flags more rather than delete/reparent lists
  // pRender->SetFlag(NF_SUPER, !IsNodeVisible(y1, y2)); 
  myint iDasherMinX;
  myint iDasherMinY;
  myint iDasherMaxX;
  myint iDasherMaxY;
  VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

  pRender->SetFlag(NF_SUPER, (y1 < iDasherMinY) && (y2 > iDasherMaxY) && ((y2 - y1) > iDasherMaxX));

  // If there are no children then we still need to render the parent
  if(pRender->ChildCount() == 0) {
    
    RenderNodePartFast(pRender->GetDisplayInfo()->iColour, y1, y2, mostleft,
		       pRender->GetDisplayInfo()->strDisplayText,
		       pRender->GetDisplayInfo()->bShove,y2-y1);
    vNodeList.push_back(pRender);

    return true;  // CHANGED BY IGNAS. I return 1 when the child was rendered and in this case the 
    //child was rendered.
  }

  // Render children  
  int norm = (myint)GetLongParameter(LP_NORMALIZATION);

  myint lasty=y1;
  
  
  int id=-1;
  //  int lower=-1,upper=-1;
  temp_parentwidth=y2-y1;
  if(pRender->GetDisplayInfo()->bVisible)
    temp_parentcolor=pRender->GetDisplayInfo()->iColour;
  else
    temp_parentcolor=parent_color;

  // TODO: Is bDraw being handled correctly here? What is it even used for?
    
  for(CDasherNode::ChildMap::const_iterator i = pRender->GetChildren().begin();
      i != pRender->GetChildren().end(); i++) {
    id++;
    CDasherNode *pChild = *i;
    
    myint Range = y2 - y1;
    myint newy1 = y1 + (Range * (myint)pChild->Lbnd()) / (myint)norm;/// norm and lbnd are simple ints
    myint newy2 = y1 + (Range * (myint)pChild->Hbnd()) / (myint)norm;
    
    bool bDrawChild = (newy2 - newy1 > 50) || (pChild->GetFlag(NF_ALIVE));
    
    if(bDrawChild){
      
      pChild->SetFlag(NF_ALIVE, true);
      
      bool bChildRendered = RecursiveRender(pChild, newy1, newy2, mostleft, 
					    vNodeList, vDeleteList, pvGamePointer, 
					    bDrawChild, temp_parentwidth, 
					    temp_parentcolor, iDepth + 1);
      
      //       if(iDepth == 1) {
      if (bChildRendered) {
	//std::cout << lasty << " " << newy1 << " " << newy2 << std::endl;
	
	if ((bDraw)&&(lasty<newy1)) {
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
    //    }
  }
  
  // Finish off the drawing process

  //  if(iDepth == 1) {
  if (bDraw) {
    // Theres a chance that we haven't yet filled the entire parent, so finish things off if necessary.
    if(lasty<y2) {
      RenderNodePartFast(temp_parentcolor, lasty, y2, mostleft, 
			 pRender->GetDisplayInfo()->strDisplayText, 
			 pRender->GetDisplayInfo()->bShove,
			 temp_parentwidth);
    }
    
    // Draw the outline
    if(!(pRender->GetFlag(NF_SUBNODE))) {
      RenderNodeOutlineFast(pRender->GetDisplayInfo()->iColour, 
			    y1, y2, mostleft, 
			    pRender->GetDisplayInfo()->strDisplayText, 
			    pRender->GetDisplayInfo()->bShove);
    }
    //  }
}

  //  if(iDepth == 2)
    //    std::cout << "done." << std::endl;
  
  return true;
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

// Render a block of code as far as the child, used when the child is
// visible. TODO: Merge this with the function above
int CDasherViewSquare::RenderNodeFatherFast(const int parent_color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove,myint iParentWidth, bool bVisible) {

  // Commenting because click mode occasionally fails this assert.
  // I don't know why.  -- cjb.
  // Why does this return 1?  -- pconlon -- I'm changing it to 0.
  if (!(y2 >= y1)) { return 0; }

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

  myint iDasherSize(y2 - y1);

  // FIXME - get rid of pointless assignment below

  int iTruncation(GetLongParameter(LP_TRUNCATION));     // Trucation farction times 100;
  // int iTruncationType(GetLongParameter(LP_TRUNCATIONTYPE));

  if(iTruncation == 0) {        // Regular squares
    DasherDrawRectangle(std::min(iParentWidth,iDasherMaxX), std::max(y1,iDasherMinY), std::min(iDasherSize,iDasherMaxX), std::min(y2,iDasherMaxY), parent_color, -1, Nodes1, false, true, 1);
  }
  else { }
  myint iDasherAnchorX(iDasherSize);
  if( sDisplayText.size() > 0 )
  {  
	  DasherDrawText(iDasherAnchorX, y1, iDasherAnchorX, y2, sDisplayText, mostleft, bShove);
  }
  return 1;
}

// int CDasherViewSquare::RenderNode(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove) {

//   // Commenting because click mode occasionally fails this assert.
//   // I don't know why.  -- cjb.
//   if (!(y2 >= y1)) { return 1; }

//   // TODO - Get sensible limits here (to allow for non-linearities)
//   myint iDasherMinX;
//   myint iDasherMinY;
//   myint iDasherMaxX;
//   myint iDasherMaxY;

//   VisibleRegion(iDasherMinX, iDasherMinY, iDasherMaxX, iDasherMaxY);

//   screenint iScreenX1;
//   screenint iScreenY1;
//   screenint iScreenX2;
//   screenint iScreenY2;
  
//   Dasher2Screen(0, std::max(y1, iDasherMinY), iScreenX1, iScreenY1);
//   Dasher2Screen(0, std::min(y2, iDasherMaxY), iScreenX2, iScreenY2);

//   Cint32 iHeight = std::max(myint(iScreenY2 - iScreenY1),myint( 0));

//   if(iHeight <= 1)
//     return 0;                   // We're too small to render

//   if((y1 > iDasherMaxY) || (y2 < iDasherMinY)){
//     return 0;                   // We're entirely off screen, so don't render.
//   }

//   myint iDasherSize(y2 - y1);

//   // FIXME - get rid of pointless assignment below

//   int iTruncation(GetLongParameter(LP_TRUNCATION));     // Trucation farction times 100;
//   //  int iTruncationType(GetLongParameter(LP_TRUNCATIONTYPE));

//   if(iTruncation == 0) {        // Regular squares
//     DasherDrawRectangle(std::min(iDasherSize,iDasherMaxX), std::min(y2,iDasherMaxY), 0, std::max(y1,iDasherMinY), Color, -1, Nodes1, GetBoolParameter(BP_OUTLINE_MODE), true, 1);
//   }
//   else {
//     // TODO: Reimplement

// //     int iDasherY((myint)GetLongParameter(LP_MAX_Y));

// //     int iSpacing(iDasherY / 128);       // FIXME - assuming that this is an integer below

// //     int iXStart = 0;

// //     switch (iTruncationType) {
// //     case 1:
// //       iXStart = iSize - iSize * iTruncation / 200;
// //       break;
// //     case 2:
// //       iXStart = iSize - iSize * iTruncation / 100;
// //       break;
// //     }

// //     int iTipMin((y2 - y1) * iTruncation / (200) + y1);
// //     int iTipMax(y2 - (y2 - y1) * iTruncation / (200));

// //     int iLowerMin(((y1 + 1) / iSpacing) * iSpacing);
// //     int iLowerMax(((iTipMin - 1) / iSpacing) * iSpacing);

// //     int iUpperMin(((iTipMax + 1) / iSpacing) * iSpacing);
// //     int iUpperMax(((y2 - 1) / iSpacing) * iSpacing);

// //     if(iLowerMin < 0)
// //       iLowerMin = 0;

// //     if(iLowerMax < 0)
// //       iLowerMax = 0;

// //     if(iUpperMin < 0)
// //       iUpperMin = 0;

// //     if(iUpperMax < 0)
// //       iUpperMax = 0;

// //     if(iLowerMin > iDasherY)
// //       iLowerMin = iDasherY;

// //     if(iLowerMax > iDasherY)
// //       iLowerMax = iDasherY;

// //     if(iUpperMin > iDasherY)
// //       iUpperMin = iDasherY;

// //     if(iUpperMax > iDasherY)
// //       iUpperMax = iDasherY;

// //     while(iLowerMin < y1)
// //       iLowerMin += iSpacing;

// //     while(iLowerMax > iTipMin)
// //       iLowerMax -= iSpacing;

// //     while(iUpperMin < iTipMax)
// //       iUpperMin += iSpacing;

// //     while(iUpperMax > y2)
// //       iUpperMax -= iSpacing;

// //     int iLowerCount((iLowerMax - iLowerMin) / iSpacing + 1);
// //     int iUpperCount((iUpperMax - iUpperMin) / iSpacing + 1);

// //     if(iLowerCount < 0)
// //       iLowerCount = 0;

// //     if(iUpperCount < 0)
// //       iUpperCount = 0;

// //     int iTotalCount(iLowerCount + iUpperCount + 6);

// //     myint *x = new myint[iTotalCount];
// //     myint *y = new myint[iTotalCount];

// //     // Weird duplication here is to make truncated squares possible too

// //     x[0] = 0;
// //     y[0] = y1;
// //     x[1] = iXStart;
// //     y[1] = y1;

// //     x[iLowerCount + 2] = iDasherSize;
// //     y[iLowerCount + 2] = iTipMin;
// //     x[iLowerCount + 3] = iDasherSize;
// //     y[iLowerCount + 3] = iTipMax;

// //     x[iTotalCount - 2] = iXStart;
// //     y[iTotalCount - 2] = y2;
// //     x[iTotalCount - 1] = 0;
// //     y[iTotalCount - 1] = y2;

// //     for(int i(0); i < iLowerCount; ++i) {
// //       x[i + 2] = (iLowerMin + i * iSpacing - y1) * (iDasherSize - iXStart) / (iTipMin - y1) + iXStart;
// //       y[i + 2] = iLowerMin + i * iSpacing;
// //     }

// //     for(int j(0); j < iUpperCount; ++j) {
// //       x[j + iLowerCount + 4] = (y2 - (iUpperMin + j * iSpacing)) * (iDasherSize - iXStart) / (y2 - iTipMax) + iXStart;
// //       y[j + iLowerCount + 4] = iUpperMin + j * iSpacing;
// //     }

// //     DasherPolygon(x, y, iTotalCount, Color);

// //     delete x;
// //     delete y;
//   }

//   myint iDasherAnchorX(iDasherSize);

//   if( sDisplayText.size() > 0 )
//     DasherDrawText(iDasherAnchorX, y1, iDasherAnchorX, y2, sDisplayText, mostleft, bShove);

//   return 1;
// }

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

  // TODO: Is this used any more?
  if( b1D ) { // Special case for 1D mode...
    DASHER_ASSERT(false);
    //    iDasherX = iInputX * iDasherWidth / iScreenWidth;
    // iDasherY = iInputY * iDasherHeight / iScreenHeight;
    //return;
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


inline myint CDasherViewSquare::CustomIDiv(myint iNumerator, myint iDenominator) { 
  // Integer division rounding away from zero
  
#ifdef _WIN32
  myint quot = iNumerator / iDenominator;
  myint rem = (int64)iNumerator % (int64)iDenominator;

  if(rem < 0)
    return quot - 1;
  else if (rem > 0)
    return quot + 1;
  else
    return quot;
#else
  lldiv_t res = ::lldiv(iNumerator, iDenominator);
  
  if(res.rem < 0)
    return res.quot - 1;
  else if (res.rem > 0)
    return res.quot + 1;
  else
    return res.quot;
#endif

  // return (iNumerator + iDenominator - 1) / iDenominator;
}

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


  // Note that integer division is rounded *away* from zero here to
  // enesure that this really is the inverse of the map the other way
  // around.

  switch( eOrientation ) {
  case Dasher::Opts::LeftToRight:
    iScreenX = screenint(iScreenWidth / 2 - 
			 CustomIDiv((( iDasherX - iDasherWidth / 2 ) * iScaleFactorX), m_iScalingFactor));
    iScreenY = screenint(iScreenHeight / 2 +
			 CustomIDiv(( iDasherY - iDasherHeight / 2 ) * iScaleFactorY, m_iScalingFactor));
    break;
  case Dasher::Opts::RightToLeft:
    iScreenX = screenint(iScreenWidth / 2 + 
			 CustomIDiv(( iDasherX - iDasherWidth / 2 ) * iScaleFactorX, m_iScalingFactor));
    iScreenY = screenint(iScreenHeight / 2 + 
			 CustomIDiv(( iDasherY - iDasherHeight / 2 ) * iScaleFactorY, m_iScalingFactor));
    break;
  case Dasher::Opts::TopToBottom:
    iScreenX = screenint(iScreenWidth / 2 + 
			 CustomIDiv(( iDasherY - iDasherHeight / 2 ) * iScaleFactorX, m_iScalingFactor));
    iScreenY = screenint(iScreenHeight / 2 - 
			 CustomIDiv(( iDasherX - iDasherWidth / 2 ) * iScaleFactorY, m_iScalingFactor));
    break;
  case Dasher::Opts::BottomToTop:
    iScreenX = screenint(iScreenWidth / 2 + 
			 CustomIDiv(( iDasherY - iDasherHeight / 2 ) * iScaleFactorX, m_iScalingFactor));
    iScreenY = screenint(iScreenHeight / 2 + 
			 CustomIDiv(( iDasherX - iDasherWidth / 2 ) * iScaleFactorY, m_iScalingFactor));
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

