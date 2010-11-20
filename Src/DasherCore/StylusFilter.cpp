#include "../Common/Common.h"
#include "StylusFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

using namespace Dasher;

CStylusFilter::CStylusFilter(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName)
  : CDefaultFilter(pEventHandler, pSettingsStore, pInterface, iID, szName) {
}

bool CStylusFilter::Timer(int iTime, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol)
{
  //First, try to continue any zoom scheduled by a previous click...
  if (pModel->NextScheduledStep(iTime, pAdded, pNumDeleted)) {
    //note that this skips the rest of CDefaultFilter::Timer;
    //however, given we're paused, this is only the Start Handler,
    //which we're not using anyway.
    return true;
  }
  return CDefaultFilter::Timer(iTime, pView, pInput, pModel, pAdded, pNumDeleted, pol);
}

void CStylusFilter::KeyDown(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog) {
  if(iId == 100) {
    pModel->ClearScheduledSteps();
    m_pInterface->Unpause(iTime);
    m_iKeyDownTime = iTime;
  }
}

void CStylusFilter::KeyUp(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  if(iId == 100) {
    if (iTime - m_iKeyDownTime < GetLongParameter(LP_TAP_TIME)) {
      pInput->GetDasherCoords(m_iLastX, m_iLastY, pView);
      //Do not apply transform. (Could add extra virtual method, ApplyClickTransform?)
      pModel->ScheduleZoom(iTime, m_iLastX, m_iLastY, GetLongParameter(LP_MAXZOOM));
    } else {
      m_pInterface->Stop();
    }
  }
}
