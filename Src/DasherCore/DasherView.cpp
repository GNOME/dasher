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

CDasherView::CDasherView(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherScreen *DasherScreen, CDasherModel *DasherModel)
:CDasherComponent(pEventHandler, pSettingsStore), m_pScreen(DasherScreen), m_pDasherModel(DasherModel), m_pInput(0) {

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

//void CDasherView::ChangeOrientation(Dasher::Opts::ScreenOrientations Orientation)
//{
//      ScreenOrientation = Orientation;
//}

void CDasherView::DrawMousePosBox() {
  int iHeight = Screen()->GetHeight();
  int iWidth = Screen()->GetWidth();

  //    DASHER_TRACEOUTPUT("which %d\n",iWhich);
  int iMousePosDist = GetLongParameter(LP_MOUSEPOSDIST);
  int iDrawMousePosBox = GetLongParameter(LP_MOUSE_POS_BOX);

  //  std::cout << "iDrawMousePosBox: " << iDrawMousePosBox << std::endl;

  switch (iDrawMousePosBox) {
  case 1:
    Screen()->DrawRectangle(8, iHeight / 2 - iMousePosDist + 50, iWidth-16, iHeight / 2 - iMousePosDist - 50, 0, 119, Opts::Nodes1,true, false, 4);
    break;
  case 2:
    Screen()->DrawRectangle(8, iHeight / 2 + iMousePosDist + 50, iWidth-16, iHeight / 2 + iMousePosDist - 50, 0, 120, Opts::Nodes1,true, false, 4);
    break;
  default:
    //      DASHER_ASSERT(0);
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////

bool CDasherView::Render(int iMouseX, int iMouseY, bool bRedrawDisplay) {

  bool bDidSomething(false);    // Have we actually done any drawing - no
  // point updating the display if we
  // didn't

  if(bRedrawDisplay) {
    Screen()->SendMarker(0);     // Start of 'dasher field'
    RenderNodes();
    bDidSomething = true;
  }

  Screen()->SendMarker(1);       // Start of 'decoration'

  if(GetBoolParameter(BP_DRAW_MOUSE)) {
    DrawMouse(iMouseX, iMouseY);
    bDidSomething = true;
  }
  if(GetBoolParameter(BP_DRAW_MOUSE_LINE)) {
    DrawMouseLine(iMouseX, iMouseY);
    bDidSomething = true;
  }
 //  if(GetBoolParameter(BP_KEYBOARD_MODE)) {
//     DrawKeyboard();
//     bDidSomething = true;

//   }
  if(GetLongParameter(LP_MOUSE_POS_BOX) != -1) {
    DrawMousePosBox();
    bDidSomething = true;
  }

  if(GetBoolParameter(BP_GAME_MODE) != 0) {
    DrawGameModePointer();
    bDidSomething = true;
  }

  return bDidSomething;

}

/////////////////////////////////////////////////////////////////////////////

void CDasherView::Render() {

  // FIXME - when does this get called?

  Screen()->SendMarker(0);

  RenderNodes();

  Screen()->SendMarker(1);

  //  if(GetBoolParameter(BP_KEYBOARD_MODE))
  //    DrawKeyboard();
  if(GetLongParameter(LP_MOUSE_POS_BOX) != -1)
    DrawMousePosBox();
}

  int CDasherView::GetCoordinateCount() {
    if(m_pInput)
      return m_pInput->GetCoordinateCount();

    return 0;
  }

int CDasherView::GetCoordinates(int iN, myint * pCoordinates) {

  if(m_pInput)
    return m_pInput->GetCoordinates(iN, pCoordinates);

  return 0;
}

void CDasherView::SetInput(CDasherInput * _pInput) {

  DASHER_ASSERT_VALIDPTR_RW(_pInput);

  // Don't delete the old input class; whoever is calling this method
  // might want to keep several Input class instances around and
  // change which one is currently driving dasher without deleting any

  m_pInput = _pInput;

  // Tell the new object about maximum values

  myint iMaxCoordinates[2];

  iMaxCoordinates[0] = m_pDasherModel->DasherY();
  iMaxCoordinates[1] = m_pDasherModel->DasherY();

  m_pInput->SetMaxCoordinates(2, iMaxCoordinates);

}

void CDasherView::Display() {
  m_pScreen->Display();
}

