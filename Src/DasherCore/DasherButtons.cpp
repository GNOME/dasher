// DasherButtons.cpp, build a set of boxes for Button Dasher.
// Copyright 2005, Chris Ball and David MacKay.  GPL.

// Idea - should back off button always just undo the previous 'forwards' button?

#include "../Common/Common.h"


#include "DasherButtons.h"
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

static SModuleSettings sSettings[] = {
  /* TRANSLATORS: The number of time steps over which to perform the zooming motion in button mode. */
  {LP_ZOOMSTEPS, T_LONG, 1, 63, 1, 1, _("Zoom steps")},
  /* TRANSLATORS: The zoom factor per press when moving to the right in compass mode. */
  {LP_RIGHTZOOM, T_LONG, 1024, 10240, 1024, 1024, _("Right zoom")},
  {LP_B, T_LONG, 2, 10, 1, 1, _("Number of boxes")},
  {LP_S, T_LONG, 0, 256, 1, 1, _("Safety margin")},
  /* TRANSLATORS: The boxes (zoom targets) in button mode can either be the same size, or different sizes - this is the extent to which the sizes are allowed to differ from each other. */
  {LP_R, T_LONG, -400, 400, 1, 10, _("Box non-uniformity")},
  /* TRANSLATORS: Intercept keyboard events for 'special' keys even when the Dasher window doesn't have keyboard focus.*/
  {BP_GLOBAL_KEYBOARD, T_BOOL, -1, -1, -1, -1, _("Global keyboard grab")}
};

// FIXME - should compass mode be made a separate class?

CDasherButtons::CDasherButtons(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, int iNumBoxes, int iStyle, bool bMenu, ModuleID_t iID, const char *szName)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, iID, 1, szName) {

  m_pBoxes = 0;

  m_iNumBoxes = iNumBoxes;
  m_iStyle = iStyle;
  m_bMenu = bMenu;

  m_pSettingsStore = pSettingsStore;

  m_bDecorationChanged = true;

  SetupBoxes();
}

CDasherButtons::~CDasherButtons()
{
  delete[] m_pBoxes;
}

