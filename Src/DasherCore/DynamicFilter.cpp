#include "DynamicFilter.h"
#include "Event.h"

CDynamicFilter::CDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore)
  : CInputFilter(pEventHandler, pSettingsStore) {
  m_iTarget = 0;

  m_iTargetX = new int[2];
  m_iTargetY = new int[2];

  m_iTargetX[0] = 100;
  m_iTargetY[0] = 100;

  m_iTargetX[1] = 100;
  m_iTargetY[1] = 3996; 

  if(GetBoolParameter(BP_BUTTONPULSING))
    m_iStyle = 1;
  else
    m_iStyle = 0;
}

CDynamicFilter::~CDynamicFilter() {
  delete[] m_iTargetX;
  delete[] m_iTargetY;  
}

void CDynamicFilter::DecorateView(CDasherView *pView) {

  CDasherScreen *pScreen(pView->Screen());

  screenint x1;
  screenint y1;
  screenint x2;
  screenint y2;

  if(m_iTarget == 0) {
    pView->Dasher2Screen(-100, 1000, x1, y1);
    pView->Dasher2Screen(-200, 4096, x2, y2);
  }
  else {
    pView->Dasher2Screen(-100, 0, x1, y1);
    pView->Dasher2Screen(-200, 3096, x2, y2);
  }

  pScreen->DrawRectangle(x1, y1, x2, y2, 1, 1, Opts::ColorSchemes(Objects), true, false, 1);
    
  if(m_iTarget == 0) {
    pView->Dasher2Screen(-100, 0, x1, y1);
    pView->Dasher2Screen(-200, 3096, x2, y2);
  }
  else {
    pView->Dasher2Screen(-100, 1000, x1, y1);
    pView->Dasher2Screen(-200, 4096, x2, y2);
  }

  pScreen->DrawRectangle(x1, y1, x2, y2, 2, 2, Opts::ColorSchemes(Objects), true, false, 1);
}

void CDynamicFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  m_pDasherModel->Tap_on_display(m_iTargetX[m_iTarget], m_iTargetY[m_iTarget], Time, 0, 0);
}

void CDynamicFilter::KeyDown(int iTime, int iId, CDasherModel *pModel) {
  if(iId == 1)
    m_iTarget = 1 - m_iTarget;
}

void CDynamicFilter::HandleEvent(Dasher::CEvent * pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    
    switch (pEvt->m_iParameter) {
    case BP_BUTTONSTEADY:
    case BP_BUTTONPULSING:
      // Delibarate fall through
      if(GetBoolParameter(BP_BUTTONPULSING))
	m_iStyle = 1;
      else
	m_iStyle = 0;
      break;
    }
  }
}
