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

CDynamicFilter::CDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, int iType, const char *szName)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, iID, iType, szName) { 
  m_iState = 0;
    m_bDecorationChanged = true;
  m_bKeyDown = false;
}

bool CDynamicFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) {
  if(m_bKeyDown && !m_bKeyHandled && ((Time - m_iKeyDownTime) > GetLongParameter(LP_HOLD_TIME))) {
    Event(Time, m_iHeldId, 1, m_pDasherModel, m_pUserLog);
    m_bKeyHandled = true;
    return true;
  }

  if(m_iState == 2) //backing off
    return m_pDasherModel->OneStepTowards(41943,2048, Time, pAdded, pNumDeleted);
  else if(m_iState == 1)
    return TimerImpl(Time, m_pDasherView, m_pDasherModel, pAdded, pNumDeleted);
  else
    return false;
}

void CDynamicFilter::KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog) {
  
  m_pUserLog = pUserLog;
  
  if(((iId == 0) || (iId == 1) || (iId == 100)) && !GetBoolParameter(BP_BACKOFF_BUTTON))
    return;

  if(m_bKeyDown)
    return;

  // Pass the basic key down event to the handler
  // TODO: bit of a hack here

  int iPreviousState = m_iState;
  Event(iTime, iId, 0, pModel, pUserLog);
  bool bStateChanged = m_iState != iPreviousState;
    
  // Store the key down time so that long presses can be determined
  // TODO: This is going to cause problems if multiple buttons are
  // held down at once
  m_iKeyDownTime = iTime;
  
  // Check for multiple clicks
  if(iId == m_iQueueId) {
    while((m_deQueueTimes.size() > 0) && (iTime - m_deQueueTimes.front()) > GetLongParameter(LP_MULTIPRESS_TIME))
      m_deQueueTimes.pop_front();

    if(m_deQueueTimes.size() == static_cast<unsigned int>(GetLongParameter(LP_MULTIPRESS_COUNT) - 1)) { 
      Event(iTime, iId, 2, pModel, pUserLog);
      m_deQueueTimes.clear();
    }
    else {
      if(!bStateChanged)
	m_deQueueTimes.push_back(iTime);
    }
  }
  else {
    if(!bStateChanged) {
      m_deQueueTimes.clear();
      m_deQueueTimes.push_back(iTime);
      m_iQueueId = iId;
    }
  }

  m_iHeldId = iId;
  m_bKeyDown = true;
  m_bKeyHandled = false;
}

void CDynamicFilter::KeyUp(int iTime, int iId, CDasherView *pView, CDasherModel *pModel) {
  m_bKeyDown = false;
}

void CDynamicFilter::Event(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog) {
  // Types:
  // 0 = ordinary click
  // 1 = long click
  // 2 = multiple click
  
  if(iType == 2)
    RevertPresses(GetLongParameter(LP_MULTIPRESS_COUNT) - 1);
  
  // First sanity check - if Dasher is paused then jump to the
  // appropriate state
  if(GetBoolParameter(BP_DASHER_PAUSED))
    m_iState = 0;

  // TODO: Check that state diagram implemented here is what we
  // decided upon

  // What happens next depends on the state:
  switch(m_iState) {
  case 0: // Any button when paused causes a restart
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 1);
    m_pInterface->Unpause(iTime);
    SetBoolParameter(BP_DELAY_VIEW, true);
    m_iState = 1;
    m_deQueueTimes.clear();
    break;
  case 1:
    switch(iType) {
    case 0:
      if((iButton == 0) || (iButton == 100)) {
	if(pUserLog)
	  pUserLog->KeyDown(iButton, iType, 2);
	m_iState = 0;
	m_deQueueTimes.clear();
	SetBoolParameter(BP_DELAY_VIEW, false);
	m_pInterface->PauseAt(0, 0);
      }
      else if(iButton == 1) {
	if(pUserLog)
	  pUserLog->KeyDown(iButton, iType, 6);
	SetBoolParameter(BP_DELAY_VIEW, false);
	m_iState = 2;
	m_deQueueTimes.clear();
      }
      else {
	ActionButton(iTime, iButton, iType, pModel, pUserLog);
      }
      break;
    case 1: // Delibarate fallthrough
    case 2: 
      if((iButton >= 2) && (iButton <= 4)) {
	if(pUserLog)
	  pUserLog->KeyDown(iButton, iType, 6);
	SetBoolParameter(BP_DELAY_VIEW, false);
	m_iState = 2;
	m_deQueueTimes.clear();
       }
      else {
	if(pUserLog)
	  pUserLog->KeyDown(iButton, iType, 0);
      }
      break;
    }
    break;
  case 2:
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 2);
    
    m_iState = 0;
    m_deQueueTimes.clear();
    m_pInterface->PauseAt(0, 0);
    break;
  }

  m_iLastButton = iButton;
}

bool CDynamicFilter::GetMinWidth(int &iMinWidth) {
  iMinWidth = 1024;
  return true;
}
