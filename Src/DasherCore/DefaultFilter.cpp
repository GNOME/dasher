#include "../Common/Common.h"
#include "DefaultFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

CDefaultFilter::CDefaultFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, CDasherModel *m_pDasherModel)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface) {

  m_pAutoSpeedControl = new CAutoSpeedControl(m_pEventHandler, m_pSettingsStore, m_pDasherModel->Framerate());

}

CDefaultFilter::~CDefaultFilter() {
  delete m_pAutoSpeedControl;
}

void CDefaultFilter::DecorateView(CDasherView *pView) {
}

void CDefaultFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  myint iDasherX;
  myint iDasherY;

  m_pDasherView->TapOnDisplay(0, 0, Time, iDasherX, iDasherY);
  m_pDasherModel->Tap_on_display(iDasherX,iDasherY, Time, 0, 0);

  m_pAutoSpeedControl->SpeedControl(iDasherX, iDasherY, m_pDasherModel->Framerate(), m_pDasherView);
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
}
