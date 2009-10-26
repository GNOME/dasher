#include "../Common/Common.h"
#include "ClickFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

using namespace Dasher;

static SModuleSettings sSettings[] = {
  {LP_MAXZOOM, T_LONG, 11, 400, 10, 1, _("Maximum Zoom")},
};

bool CClickFilter::DecorateView(CDasherView *pView) {
  return false;
}

bool CClickFilter::Timer(int Time, CDasherView *pDasherView, CDasherModel *pModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol) {
  return pModel->NextScheduledStep(Time, pAdded, pNumDeleted);
}

void CClickFilter::KeyDown(int iTime, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY) {
  switch(iId) {
  case 100: // Mouse clicks
    if(pDasherView) {
      myint iDasherX;
      myint iDasherY;

      pDasherView->GetCoordinates(iDasherX, iDasherY);

      pModel->ScheduleZoom(iDasherX,iDasherY, GetLongParameter(LP_MAXZOOM));
    }
    break;
  default:
    break;
  }
}

void CClickFilter::HandleEvent(Dasher::CEvent * pEvent) {
}

bool CClickFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
};
