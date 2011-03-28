
#include "../Common/Common.h"

#include "OneButtonFilter.h"
#include "Event.h"
#include "DasherScreen.h"

using namespace Dasher;

static SModuleSettings sSettings[] = {
  {LP_STATIC1B_TIME, T_LONG, 100, 5000, 1, 100, _("Scan time (each direction), in ms")},
  {LP_STATIC1B_ZOOM, T_LONG, 1, 16, 1, 1, _("Factor by which to zoom in")},
  {LP_DYNAMIC_BUTTON_LAG, T_LONG, 0, 1000, 1, 25, _("Lag before user actually pushes button (ms)")},
};

COneButtonFilter::COneButtonFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, 9, "Static One Button Mode") {

  bStarted = 0;
  iLocation = 0;

}

COneButtonFilter::~COneButtonFilter() {
}

bool COneButtonFilter::DecorateView(CDasherView *pView, CDasherInput *pInput) {

  CDasherScreen *pScreen(pView->Screen());

  if (iLocation == 0) {
    if (!bStarted) {
      if (m_bNoDecorations) return false;
      m_bNoDecorations = true;
      return true;
    }
    //reverse!
    CDasherScreen::point p[4];
    pView->Dasher2Screen(2048, 0, p[0].x, p[0].y);
    pView->Dasher2Screen(4096, 0, p[1].x, p[1].y);
    pView->Dasher2Screen(4096, 4096, p[2].x, p[2].y);
    pView->Dasher2Screen(2048, 4096, p[3].x, p[3].y);
    pScreen->Polyline(p, 4, 1, 1);
  } else {
    CDasherScreen::point p[2];

    pView->Dasher2Screen(-100, iLocation, p[0].x, p[0].y);
    pView->Dasher2Screen(-1000, iLocation, p[1].x, p[1].y);
    pScreen->Polyline(p, 2, 1, 1);
  }
  m_bNoDecorations = false;
  return true;
}

bool COneButtonFilter::Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol) {

  if(bStarted) {
    iLocation = (Time - iStartTime) * 4096 / GetLongParameter(LP_STATIC1B_TIME);
    if (iLocation > 8192) {
      //travelled down and back.
      if (iLocation > 12288) bStarted = false; //wait for a button push then scan again
      //else, leave bStarted = true -> "reverse"
      iLocation = 0;
    } else if (iLocation > 4096) {
      //reached bottom, scan back up
      iLocation = 8192-iLocation;
    }
  }

  return m_pDasherModel->NextScheduledStep(Time);
}

void COneButtonFilter::KeyDown(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog) {
  if (bStarted) {
    if (iLocation == 0) {
      //back up by one zoom step.
      const myint x(GetLongParameter(LP_STATIC1B_ZOOM)*2048);
      pModel->ScheduleZoom(iTime, 2048-x, 2048+x);
    } else {
      iLocation -= (GetLongParameter(LP_DYNAMIC_BUTTON_LAG)*4096) / GetLongParameter(LP_STATIC1B_TIME);
      if (iLocation>4096) iLocation =8192-iLocation;
      const myint x(2048/GetLongParameter(LP_STATIC1B_ZOOM));
      pModel->ScheduleZoom(iTime, 2048-x, 2048+x);
    }
    bStarted = false;
  } else {
    bStarted = true;
    iStartTime = iTime;
  }
}

bool COneButtonFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
}
