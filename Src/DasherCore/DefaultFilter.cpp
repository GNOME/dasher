#include "../Common/Common.h"
#include "DefaultFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

#include <iostream>

CDefaultFilter::CDefaultFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, 3, 1) {
  m_pStartHandler = 0;
  m_pAutoSpeedControl = new CAutoSpeedControl(m_pEventHandler, m_pSettingsStore, m_pDasherModel->Framerate());

  CreateStartHandler();
}

CDefaultFilter::~CDefaultFilter() {
  delete m_pAutoSpeedControl;
}

void CDefaultFilter::DecorateView(CDasherView *pView) {
  if(m_pStartHandler)
    m_pStartHandler->DecorateView(pView);
}

void CDefaultFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  myint iDasherX;
  myint iDasherY;

  m_pDasherView->TapOnDisplay(0, 0, Time, iDasherX, iDasherY);
  m_pDasherModel->Tap_on_display(iDasherX,iDasherY, Time, 0, 0);

  m_pAutoSpeedControl->SpeedControl(iDasherX, iDasherY, m_pDasherModel->Framerate(), m_pDasherView);

  if(m_pStartHandler)
    m_pStartHandler->Timer(Time, m_pDasherView, m_pDasherModel);
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
    case BP_CIRCLE_START:
      CreateStartHandler();
      break;
    }
  }
}

void CDefaultFilter::CreateStartHandler() {
  if(m_pStartHandler) {
    delete m_pStartHandler;
    m_pStartHandler = 0;
  }

  if(GetBoolParameter(BP_CIRCLE_START))
    m_pStartHandler = new CCircleStartHandler(m_pEventHandler, m_pSettingsStore, m_pInterface);

}
