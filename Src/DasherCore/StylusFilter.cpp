#include "../Common/Common.h"
#include "StylusFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"
#include "ClickFilter.h"

using namespace Dasher;

CStylusFilter::CStylusFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, ModuleID_t iID, const char *szName)
  : CDefaultFilter(pCreator, pInterface, iID, szName) {
}

bool CStylusFilter::Timer(unsigned long iTime, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CExpansionPolicy **pol)
{
  //First, try to continue any zoom scheduled by a previous click...
  if (pModel->NextScheduledStep(iTime)) {
    //note that this skips the rest of CDefaultFilter::Timer;
    //however, given we're paused, this is only the Start Handler,
    //which we're not using anyway.
    return true;
  }
  return CDefaultFilter::Timer(iTime, pView, pInput, pModel, pol);
}

void CStylusFilter::KeyDown(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
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
      ApplyClickTransform(m_iLastX, m_iLastY, pView);
      pModel->ScheduleZoom(iTime, m_iLastY-m_iLastX, m_iLastY+m_iLastX);
    } else {
      m_pInterface->Stop();
    }
  }
}

void CStylusFilter::ApplyClickTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView) {
  CZoomAdjuster(this).AdjustZoomCoords(iDasherX, iDasherY, pView);
}

CStartHandler *CStylusFilter::MakeStartHandler() {
  return NULL;
}