void CDasherButtons::SetupBoxes()
{
  
  int iDasherY(GetLongParameter(LP_MAX_Y));


  if(m_iStyle == 3) {
    // Alternating direct mode

    m_pBoxes = new SBoxInfo[5];

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

    m_iNumBoxes = 5; 
    m_pBoxes[m_iNumBoxes-1].iDisplayTop = 0;
    m_pBoxes[m_iNumBoxes-1].iDisplayBottom = iDasherY;
    
    m_pBoxes[m_iNumBoxes-1].iTop = int(- iDasherY / 2);
    m_pBoxes[m_iNumBoxes-1].iBottom = int(iDasherY * 1.5);

    m_iLastBox = -1;
    
  }
  else if(m_iStyle == 2) { // Compass mode
    m_pBoxes = new SBoxInfo[4];

    iTargetWidth = iDasherY * 1024 / GetLongParameter(LP_RIGHTZOOM);

    // FIXME - need to relate these to cross-hair position as stored in the parameters

    // Not sure whether this is at all the right algorithm here - need to check

    m_pBoxes[1].iTop = (2048 - iTargetWidth / 2);
    m_pBoxes[1].iBottom = 4096 - m_pBoxes[1].iTop;

    // Make this the inverse of the right zoom option

    m_pBoxes[3].iTop = -2048 *  m_pBoxes[1].iTop / (2048 -  m_pBoxes[1].iTop);
    m_pBoxes[3].iBottom = 4096 - m_pBoxes[3].iTop;

    m_pBoxes[0].iTop = -iTargetWidth;
    m_pBoxes[0].iBottom = iDasherY - iTargetWidth;
    m_pBoxes[2].iTop = iTargetWidth;
    m_pBoxes[2].iBottom = iDasherY + iTargetWidth;

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

    if((m_iStyle == 1) || (m_iStyle == 0)) 
      m_iNumBoxes = GetLongParameter(LP_B) + 1; // One extra box for backoff
    
    if(m_pBoxes) {
      delete[] m_pBoxes;
      m_pBoxes = 0;
    }

    m_pBoxes = new SBoxInfo[m_iNumBoxes];
    int iForwardBoxes(m_iNumBoxes - 1);

    // Calculate the sizes of non-uniform boxes using standard
    // geometric progression results

    double dRatio;
    double dNorm;
    
    // FIXME - implement this using DJCM's integer method?
    // See ~mackay/dasher/buttons/
    dRatio = pow(129/127.0, -static_cast<double>(GetLongParameter(LP_R)));

    if(m_bMenu) {

      double dMaxSize;
      if(dRatio == 1.0)
	dMaxSize = iDasherY / static_cast<double>(iForwardBoxes);
      else
	dMaxSize = ((dRatio - 1)/(pow(dRatio, iForwardBoxes) - 1)) * iDasherY; 
      
      double dMin(0.0);
      double dMax;
      
      for(int i(0); i < m_iNumBoxes - 1; ++i) { // One button reserved for backoff
	dMax = dMin + dMaxSize * pow(dRatio, i);

//       m_pBoxes[i].iDisplayTop = (i * iDasherY) / (m_iNumBoxes - 1);
//       m_pBoxes[i].iDisplayBottom = ((i+1) * iDasherY) / (m_iNumBoxes - 1);
	
	m_pBoxes[i].iDisplayTop = static_cast<int>(dMin);
	m_pBoxes[i].iDisplayBottom = static_cast<int>(dMax);
	
	m_pBoxes[i].iTop = m_pBoxes[i].iDisplayTop - GetLongParameter(LP_S);
	m_pBoxes[i].iBottom = m_pBoxes[i].iDisplayBottom + GetLongParameter(LP_S);
	
	dMin = dMax;
      }

    }
    else {      
      if(m_iNumBoxes == 2+1) { // Special case for two forwards buttons
	dNorm = 1+dRatio;

	m_pBoxes[0].iDisplayTop = 0;
	m_pBoxes[0].iDisplayBottom = int( (1 / dNorm) * iDasherY );
	
	m_pBoxes[1].iDisplayTop = int( (1 / dNorm) * iDasherY );
	m_pBoxes[1].iDisplayBottom = iDasherY;
      }
      else {
	int iForwardsButtons(m_iNumBoxes - 1);
	bool bEven(iForwardsButtons % 2 == 0);

	int iGeometricTerms;

	if(bEven)
	  iGeometricTerms = iForwardsButtons / 2;
	else
	  iGeometricTerms = (1+iForwardsButtons) / 2;

	double dMaxSize;

	if(dRatio == 1.0) {
	  dMaxSize = iDasherY / iForwardsButtons;
	}
	else {
	  if(bEven)
	    dMaxSize = iDasherY * (dRatio - 1) / (2 * (pow(dRatio, iGeometricTerms) - 1));
	  else
	    dMaxSize = iDasherY * (dRatio - 1) / (2 * (pow(dRatio, iGeometricTerms) - 1) - (dRatio - 1));
	}

	double dMin;
	double dMax;
	
	if(bEven)
	  dMin = iDasherY / 2;
	else
	  dMin = (iDasherY - dMaxSize)/2;

	int iUpBase;
	int iDownBase;

	if(bEven) {
	  iUpBase = iForwardsButtons / 2;
	  iDownBase = iUpBase - 1;
	}
	else {
	  iUpBase = (iForwardsButtons - 1)/ 2;
	  iDownBase = iUpBase;
	}
	
	for(int i(0); i < iGeometricTerms; ++i) { // One button reserved for backoff
	  dMax = dMin + dMaxSize * pow(dRatio, i);
	  
	  m_pBoxes[iUpBase + i].iDisplayTop = int(dMin);
	  m_pBoxes[iUpBase + i].iDisplayBottom = int(dMax);

	  m_pBoxes[iDownBase - i].iDisplayTop = int(iDasherY - dMax);
	  m_pBoxes[iDownBase - i].iDisplayBottom = int(iDasherY - dMin);

	  dMin = dMax;
	}
      }
    }

    for(int i(0); i < m_iNumBoxes - 1; ++i) {
      m_pBoxes[i].iTop = m_pBoxes[i].iDisplayTop - GetLongParameter(LP_S);
      m_pBoxes[i].iBottom = m_pBoxes[i].iDisplayBottom + GetLongParameter(LP_S);
    }
    
    m_pBoxes[m_iNumBoxes-1].iDisplayTop = 0;
    m_pBoxes[m_iNumBoxes-1].iDisplayBottom = iDasherY;
    
    m_pBoxes[m_iNumBoxes-1].iTop = int(- iDasherY / 2);
    m_pBoxes[m_iNumBoxes-1].iBottom = int(iDasherY * 1.5);
  }
 
  iActiveBox = 0;
  m_bDecorationChanged = true;
}

bool CDasherButtons::DecorateView(CDasherView *pView) {
  if(m_iStyle == 2) {
    CDasherScreen *pScreen(pView->Screen());

    int iPos(2048 - iTargetWidth / 2);

    bool bFirst(true);

    while(iPos >= 0) {
      CDasherScreen::point p[2];

      myint iDasherX;
      myint iDasherY;

      iDasherX = -100;
      iDasherY = iPos;

      pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);

      iDasherX = -1000;
      iDasherY = iPos;
      
      pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);

      if(bFirst)
	pScreen->Polyline(p, 2, 1, 1);
      else
	pScreen->Polyline(p, 2, 1, 2);

      iDasherX = -100;
      iDasherY = 4096 - iPos;

      pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);

      iDasherX = -1000;
      iDasherY = 4096 - iPos;
      
      pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);

      if(bFirst)
	pScreen->Polyline(p, 2, 1, 1);
      else
	pScreen->Polyline(p, 2, 1, 2);

      iPos -= iTargetWidth;
      bFirst = false;
    }
  }
  else if(m_iStyle == 3) {
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
  }
  else {
    for(int i(0); i < m_iNumBoxes; ++i) {
      if(i != iActiveBox)
	NewDrawGoTo(pView, m_pBoxes[i].iDisplayTop, m_pBoxes[i].iDisplayBottom, false);
    }
    NewDrawGoTo(pView, m_pBoxes[iActiveBox].iDisplayTop, m_pBoxes[iActiveBox].iDisplayBottom, m_bMenu || m_bHighlight);
  }

  bool bRV(m_bDecorationChanged);
  m_bDecorationChanged = false;
  return bRV;
}
 

