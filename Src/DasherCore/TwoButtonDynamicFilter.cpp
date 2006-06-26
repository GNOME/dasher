#include "../Common/Common.h"

#include "TwoButtonDynamicFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

CTwoButtonDynamicFilter::CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, 14, 1, "Two Button Dynamic Mode") {
  m_bBackoff = false;
  m_bDecorationChanged = true;
}

bool CTwoButtonDynamicFilter::DecorateView(CDasherView *pView) {

  CDasherScreen *pScreen(pView->Screen());

  CDasherScreen::point p[2];
  
  myint iDasherX;
  myint iDasherY;
  
  iDasherX = -100;
  iDasherY = 2048 - GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = 2048 - GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  pScreen->Polyline(p, 2, 1, 2);

  iDasherX = -100;
  iDasherY = 2048 + GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = 2048 + GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  pScreen->Polyline(p, 2, 1, 2);

  bool bRV(m_bDecorationChanged);
  m_bDecorationChanged = false;
  return bRV;
}

bool CTwoButtonDynamicFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  if(m_bBackoff)
    return m_pDasherModel->Tap_on_display(4096,2048, Time, 0, 0);
  else
    return m_pDasherModel->Tap_on_display(100,2048, Time, 0, 0);
}

void CTwoButtonDynamicFilter::KeyDown(int iTime, int iId, CDasherModel *pModel) {

  switch(iId) {
  case 0: // Start on space
    // FIXME - wrap this in a 'start/stop' method (and use for buttons as well as keys)
    if(GetBoolParameter(BP_DASHER_PAUSED))
	    m_pInterface->Unpause(iTime);
    else
	    m_pInterface->PauseAt(0, 0);
    break; 
  case 1:
    m_bBackoff = true;
    SetBoolParameter(BP_DELAY_VIEW, false);
    break;
  case 2:
    if(GetBoolParameter(BP_DASHER_PAUSED))
      m_pInterface->Unpause(iTime);
    else
      pModel->Offset(GetLongParameter(LP_TWO_BUTTON_OFFSET));
    break;
  case 3:
  case 4:
    if(GetBoolParameter(BP_DASHER_PAUSED))
      m_pInterface->Unpause(iTime);
    else
      pModel->Offset(-GetLongParameter(LP_TWO_BUTTON_OFFSET));
    break;
  case 100: // Start on mouse
    if(GetBoolParameter(BP_DASHER_PAUSED))
     m_pInterface->Unpause(iTime);
    else
     m_pInterface->PauseAt(0, 0);
    break;
  }

}

void CTwoButtonDynamicFilter::KeyUp(int iTime, int iId, CDasherModel *pModel) {
  switch(iId) {
  case 1:
    m_bBackoff = false;
    SetBoolParameter(BP_DELAY_VIEW, true);
    break;
  }
}

void CTwoButtonDynamicFilter::Activate() {
  SetBoolParameter(BP_DELAY_VIEW, true);
}

void CTwoButtonDynamicFilter::Deactivate() {
  SetBoolParameter(BP_DELAY_VIEW, false);
}
