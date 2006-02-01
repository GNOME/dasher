#include "../Common/Common.h"
#include "DefaultFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

#include <iostream>

CDefaultFilter::CDefaultFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface) {

  m_pAutoSpeedControl = new CAutoSpeedControl(m_pEventHandler, m_pSettingsStore, m_pDasherModel->Framerate());

  m_iStatus = -1;
  m_iChangeTime = 0;
  m_iCircleRadius = 50;
}

CDefaultFilter::~CDefaultFilter() {
  delete m_pAutoSpeedControl;
}

void CDefaultFilter::DecorateView(CDasherView *pView) {
  screenint iCX;
  screenint iCY;

  pView->Dasher2Screen(2048, 2048, iCX, iCY);
  if(m_iStatus == -1)
    pView->Screen()->DrawCircle(iCX, iCY, m_iCircleRadius, 0);
  else
    pView->Screen()->DrawCircle(iCX, iCY, m_iCircleRadius, m_iStatus);
}

void CDefaultFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  myint iDasherX;
  myint iDasherY;

  m_pDasherView->TapOnDisplay(0, 0, Time, iDasherX, iDasherY);
  m_pDasherModel->Tap_on_display(iDasherX,iDasherY, Time, 0, 0);

  m_pAutoSpeedControl->SpeedControl(iDasherX, iDasherY, m_pDasherModel->Framerate(), m_pDasherView);

  // The following is 'start on circle' code, and will be moved elsewhere when it's finished

  screenint iCX;
  screenint iCY;
  m_pDasherView->Dasher2Screen(2048, 2048, iCX, iCY);
  
  screenint iCursorX;
  screenint iCursorY;
  m_pDasherView->Dasher2Screen(iDasherX, iDasherY, iCursorX, iCursorY);

  double dR;

  dR = sqrt(pow(iCX - iCursorX, 2.0) + pow(iCY - iCursorY, 2.0));

  int iNewStatus;

  // Status flags:
  // -1 undefined
  // 0 = out of circle, stopped
  // 1 = out of circle, started
  // 2 = in circle, stopped
  // 3 = in circle, started
  // 4 = in circle, stopping
  // 5 = in circle, starting

  // TODO - need to check that these respond correctly to (eg) external pauses

  if(dR < m_iCircleRadius) {
    switch(m_iStatus) {
    case -1:
      if(GetBoolParameter(BP_DASHER_PAUSED))
	iNewStatus = 2;
      else
	iNewStatus = 3;
      break;
    case 0:
      iNewStatus = 5;
      break;
    case 1:
      iNewStatus = 4;
      break;
    case 2:
    case 3:
    case 4:
    case 5:
      iNewStatus = m_iStatus;
      break;
    }
  }
  else {
    switch(m_iStatus) {
    case -1:
      if(GetBoolParameter(BP_DASHER_PAUSED))
	iNewStatus = 0;
      else
	iNewStatus = 1;
      break;
    case 0:
    case 1:
      iNewStatus = m_iStatus;
      break;
    case 2:
      iNewStatus = 0;
      break;
    case 3:
      iNewStatus = 1;
      break;
    case 4:
      iNewStatus = 1;
      break;
    case 5:
      iNewStatus = 0;
      break;
    }
  }

  if(iNewStatus != m_iStatus) {
    std::cout << "Status changed to: " << iNewStatus << std::endl;
    m_iChangeTime = Time;
  }

  if(Time - m_iChangeTime > 1000) {
    if(iNewStatus == 4) {
      iNewStatus = 2;
      m_pInterface->PauseAt(0, 0);
    } 
    else if(iNewStatus == 5) {
      iNewStatus = 3;
      m_pInterface->Unpause(Time);
    }
  }

  m_iStatus = iNewStatus;
}

void CDefaultFilter::KeyDown(int iTime, int iId, CDasherModel *pModel) {

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

void CDefaultFilter::HandleEvent(Dasher::CEvent * pEvent) {

  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    
    switch (pEvt->m_iParameter) {
    case BP_DASHER_PAUSED:
      m_iStatus = -1;
      break;
    }
  }
}
