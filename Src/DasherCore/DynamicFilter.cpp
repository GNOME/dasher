// DynamicFilter.cpp
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
#include "DynamicFilter.h"

using namespace Dasher;

CDynamicFilter::CDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, int iType, const char *szName)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, iID, iType, szName) {
  m_bDecorationChanged = true;
  m_bKeyDown = false;
  pause();
}

bool CDynamicFilter::Timer(int iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol)
{
  if(m_bKeyDown && !m_bKeyHandled && ((iTime - m_iKeyDownTime) > GetLongParameter(LP_HOLD_TIME))) {
    Event(iTime, m_iHeldId, 1, m_pDasherModel, m_pUserLog);
    m_bKeyHandled = true;
    //return true; //ACL although that's what old DynamicFilter did, surely we should progress normally?
  }
  if (isPaused()) return false;
  if (isReversing()) {
    m_pDasherModel->OneStepTowards(41943,2048, iTime, pAdded, pNumDeleted);
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
  return TimerImpl(iTime, m_pDasherView, m_pDasherModel, pAdded, pNumDeleted, pol);
}

void CDynamicFilter::KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog) {
  
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

void CDynamicFilter::KeyUp(int iTime, int iId, CDasherView *pView, CDasherModel *pModel) {
  if (iId == m_iHeldId) m_bKeyDown = false;
}

void CDynamicFilter::Event(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog) {
  // Types known at this point in inheritance hierarchy:
  // 0 = ordinary click
  // 1 = long click
  
  // First sanity check - if Dasher is paused then jump to the
  // appropriate state
  if(GetBoolParameter(BP_DASHER_PAUSED) && !isPaused())
    pause();

  // TODO: Check that state diagram implemented here is what we
  // decided upon

  // What happens next depends on the state:
  if (isPaused()) {
  //Any button causes a restart
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 1);
    m_pInterface->Unpause(iTime);
    run();
  } else if (isReversing()) {
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 2);
    
    pause();
    m_pInterface->PauseAt(0, 0);
  } else {
    //running; examine event/button-press type
    switch(iType) {
    case 0: //single press
      if((iButton == 0) || (iButton == 100)) {
        //dedicated pause button
        if(pUserLog)
          pUserLog->KeyDown(iButton, iType, 2);
        pause();
        m_pInterface->PauseAt(0, 0);
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

void CDynamicFilter::reverse()
{
  m_iState = 1;
  if (GetBoolParameter(BP_AUTO_SPEEDCONTROL))
  {
	//treat reversing as a sign of distress --> slow down!
	SetLongParameter(LP_MAX_BITRATE, GetLongParameter(LP_MAX_BITRATE) *
					 (1.0 - GetLongParameter(LP_DYNAMIC_SPEED_DEC)/100.0));
  }
}

void CDynamicFilter::run()
{
  if (m_iState<2) //wasn't running previously
    m_uSpeedControlTime = 0; //will be set in Timer()
  m_iState = 2;
}

