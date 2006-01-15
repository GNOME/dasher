// DasherButtons.cpp, build a set of boxes for Button Dasher.
// Copyright 2005, Chris Ball and David MacKay.  GPL.

#include "DasherButtons.h"
#include <valarray>
#include <iostream>

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// FIXME - should compass mode be made a separate class?

CDasherButtons::CDasherButtons(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, int iNumBoxes, int iStyle, bool bMenu)
  : CInputFilter(pEventHandler, pSettingsStore) {

  m_pBoxes = 0;

  m_iNumBoxes = iNumBoxes;
  m_iStyle = iStyle;
  m_bMenu = bMenu;

  m_pSettingsStore = pSettingsStore;

  SetupBoxes();
}

CDasherButtons::~CDasherButtons()
{
  delete[] m_pBoxes;
}

void CDasherButtons::SetupBoxes()
{ 
  if(m_iStyle == 2) { // Compass mode
    m_pBoxes = new SBoxInfo[4];

    // FIXME - need to relate these to cross-hiar position as stored in the parameters

    // Not sure whether this is at all the right algorithm here - need to check

    m_pBoxes[1].iTop = (2048 - 500) * GetLongParameter(LP_RIGHTZOOM) / (9 * 1024);
    m_pBoxes[1].iBottom = 4096 - m_pBoxes[1].iTop;

    // Make this the inverse of the right zoom option

    m_pBoxes[0].iTop = -2048 *  m_pBoxes[1].iTop / (2048 -  m_pBoxes[1].iTop);
    m_pBoxes[0].iBottom = 4096 - m_pBoxes[0].iTop;

    m_pBoxes[2].iTop = -1000;
    m_pBoxes[2].iBottom = 3096;
    m_pBoxes[3].iTop = 1000;
    m_pBoxes[3].iBottom = 5096;

    m_pBoxes[0].iDisplayTop = m_pBoxes[0].iTop; 
    m_pBoxes[0].iDisplayBottom = m_pBoxes[0].iBottom;
    m_pBoxes[1].iDisplayTop = m_pBoxes[1].iTop; 
    m_pBoxes[1].iDisplayBottom = m_pBoxes[1].iBottom;
    m_pBoxes[2].iDisplayTop = m_pBoxes[2].iTop; 
    m_pBoxes[2].iDisplayBottom = m_pBoxes[2].iBottom;
    m_pBoxes[3].iDisplayTop = m_pBoxes[3].iTop; 
    m_pBoxes[3].iDisplayBottom = m_pBoxes[3].iBottom;

  }
  else {

    if(m_iStyle == 1) 
      m_iNumBoxes = GetLongParameter(LP_B) + 1; // One extra box for backoff
    
    if(m_pBoxes) {
      delete[] m_pBoxes;
      m_pBoxes = 0;
    }

    m_pBoxes = new SBoxInfo[m_iNumBoxes];
  
    int iDasherY(GetLongParameter(LP_MAX_Y));

    for(int i(0); i < m_iNumBoxes - 1; ++i) { // One button reserved for backoff
      m_pBoxes[i].iDisplayTop = (i * iDasherY) / (m_iNumBoxes - 1);
      m_pBoxes[i].iDisplayBottom = ((i+1) * iDasherY) / (m_iNumBoxes - 1);
      
      m_pBoxes[i].iTop = m_pBoxes[i].iDisplayTop - 100; // FIXME - hardcoded value
      m_pBoxes[i].iBottom = m_pBoxes[i].iDisplayBottom + 100;
    }
    
    m_pBoxes[m_iNumBoxes-1].iDisplayTop = 0;
    m_pBoxes[m_iNumBoxes-1].iDisplayBottom = iDasherY;
    
    m_pBoxes[m_iNumBoxes-1].iTop = - iDasherY / 2;
    m_pBoxes[m_iNumBoxes-1].iBottom = iDasherY * 1.5;
  }
 
  iActiveBox = 0;
}

void CDasherButtons::DecorateView(CDasherView *pView) {
  if(m_iStyle == 2) {
    CDasherScreen *pScreen(pView->Screen());

    for(int i(-3); i <= 3; i+=2) {
      CDasherScreen::point p[2];

      myint iDasherX;
      myint iDasherY;

      iDasherX = -100;
      iDasherY = 2048 + 500 * i;

      pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);

      iDasherX = -1000;
      iDasherY = 2048 + 500 * i;
      
      pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);

      pScreen->Polyline(p, 2, 1, 1);
    }
  }
  else {
    for(int i(0); i < m_iNumBoxes; ++i) {
      if(i != iActiveBox)
	pView->NewDrawGoTo(m_pBoxes[i].iDisplayTop, m_pBoxes[i].iDisplayBottom, false);
    }
    pView->NewDrawGoTo(m_pBoxes[iActiveBox].iDisplayTop, m_pBoxes[iActiveBox].iDisplayBottom, m_bMenu);
  }
}
 

void CDasherButtons::KeyDown(int iTime, int iId, CDasherModel *pModel) {

  if(m_bMenu) {
    switch(iId) {
    case 1:
      ++iActiveBox;
      if(iActiveBox == m_iNumBoxes)
	iActiveBox = 0;
      break;
    case 2:
      pModel->ScheduleZoom((m_pBoxes[iActiveBox].iBottom - m_pBoxes[iActiveBox].iTop)/2, (m_pBoxes[iActiveBox].iBottom + m_pBoxes[iActiveBox].iTop)/2);
      break;
    }
  }
  else {
    if(iId <= m_iNumBoxes) 
      pModel->ScheduleZoom((m_pBoxes[iId-1].iBottom - m_pBoxes[iId-1].iTop)/2, (m_pBoxes[iId-1].iBottom + m_pBoxes[iId-1].iTop)/2);
  }

}

void CDasherButtons::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  m_pDasherModel->Tap_on_display(0, 0, Time, 0, 0);
}

void CDasherButtons::HandleEvent(Dasher::CEvent * pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));

    switch (pEvt->m_iParameter) {
    case LP_B:
    case LP_RIGHTZOOM:
      // Delibarate fallthrough
      SetupBoxes();
      break;
    }
  }
}
