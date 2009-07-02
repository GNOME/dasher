// TwoButtonDynamicFilter.cpp
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "../Common/Common.h"

#include "TwoButtonDynamicFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

static SModuleSettings sSettings[] = {
  {LP_TWO_BUTTON_OFFSET, T_LONG, 1024, 2048, 2048, 100, _("Button offset")},
  /* TRANSLATORS: The time for which a button must be held before it counts as a 'long' (rather than short) press. */
  {LP_HOLD_TIME, T_LONG, 100, 10000, 1000, 100, _("Long press time")},
  /* TRANSLATORS: Multiple button presses are special (like a generalisation on double clicks) in some situations. This is the maximum time between two presses to count as _part_of_ a multi-press gesture
  (potentially more than two presses). */
  {LP_MULTIPRESS_TIME, T_LONG, 100, 10000, 1000, 100, _("Multiple press interval")},
  /* TRANSLATORS: Backoff = reversing in Dasher to correct mistakes. This allows a single button to be dedicated to activating backoff, rather than using multiple presses of other buttons, and another to be dedicated to starting and stopping. 'Button' in this context is a physical hardware device, not a UI element.*/
  {BP_BACKOFF_BUTTON,T_BOOL, -1, -1, -1, -1, _("Enable backoff and start/stop buttons")},
  /* TRANSLATORS: What is normally the up button becomes the down button etc. */
  {BP_TWOBUTTON_REVERSE,T_BOOL, -1, -1, -1, -1, _("Reverse up and down buttons")},
  /* TRANSLATORS: Pushing the up/down button twice quickly has the same effect as pushing the other
  button once; in this case, one must push three times (or push-and-hold) to reverse. */
  {BP_2B_INVERT_DOUBLE, T_BOOL, -1, -1, -1, -1, _("Double-click is opposite up/down - triple to reverse")},
  {BP_SLOW_START,T_BOOL, -1, -1, -1, -1, _("Slow startup")},
  {LP_SLOW_START_TIME, T_LONG, 0, 10000, 1000, 100, _("Startup time")},
  {LP_DYNAMIC_BUTTON_LAG, T_LONG, 0, 1000, 1, 25, _("Lag before user actually pushes button (ms)")}, 
  {LP_DYNAMIC_SPEED_INC, T_LONG, 1, 100, 1, 1, _("%age by which to automatically increase speed")},
  {LP_DYNAMIC_SPEED_FREQ, T_LONG, 1, 1000, 1, 1, _("Time after which to automatically increase speed (secs)")},
  {LP_DYNAMIC_SPEED_DEC, T_LONG, 1, 99, 1, 1, _("%age by which to decrease speed upon reverse")}
};

CTwoButtonDynamicFilter::CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CButtonMultiPress(pEventHandler, pSettingsStore, pInterface, 14, 1, _("Two Button Dynamic Mode")), m_dMulSinceFirstPush(1.0)
{
  //ensure that m_dLagMul is properly initialised
  Dasher::CParameterNotificationEvent oEvent(LP_DYNAMIC_BUTTON_LAG);
  HandleEvent(&oEvent);
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
  
  pScreen->Polyline(p, 2, 3, 242);

  iDasherX = -100;
  iDasherY = 2048 + GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = 2048 + GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  pScreen->Polyline(p, 2, 3, 242);

  bool bRV(m_bDecorationChanged);
  m_bDecorationChanged = false;
  return bRV;
}

bool CTwoButtonDynamicFilter::TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) {
  return m_pDasherModel->OneStepTowards(100,2048, Time, pAdded, pNumDeleted);
}

void CTwoButtonDynamicFilter::Activate() {
  SetBoolParameter(BP_SMOOTH_OFFSET, true);
}

void CTwoButtonDynamicFilter::Deactivate() {
  SetBoolParameter(BP_SMOOTH_OFFSET, false);
}

void CTwoButtonDynamicFilter::run(int iState) {
  SetBoolParameter(BP_SMOOTH_OFFSET, true);
  CButtonMultiPress::run(iState);
}

void CTwoButtonDynamicFilter::pause() {
  SetBoolParameter(BP_SMOOTH_OFFSET, false);
  CButtonMultiPress::pause();
}

void CTwoButtonDynamicFilter::reverse() {
  //hmmmm. If we ever actually did Offset() while reversing,
  // we might want BP_SMOOTH_OFFSET on....
  SetBoolParameter(BP_SMOOTH_OFFSET, false);
  CButtonMultiPress::reverse();
}

void CTwoButtonDynamicFilter::Event(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog)
{
  if (GetBoolParameter(BP_2B_INVERT_DOUBLE) && iType == 2 && iButton>=2 && iButton<=4)
  { //double-press - treat as single-press of the other button....
    iType = 0; //0=normal, 1=long press
    iButton = (iButton == 2) ? 3 : 2;
    m_dMulSinceFirstPush = exp(pModel->GetNats());
  }
  CDynamicFilter::Event(iTime, iButton, iType, pModel, pUserLog);
}
    
void CTwoButtonDynamicFilter::ApplyOffset(CDasherModel *pModel, long lOffset)
{
  lOffset *= m_dMulSinceFirstPush; m_dMulSinceFirstPush = 1.0;
  pModel->Offset(lOffset);
  m_pModel = pModel;
  m_lOffsetApplied = lOffset;
  pModel->ResetNats();
}

void CTwoButtonDynamicFilter::RevertPresses(int iCount)
{
  //invert the *last* invocation of ApplyOffset.
  //this'll handle reverting single clicks and (if BP_2B_INVERT_DOUBLE is on) double-clicks,
  //but we'll get into trouble if the user e.g. double-presses the reverse button!

  //correct for expansion since the first click, if any (if we've rendered any frames!)
  m_pModel->Offset(-m_lOffsetApplied * exp(m_pModel->GetNats()));
  m_lOffsetApplied = 0;
}

void CTwoButtonDynamicFilter::ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog) {
  int iFactor(1);

  if(GetBoolParameter(BP_TWOBUTTON_REVERSE))
    iFactor = -1;

  if(iButton == 2) {
    ApplyOffset(pModel, iFactor * GetLongParameter(LP_TWO_BUTTON_OFFSET) * m_dLagMul);
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 3);
  }
  else if((iButton == 3) || (iButton == 4)) {
    ApplyOffset(pModel, iFactor * -GetLongParameter(LP_TWO_BUTTON_OFFSET) * m_dLagMul);
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 4);
  }
  else {
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 0);
  }
}

bool CTwoButtonDynamicFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
};

bool CTwoButtonDynamicFilter::GetMinWidth(int &iMinWidth) {
  iMinWidth = 1024;
  return true;
}

void CTwoButtonDynamicFilter::HandleEvent(Dasher::CEvent *pEvent)
{
  if (pEvent->m_iEventType == EV_PARAM_NOTIFY)
  {
    Dasher::CParameterNotificationEvent *pEvt = static_cast<Dasher::CParameterNotificationEvent *>(pEvent);
    switch (pEvt->m_iParameter)
    {
    case LP_MAX_BITRATE:
    case LP_BOOSTFACTOR: // Deliberate fallthrough
    case LP_DYNAMIC_BUTTON_LAG:
      {
        double dMaxRate = GetLongParameter(LP_MAX_BITRATE) * GetLongParameter(LP_BOOSTFACTOR) / 10000.0;
        m_dLagMul = exp(dMaxRate * GetLongParameter(LP_DYNAMIC_BUTTON_LAG)/1000.0);
      }
    }
  }
}
