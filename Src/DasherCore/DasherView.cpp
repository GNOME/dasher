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

CDasherView::CDasherView(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherScreen* DasherScreen, CDasherModel& DasherModel)
  : CDasherComponent( pEventHandler, pSettingsStore ), m_pScreen(DasherScreen), m_DasherModel(DasherModel),
    m_pInput(0)
{

}



void CDasherView::HandleEvent( Dasher::CEvent *pEvent ) {

		if(	pEvent->m_iEventType	== 1 ) {
			Dasher::CParameterNotificationEvent	*pEvt( static_cast<	Dasher::CParameterNotificationEvent	* >( pEvent	));

			switch(	pEvt->m_iParameter ) {

				default:
					break;
			}
		}

	};


/////////////////////////////////////////////////////////////////////////////

void CDasherView::ChangeScreen(CDasherScreen* NewScreen)
{
	m_pScreen=NewScreen;
}

/////////////////////////////////////////////////////////////////////////////

//void CDasherView::ChangeOrientation(Dasher::Opts::ScreenOrientations Orientation)
//{
//	ScreenOrientation = Orientation;
//}

void CDasherView::DrawMousePosBox()
{
	int iHeight = Screen().GetHeight();
	int iWidth = Screen().GetWidth();

//	DASHER_TRACEOUTPUT("which %d\n",iWhich);
	int iMousePosDist = GetLongParameter(LP_MOUSEPOSDIST);
    int iDrawMousePosBox = GetLongParameter(LP_MOUSE_POS_BOX);

    std::cout << "iDrawMousePosBox: " << iDrawMousePosBox << std::endl;

	switch (iDrawMousePosBox) 
	{
		case 1:
			Screen().DrawRectangle(0,iHeight/2-iMousePosDist+50,iWidth,iHeight/2-iMousePosDist-50,119, Opts::Nodes1);
			break;	
		case 2:
			Screen().DrawRectangle(0,iHeight/2+iMousePosDist+50,iWidth,iHeight/2+iMousePosDist-50,120, Opts::Nodes1);
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
  
  if (GetBoolParameter(BP_DRAW_MOUSE)) {
    DrawMouse(iMouseX, iMouseY);
    bDidSomething = true;
  }
  if (GetBoolParameter(BP_DRAW_MOUSE_LINE)) {
    DrawMouseLine(iMouseX, iMouseY);
    bDidSomething = true;
  }
  if (GetBoolParameter(BP_KEYBOARD_MODE)) {
    DrawKeyboard();
    bDidSomething = true;

  }
  if (GetLongParameter(LP_MOUSE_POS_BOX) !=-1) {
    DrawMousePosBox();
    bDidSomething = true;
  }

  if (GetBoolParameter(BP_GAME_MODE) != 0) {
    DrawGameModePointer();
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
  
  if (GetBoolParameter(BP_KEYBOARD_MODE)) 
    DrawKeyboard();
  if (GetLongParameter(LP_MOUSE_POS_BOX) !=-1)
    DrawMousePosBox();
}
