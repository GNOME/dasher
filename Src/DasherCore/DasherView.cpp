// DasherView.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

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

void CDasherView::Render(int iMouseX, int iMouseY)
{
	RenderNodes();

	if (m_bDrawMouse) 
		DrawMouse(iMouseX, iMouseY);
	if (m_bDrawMouseLine) 
		DrawMouseLine(iMouseX, iMouseY);
	if (m_bDrawKeyboard) 
		DrawKeyboard();
	if (m_iDrawMousePosBox !=0)
		DrawMousePosBox();
}


/////////////////////////////////////////////////////////////////////////////

void CDasherView::Render()
{
	RenderNodes();

	if (m_bDrawKeyboard) 
		DrawKeyboard();
	if (m_iDrawMousePosBox !=0)
		DrawMousePosBox();
}

void CDasherView::SetDrawMousePosBox(int iWhich)
{
	m_iDrawMousePosBox = iWhich;

}