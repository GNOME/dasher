// DasherView.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include <iostream>

#include "DasherView.h"
using namespace Dasher;

/////////////////////////////////////////////////////////////////////////////

CDasherView::CDasherView(CDasherScreen* DasherScreen, CDasherModel& DasherModel, Opts::ScreenOrientations Orientation, bool ColourMode)
  : ScreenOrientation(Orientation), ColourMode(ColourMode), m_pScreen(DasherScreen), m_DasherModel(DasherModel),
  m_bDrawMouse(false),m_bDrawMouseLine(false),m_bDrawKeyboard(false),m_iDrawMousePosBox(0),
  m_iMousePosDist(50)
{
	// myint ySum, ySumCounter=0, yFilterTimescale=2, yAutoOffset=0, ySigBiasPixels=0, ySigBiasPercentage=0;   
}


/////////////////////////////////////////////////////////////////////////////

void CDasherView::ChangeScreen(CDasherScreen* NewScreen)
{
	m_pScreen=NewScreen;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherView::ChangeOrientation(Dasher::Opts::ScreenOrientations Orientation)
{
	ScreenOrientation = Orientation;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherView::SetDrawMouse(bool bMouse)
{
	m_bDrawMouse = bMouse;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherView::SetDrawMouseLine(bool bMouseLine)
{
	m_bDrawMouseLine = bMouseLine;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherView::DrawMousePosBox()
{
	int iHeight = Screen().GetHeight();
	int iWidth = Screen().GetWidth();

//	DASHER_TRACEOUTPUT("which %d\n",iWhich);
	switch (m_iDrawMousePosBox) 
	{
	
		case 1:
			Screen().DrawRectangle(0,iHeight/2-m_iMousePosDist+50,iWidth,iHeight/2-m_iMousePosDist-50,119, Opts::Nodes1);
			break;	
		case 2:
			Screen().DrawRectangle(0,iHeight/2+m_iMousePosDist+50,iWidth,iHeight/2+m_iMousePosDist-50,120, Opts::Nodes1);
			break;
		default:
			DASHER_ASSERT(0);
	}

}

/////////////////////////////////////////////////////////////////////////////

bool CDasherView::Render(int iMouseX, int iMouseY, bool bRedrawDisplay)
{

  bool bDidSomething(false); // Have we actually done any drawing - no
			     // point updating the display if we
			     // didn't

  if( bRedrawDisplay ) {
    Screen().SendMarker(0); // Start of 'dasher field'
    RenderNodes();
    bDidSomething = true;
  }

  Screen().SendMarker(1); // Start of 'decoration'
  
  if (m_bDrawMouse) {
    DrawMouse(iMouseX, iMouseY);
    bDidSomething = true;
  }
  if (m_bDrawMouseLine) {
    DrawMouseLine(iMouseX, iMouseY);
    bDidSomething = true;
  }
  if (m_bDrawKeyboard) {
    DrawKeyboard();
    bDidSomething = true;

  }
  if (m_iDrawMousePosBox !=0) {
    DrawMousePosBox();
    bDidSomething = true;
  }

  return bDidSomething;

}


/////////////////////////////////////////////////////////////////////////////

void CDasherView::Render()
{
  Screen().SendMarker(0);

  RenderNodes();
  
  Screen().SendMarker(1);
  
  if (m_bDrawKeyboard) 
    DrawKeyboard();
  if (m_iDrawMousePosBox !=0)
    DrawMousePosBox();
}

void CDasherView::SetDrawMousePosBox(int iWhich)
{
	m_iDrawMousePosBox = iWhich;

}


/////////////////////////////////////////////////////////////////////////////

int CDasherView::GetOneButton() const 
{
     return onebutton;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherView::SetOneButton(int Value) 
{
       if (onebutton < -5000) 
               onebutton=-5000;
       if (onebutton > 7000)
               onebutton=7000;
 
       onebutton += Value;
}
