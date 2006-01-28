
#include "../Common/Common.h"

#include "OneButtonFilter.h"
#include "Event.h"

COneButtonFilter::COneButtonFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface) {

  bStarted = 0;
  iLocation = 0;

}

COneButtonFilter::~COneButtonFilter() {
}

void COneButtonFilter::DecorateView(CDasherView *pView) {
  CDasherScreen *pScreen(pView->Screen());
  CDasherScreen::point p[2];
 
  myint iDasherX;
  myint iDasherY;
  
  iDasherX = -100;
  iDasherY = iLocation;
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = iLocation;
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  pScreen->Polyline(p, 2, 1, 1);
}

void COneButtonFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {

  if(bStarted)
    iLocation = (Time - iStartTime) * 4096 / 1000;

  if(iLocation > 4096) {
    iLocation = 0;
    iStartTime = Time;
  }
    

}

void COneButtonFilter::KeyDown(int iTime, int iId, CDasherModel *pModel) {
  bStarted = true;
  iStartTime = iTime;
}

void COneButtonFilter::HandleEvent(Dasher::CEvent * pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    
    switch (pEvt->m_iParameter) {
    default:
      break;
    }
  }
}
