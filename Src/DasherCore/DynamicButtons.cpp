// DynamicButtons.cpp
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

#include "DasherInterfaceBase.h"
#include "DynamicButtons.h"

using namespace Dasher;

CDynamicButtons::CDynamicButtons(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName)
  : CDynamicFilter(pCreator, pInterface, pFramerate, iID, szName), CSettingsObserver(pCreator) {
  m_bDecorationChanged = true;
  m_bKeyDown = false;
  pause();
}

bool CDynamicButtons::Timer(unsigned long iTime, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol)
{
  if(m_bKeyDown && !m_bKeyHandled && ((iTime - m_iKeyDownTime) > GetLongParameter(LP_HOLD_TIME))) {
    Event(iTime, m_iHeldId, 1, m_pDasherModel, m_pUserLog);
    m_bKeyHandled = true;
    //return true; //ACL although that's what old DynamicButtons did, surely we should progress normally?
  }
  if (isPaused()) return false;
  if (isReversing()) {
    OneStepTowards(m_pDasherModel, 41943,2048, iTime, SlowStartSpeedMul(iTime));
    return true;
  }
  //moving forwards. Check auto speed control...
  unsigned int uTime = static_cast<unsigned int>(iTime);
  if (GetBoolParameter(BP_AUTO_SPEEDCONTROL) && m_uSpeedControlTime < uTime)
  {
	  if (m_uSpeedControlTime > 0) //has actually been set?
        SetLongParameter(LP_MAX_BITRATE, GetLongParameter(LP_MAX_BITRATE) * (1.0 + GetLongParameter(LP_DYNAMIC_SPEED_INC)/100.0));
	  m_uSpeedControlTime = uTime + 1000*GetLongParameter(LP_DYNAMIC_SPEED_FREQ);
  }
  return TimerImpl(iTime, pDasherView, m_pDasherModel, pol);
}

void CDynamicButtons::KeyDown(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog) {
  
  m_pUserLog = pUserLog;
  
  if(((iId == 0) || (iId == 1) || (iId == 100)) && !GetBoolParameter(BP_BACKOFF_BUTTON))
    return;

  if(m_bKeyDown)
    return;

  // Pass the basic key down event to the handler
  Event(iTime, iId, 0, pModel, pUserLog);
    
  // Store the key down time so that long presses can be determined
  // TODO: This is going to cause problems if multiple buttons are
  // held down at once
  m_iKeyDownTime = iTime;

  m_iHeldId = iId;
  m_bKeyDown = true;
  m_bKeyHandled = false;
}

void CDynamicButtons::KeyUp(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  if (iId == m_iHeldId) m_bKeyDown = false;
}

void CDynamicButtons::Event(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog) {
  // Types known at this point in inheritance hierarchy:
  // 0 = ordinary click
  // 1 = long click
  
  // TODO: Check that state diagram implemented here is what we
  // decided upon

  // What happens next depends on the state:
  if (isPaused()) {
    //Any button causes a restart
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 1);
    run();
    Unpause(iTime);
  } else if (isReversing()) {
    //Any button pauses
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 2);
    
    m_pInterface->Stop();
    //change in BP_DASHER_PAUSED calls pause().
  } else {
    //running; examine event/button-press type
    switch(iType) {
    case 0: //single press
      if((iButton == 0) || (iButton == 100)) {
        //dedicated pause button
        if(pUserLog)
          pUserLog->KeyDown(iButton, iType, 2);
        m_pInterface->Stop();
        break;
      }
      else if(iButton == 1) {
        //dedicated reverse button
        if(pUserLog)
          pUserLog->KeyDown(iButton, iType, 6);
        reverse();
        break;
      }
      //else - any non-special button - fall through
    default: //or, Any special kind of event - long, double, triple, ... 
      ActionButton(iTime, iButton, iType, pModel, pUserLog);
    }
  }
}

void CDynamicButtons::HandleEvent(int iParameter) {
  if (iParameter==BP_DASHER_PAUSED) {
    if (GetBoolParameter(BP_DASHER_PAUSED))
      pause(); //make sure we're in sync
    else if (m_pInterface->GetActiveInputMethod()==this && isPaused())
      //if we're active: can't unpause, as we don't know which way to go, run or reverse?
      SetBoolParameter(BP_DASHER_PAUSED, true);
  }
}

void CDynamicButtons::reverse()
{
  m_iState = 1;
  if (GetBoolParameter(BP_AUTO_SPEEDCONTROL)) {
    //treat reversing as a sign of distress --> slow down!
    SetLongParameter(LP_MAX_BITRATE, GetLongParameter(LP_MAX_BITRATE) *
					 (1.0 - GetLongParameter(LP_DYNAMIC_SPEED_DEC)/100.0));
  }
}

void CDynamicButtons::run()
{
  if (m_iState<2) //wasn't running previously
    m_uSpeedControlTime = 0; //will be set in Timer()
  m_iState = 2;
}

