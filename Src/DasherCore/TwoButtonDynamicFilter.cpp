#include "../Common/Common.h"

#include "TwoButtonDynamicFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

CTwoButtonDynamicFilter::CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, 14, 1, "Two Button Dynamic Mode") {
  m_iTarget = 0;

  m_iTargetX = new int[2];
  m_iTargetY = new int[2];

  m_iTargetX[0] = 100;
  m_iTargetY[0] = 100;

  m_iTargetX[1] = 100;
  m_iTargetY[1] = 3996; 

  if(GetBoolParameter(BP_BUTTONPULSING))
    m_iStyle = 1;
  else
    m_iStyle = 0;

  bStarted = false;
  m_bBackoff = false;
}

CTwoButtonDynamicFilter::~CTwoButtonDynamicFilter() {
  delete[] m_iTargetX;
  delete[] m_iTargetY;  
}

bool CTwoButtonDynamicFilter::DecorateView(CDasherView *pView) {

  CDasherScreen *pScreen(pView->Screen());

  CDasherScreen::point p[2];
  
  myint iDasherX;
  myint iDasherY;
  
  iDasherX = -100;
  iDasherY = 1024;
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = 1024;
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  pScreen->Polyline(p, 2, 1, 2);

  iDasherX = -100;
  iDasherY = 3072;
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = 3072;
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  pScreen->Polyline(p, 2, 1, 2);

  return true;
}

void CTwoButtonDynamicFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  if(m_bBackoff)
    m_pDasherModel->Tap_on_display(4096,2048, Time, 0, 0);
  else
    m_pDasherModel->Tap_on_display(100,2048, Time, 0, 0);

}

void CTwoButtonDynamicFilter::KeyDown(int iTime, int iId, CDasherModel *pModel) {

  switch(iId) {
  case 0: // Start on space
    // FIXME - wrap this in a 'start/stop' method (and use for buttons as well as keys)
    if(GetBoolParameter(BP_START_SPACE) && !GetBoolParameter(BP_CLICK_MODE)) {
      if(GetBoolParameter(BP_DASHER_PAUSED))
	m_pInterface->Unpause(iTime);
      else
	m_pInterface->PauseAt(0, 0);
    }
    break; 
  case 1:
    m_bBackoff = true;
    break;
  case 2:
    pModel->ScheduleZoom(1024,1024);
    break;
  case 3:
  case 4:
    pModel->ScheduleZoom(1024,3096);
    break;
  case 100: // Start on mouse
    if(GetBoolParameter(BP_START_MOUSE)) {
      if(GetBoolParameter(BP_DASHER_PAUSED))
	m_pInterface->Unpause(iTime);
      else
	m_pInterface->PauseAt(0, 0);
    }
    break;
  }

}

void CTwoButtonDynamicFilter::KeyUp(int iTime, int iId, CDasherModel *pModel) {
  switch(iId) {
  case 1:
    m_bBackoff = false;
  }
}



void CTwoButtonDynamicFilter::HandleEvent(Dasher::CEvent * pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    
    switch (pEvt->m_iParameter) {
    case BP_BUTTONSTEADY:
    case BP_BUTTONPULSING:
      // Delibarate fall through
      if(GetBoolParameter(BP_BUTTONPULSING))
	m_iStyle = 1;
      else
	m_iStyle = 0;
      break;
    }
  }
}
