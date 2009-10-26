#include "../Common/Common.h"
#include "StylusFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

using namespace Dasher;

CStylusFilter::CStylusFilter(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName)
  : CDefaultFilter(pEventHandler, pSettingsStore, pInterface, iID, szName) {
}

bool CStylusFilter::Timer(int iTime, CDasherView *pView, CDasherModel *pModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol)
{
  if (GetBoolParameter(BP_DASHER_PAUSED))
  {
    //continue any zoom scheduled by a previous click...
    return pModel->NextScheduledStep(iTime, pAdded, pNumDeleted);
    //note that this skips the rest of CDefaultFilter::Timer;
    //however, given we're paused, this is only the Start Handler,
    //which we're not using anyway.
  }
  return CDefaultFilter::Timer(iTime, pView, pModel, pAdded, pNumDeleted, pol);
}

void CStylusFilter::KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog) {
  if(iId == 100) {
    m_pInterface->Unpause(iTime);
    m_iKeyDownTime = iTime;
  }
}

void CStylusFilter::KeyUp(int iTime, int iId, CDasherView *pView, CDasherModel *pModel) {
  if(iId == 100) {
    m_pInterface->PauseAt(0, 0);
    if (iTime - m_iKeyDownTime < GetLongParameter(LP_TAP_TIME)) {
      myint iDasherX, iDasherY;
      pView->GetCoordinates(iDasherX, iDasherY);
      pModel->ScheduleZoom(iDasherX, iDasherY, GetLongParameter(LP_MAXZOOM));
    }
  }
}
