// DasherButtons.cpp, build a set of boxes for Button Dasher.
// Copyright 2005, Chris Ball and David MacKay.  GPL.

// Idea - should back off button always just undo the previous 'forwards' button?

#include "../Common/Common.h"

#include "AlternatingDirectMode.h"
#include "DasherScreen.h"
#include "DasherInterfaceBase.h"
#include <valarray>
#include <iostream>

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

using namespace Dasher;

static SModuleSettings sSettings[] = {
  /* TRANSLATORS: The number of time steps over which to perform the zooming motion in button mode. */
  {LP_ZOOMSTEPS, T_LONG, 1, 63, 1, 1, _("Frames over which to perform zoom")},
  /* TRANSLATORS: Intercept keyboard events for 'special' keys even when the Dasher window doesn't have keyboard focus.*/
  {BP_GLOBAL_KEYBOARD, T_BOOL, -1, -1, -1, -1, _("Global keyboard grab")}
};

CAlternatingDirectMode::CAlternatingDirectMode(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface)
  : CDasherButtons(pCreator, pInterface, false/*menu*/, 12, _("Alternating Direct Mode")) {}

void CAlternatingDirectMode::SetupBoxes()
{
  m_pBoxes = new SBoxInfo[m_iNumBoxes = 5];

  // Fast boxes

  m_pBoxes[0].iTop = 0;
  m_pBoxes[0].iBottom = 1000;
  m_pBoxes[1].iTop = 3096;
  m_pBoxes[1].iBottom = 4096;

  // Slow boxes

  m_pBoxes[2].iTop = 0;
  m_pBoxes[2].iBottom = 3096;
  m_pBoxes[3].iTop = 1000;
  m_pBoxes[3].iBottom = 4096;

  m_pBoxes[0].iDisplayTop = m_pBoxes[0].iTop;
  m_pBoxes[0].iDisplayBottom = m_pBoxes[0].iBottom;
  m_pBoxes[1].iDisplayTop = m_pBoxes[1].iTop;
  m_pBoxes[1].iDisplayBottom = m_pBoxes[1].iBottom;
  m_pBoxes[2].iDisplayTop = m_pBoxes[2].iTop;
  m_pBoxes[2].iDisplayBottom = m_pBoxes[2].iBottom;
  m_pBoxes[3].iDisplayTop = m_pBoxes[3].iTop;
  m_pBoxes[3].iDisplayBottom = m_pBoxes[3].iBottom;

  m_pBoxes[m_iNumBoxes-1].iDisplayTop = 0;
  m_pBoxes[m_iNumBoxes-1].iDisplayBottom = CDasherModel::MAX_Y;

  m_pBoxes[m_iNumBoxes-1].iTop = int(- CDasherModel::MAX_Y / 2);
  m_pBoxes[m_iNumBoxes-1].iBottom = int(CDasherModel::MAX_Y * 1.5);

  m_iLastBox = -1;
}

bool CAlternatingDirectMode::DecorateView(CDasherView *pView, CDasherInput *pInput) {

  if(m_iLastBox == 1) {
    NewDrawGoTo(pView, m_pBoxes[2].iDisplayTop, m_pBoxes[2].iDisplayBottom, false);
    NewDrawGoTo(pView, m_pBoxes[1].iDisplayTop, m_pBoxes[3].iDisplayBottom, false);
    NewDrawGoTo(pView, m_pBoxes[4].iDisplayTop, m_pBoxes[4].iDisplayBottom, false);
  }
  else {
    NewDrawGoTo(pView, m_pBoxes[0].iDisplayTop, m_pBoxes[0].iDisplayBottom, false);
    NewDrawGoTo(pView, m_pBoxes[3].iDisplayTop, m_pBoxes[1].iDisplayBottom, false);
    NewDrawGoTo(pView, m_pBoxes[4].iDisplayTop, m_pBoxes[4].iDisplayBottom, false);
  }

  bool bRV(m_bDecorationChanged);
  m_bDecorationChanged = false;
  return bRV;
}


void CAlternatingDirectMode::DirectKeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherModel *pModel) {
  int iTargetBox;
  switch(iId) {
    case 2:
      iTargetBox = (m_iLastBox == 1) ? 2 : 0;
      m_iLastBox = 1;
      break;
    case 3:
    case 4:
      iTargetBox = (m_iLastBox==2) ? 3 : 1;
      m_iLastBox = 2;
      break;
    case 1:
      iTargetBox = 4;
      break;
    default:
      //unknown button...do nothing (?)
      return;
  }
  //iTargetBox now indicates the box into which to zoom
  ScheduleZoom(pModel, m_pBoxes[iTargetBox].iTop,  m_pBoxes[iTargetBox].iBottom);
}

bool CAlternatingDirectMode::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
}
