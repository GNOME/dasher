#include "../Common/Common.h"
#include "ClickFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

#include <iostream>

CClickFilter::CClickFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, 7, 1, _("Click Mode")) {
  pDasherView = 0;
}

CClickFilter::~CClickFilter() {
}

bool CClickFilter::DecorateView(CDasherView *pView) {
  return false;
}

bool CClickFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) {
  // FIXME - REALLY, REALLY horrible - bleh
  pDasherView = m_pDasherView;
  return m_pDasherModel->UpdatePosition(0, 0, Time, pAdded, pNumDeleted);
}

void CClickFilter::KeyDown(int iTime, int iId, CDasherModel *pModel, CUserLogBase *pUserLog) {

  switch(iId) {
  case 100: // Mouse clicks
    if(pDasherView) {
      myint iDasherX;
      myint iDasherY;
      
      pDasherView->GetCoordinates(iTime, iDasherX, iDasherY);
      pModel->ScheduleZoom(iDasherX,iDasherY);
    }
    break;
  }
}

void CClickFilter::HandleEvent(Dasher::CEvent * pEvent) {
}
