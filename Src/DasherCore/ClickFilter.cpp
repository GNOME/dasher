#include "../Common/Common.h"
#include "ClickFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

#include <iostream>

CClickFilter::CClickFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface) {
  pDasherView = 0;
}

CClickFilter::~CClickFilter() {
}

void CClickFilter::DecorateView(CDasherView *pView) {
}

void CClickFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  // FIXME - REALLY, REALLY horrible - bleh
  m_pDasherModel->Tap_on_display(0, 0, Time, 0, 0);
  pDasherView = m_pDasherView;
}

void CClickFilter::KeyDown(int iTime, int iId, CDasherModel *pModel) {

  switch(iId) {
  case 100: // Mouse clicks
    if(pDasherView) {
      myint iDasherX;
      myint iDasherY;
      
      pDasherView->TapOnDisplay(0, 0, iTime, iDasherX, iDasherY);
      pModel->ScheduleZoom(iDasherX,iDasherY);
    }
    break;
  }
}

void CClickFilter::HandleEvent(Dasher::CEvent * pEvent) {
}