void CDasherButtons::KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog) {

  if(m_bMenu) {
    switch(iId) {
    case 1:
    case 4:
      m_bDecorationChanged = true;
      ++iActiveBox;
      if(iActiveBox == m_iNumBoxes)
	iActiveBox = 0;
       break;
    case 2:
    case 3:
      m_bDecorationChanged = true;
      pModel->ScheduleZoom((m_pBoxes[iActiveBox].iBottom - m_pBoxes[iActiveBox].iTop)/2, (m_pBoxes[iActiveBox].iBottom + m_pBoxes[iActiveBox].iTop)/2);
      if(iActiveBox != m_iNumBoxes-1)
	iActiveBox = 0;
      break;
    }
  }
  else {
    if(m_iStyle == 3) {
      switch(iId) {
      case 2:
	if(m_iLastBox == 1)
	  pModel->ScheduleZoom((m_pBoxes[2].iBottom - m_pBoxes[2].iTop)/2, (m_pBoxes[2].iBottom + m_pBoxes[2].iTop)/2);
	else
	  pModel->ScheduleZoom((m_pBoxes[0].iBottom - m_pBoxes[0].iTop)/2, (m_pBoxes[0].iBottom + m_pBoxes[0].iTop)/2);
	m_iLastBox = 1;
	break; 
      case 3:
      case 4:
	if(m_iLastBox == 2)
	  pModel->ScheduleZoom((m_pBoxes[3].iBottom - m_pBoxes[3].iTop)/2, (m_pBoxes[3].iBottom + m_pBoxes[3].iTop)/2);
	else
	  pModel->ScheduleZoom((m_pBoxes[1].iBottom - m_pBoxes[1].iTop)/2, (m_pBoxes[1].iBottom + m_pBoxes[1].iTop)/2);
	m_iLastBox = 2;
	break;
      case 1:
	pModel->ScheduleZoom((m_pBoxes[4].iBottom - m_pBoxes[4].iTop)/2, (m_pBoxes[4].iBottom + m_pBoxes[4].iTop)/2);
	break;
      }
    }
    else {
      if(iId == 100) // Ignore mouse events
	return;
      if(iId == 1)
	iActiveBox = m_iNumBoxes - 1;
      else if(iId <= m_iNumBoxes) 
	iActiveBox = iId-2;
      else
	iActiveBox = m_iNumBoxes-2;

      m_iLastTime = iTime;
	
      pModel->ScheduleZoom((m_pBoxes[iActiveBox].iBottom - m_pBoxes[iActiveBox].iTop)/2, (m_pBoxes[iActiveBox].iBottom + m_pBoxes[iActiveBox].iTop)/2);
    }
  }

}

bool CDasherButtons::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) {
  bool m_bOldHighlight(m_bHighlight);
  m_bHighlight = (Time - m_iLastTime < 200);

  if(m_bOldHighlight != m_bHighlight)
    m_bDecorationChanged = true;

  // TODO: This is a bit of a hack to make joystick mode work
  myint iDasherX;
  myint iDasherY;

  m_pDasherView->GetCoordinates(iDasherX, iDasherY);
  // ----

  return m_pDasherModel->UpdatePosition(0, 0, Time, pAdded, pNumDeleted);
}

void CDasherButtons::HandleEvent(Dasher::CEvent * pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));

    switch (pEvt->m_iParameter) {
    case LP_B:
    case LP_RIGHTZOOM:
    case LP_R:
      // Delibarate fallthrough
      SetupBoxes();
      break;
    }
  }
}

bool CDasherButtons::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
};

void CDasherButtons::NewDrawGoTo(CDasherView *pView, myint iDasherMin, myint iDasherMax, bool bActive) {
   myint iHeight(iDasherMax - iDasherMin);

   int iColour;
   int iWidth;

   if(bActive) {
     iColour = 1;
     iWidth = 3;
   }
   else {
     iColour = 2;
     iWidth = 1;
   }

   CDasherScreen::point p[4];

   pView->Dasher2Screen( 0, iDasherMin, p[0].x, p[0].y);
   pView->Dasher2Screen( iHeight, iDasherMin, p[1].x, p[1].y);
   pView->Dasher2Screen( iHeight, iDasherMax, p[2].x, p[2].y);
   pView->Dasher2Screen( 0, iDasherMax, p[3].x, p[3].y);

   pView->Screen()->Polyline(p, 4, iWidth, iColour);
}
