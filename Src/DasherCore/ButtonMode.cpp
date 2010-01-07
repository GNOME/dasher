// DasherButtons.cpp, build a set of boxes for Button Dasher.
// Copyright 2005, Chris Ball and David MacKay.  GPL.

// Idea - should back off button always just undo the previous 'forwards' button?

#include "../Common/Common.h"


#include "ButtonMode.h"
#include "DasherScreen.h"
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
  {LP_ZOOMSTEPS, T_LONG, 1, 63, 1, 1, _("Zoom steps")},
  {LP_BUTTON_SCAN_TIME, T_LONG, 0, 2000, 1, 100, _("Scan time in menu mode (0 to not scan)")},
  {LP_B, T_LONG, 2, 10, 1, 1, _("Number of boxes")},
  {LP_S, T_LONG, 0, 256, 1, 1, _("Safety margin")},
  /* TRANSLATORS: The boxes (zoom targets) in button mode can either be the same size, or different sizes - this is the extent to which the sizes are allowed to differ from each other. */
  /* XXX PRLW: 128 log(2) = 89, where 2 is the ratio of adjacent boxes
   * however the code seems to use ratio = (129/127)^-r, instead of
   * ratio = exp(r/128) used in the design document
   */
  {LP_R, T_LONG, -89, 89, 1, 10, _("Box non-uniformity")},
  /* TRANSLATORS: Intercept keyboard events for 'special' keys even when the Dasher window doesn't have keyboard focus.*/
  {BP_GLOBAL_KEYBOARD, T_BOOL, -1, -1, -1, -1, _("Global keyboard grab")}
};

// FIX iStyle == 0

CButtonMode::CButtonMode(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, bool bMenu, int iID, const char *szName)
: CDasherButtons(pEventHandler, pSettingsStore, pInterface, bMenu, iID, szName) {}

void CButtonMode::SetupBoxes()
{
  int iDasherY(GetLongParameter(LP_MAX_Y));

  int iForwardBoxes(GetLongParameter(LP_B));
  m_pBoxes = new SBoxInfo[m_iNumBoxes = iForwardBoxes+1];

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
    if(iForwardBoxes == 2+1) { // Special case for two forwards buttons
      dNorm = 1+dRatio;

      m_pBoxes[0].iDisplayTop = 0;
      m_pBoxes[0].iDisplayBottom = int( (1 / dNorm) * iDasherY );

      m_pBoxes[1].iDisplayTop = int( (1 / dNorm) * iDasherY );
      m_pBoxes[1].iDisplayBottom = iDasherY;
    }
    else {
      bool bEven(iForwardBoxes % 2 == 0);

      int iGeometricTerms;

      if(bEven)
        iGeometricTerms = iForwardBoxes / 2;
      else
        iGeometricTerms = (1+iForwardBoxes) / 2;

      double dMaxSize;

      if(dRatio == 1.0) {
        dMaxSize = iDasherY / iForwardBoxes;
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
        iUpBase = iForwardBoxes / 2;
        iDownBase = iUpBase - 1;
      }
      else {
        iUpBase = (iForwardBoxes - 1)/ 2;
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

bool CButtonMode::DecorateView(CDasherView *pView) {
  for(int i(0); i < m_iNumBoxes; ++i) {
    if(i != iActiveBox)
      NewDrawGoTo(pView, m_pBoxes[i].iDisplayTop, m_pBoxes[i].iDisplayBottom, false);
  }
  NewDrawGoTo(pView, m_pBoxes[iActiveBox].iDisplayTop, m_pBoxes[iActiveBox].iDisplayBottom, m_bMenu || m_bHighlight);

  bool bRV(m_bDecorationChanged);
  m_bDecorationChanged = false;
  return bRV;
}

bool CButtonMode::Timer(int Time, CDasherView *pView, CDasherModel *pModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol) {
  bool m_bOldHighlight(m_bHighlight);
  m_bHighlight = (Time - m_iLastTime < 200);
  
  if(m_bOldHighlight != m_bHighlight)
    m_bDecorationChanged = true;  

  return CDasherButtons::Timer(Time, pView, pModel, pAdded, pNumDeleted, pol);
}

void CButtonMode::KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY)
{
  if (iId == 100 && !m_bMenu) {
    //Mouse!
    myint iDasherX, iDasherY;
    pView->GetCoordinates(iDasherX, iDasherY);
    for (int i = 0; i < m_iNumBoxes; i++)
    {
      if (iDasherY < m_pBoxes[i].iDisplayBottom &&
          iDasherY > m_pBoxes[i].iDisplayTop &&
          iDasherX < (m_pBoxes[i].iDisplayBottom - m_pBoxes[i].iDisplayTop)) {
        //user has clicked in box! Simulate press of appropriate (direct-mode) button...
        CDasherButtons::KeyDown(iTime, (i==m_iNumBoxes-1) ? 1 : i+2, pView, pModel, pUserLog);
        return;
      }
    }
    //not in any box. Fall through, just to be conservative...
  }
  CInputFilter::KeyDown(iTime, iId, pView, pModel, pUserLog, bPos, iX, iY);
}

void CButtonMode::DirectKeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog) {
  CDasherButtons::DirectKeyDown(iTime, iId, pView, pModel, pUserLog);
 if (iId!=100) m_iLastTime = iTime;
}

void CButtonMode::HandleEvent(Dasher::CEvent * pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));

    switch (pEvt->m_iParameter) {
    case LP_B:
    case LP_R:
      // Delibarate fallthrough
      delete[] m_pBoxes;
      SetupBoxes();
      break;
    }
  }
}

bool CButtonMode::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
};
