#include "../Common/Common.h"
#include "StylusFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"
#include "ClickFilter.h"

using namespace Dasher;

static SModuleSettings sSettings[] = {
  {LP_TAP_TIME, T_LONG, 1, 1000, 1, 25, _("Max time for a 'tap' (anything longer is held)")},
  {LP_ZOOMSTEPS, T_LONG, 1, 63, 1, 1, _("Frames over which to perform zoom")},
};

CStylusFilter::CStylusFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName)
  : CDefaultFilter(pCreator, pInterface, pFramerate, iID, szName), m_pModel(NULL) {
}

void CStylusFilter::KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  if(iId == 100) {
    //pModel->ClearScheduledSteps(); //no need - each one step scheduled by superclass, will do this
    run(iTime);
    m_iKeyDownTime = iTime;
  } else
    CDefaultFilter::KeyDown(iTime, iId, pView, pInput, pModel);
}

void CStylusFilter::pause() {
  CDefaultFilter::pause();
  if (m_pModel) m_pModel->ClearScheduledSteps();
}

void CStylusFilter::KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  if(iId == 100) {
    pause(); //stops superclass from scheduling any more one-step movements
    if (iTime - m_iKeyDownTime < GetLongParameter(LP_TAP_TIME)) {
      pInput->GetDasherCoords(m_iLastX, m_iLastY, pView);
      ApplyClickTransform(m_iLastX, m_iLastY, pView);
      (m_pModel=pModel)->ScheduleZoom(m_iLastY-m_iLastX, m_iLastY+m_iLastX, GetLongParameter(LP_ZOOMSTEPS));
    } else {
      m_pInterface->Done();
    }
  } else
    CDefaultFilter::KeyUp(iTime, iId, pView, pInput, pModel);
}

void CStylusFilter::ApplyClickTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView) {
  AdjustZoomX(iDasherX, pView, GetLongParameter(LP_S), GetLongParameter(LP_MAXZOOM));
}

CStartHandler *CStylusFilter::MakeStartHandler() {
  return NULL;
}

bool CStylusFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(sSettings[0]);
  return true;
}
