#include "../Common/Common.h"
#include "ClickFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

bool CClickFilter::DecorateView(CDasherView *pView) {
  return false;
}

bool CClickFilter::Timer(int Time, CDasherView *pDasherView, CDasherModel *pModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) {
  return pModel->UpdatePosition(0, 0, Time, pAdded, pNumDeleted);
}

void CClickFilter::KeyDown(int iTime, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY) {
  switch(iId) {
  case 100: // Mouse clicks
    if(pDasherView) {
      myint iDasherX;
      myint iDasherY;

      pDasherView->GetCoordinates(iDasherX, iDasherY);

      pModel->ScheduleZoom(iDasherX,iDasherY);
    }
    break;
  default:
    break;
  }
}

void CClickFilter::HandleEvent(Dasher::CEvent * pEvent) {
}
