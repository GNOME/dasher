// ButtonMultiPress.cpp
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
#include "ButtonMultiPress.h"

CButtonMultiPress::CButtonMultiPress(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, int iType, const char *szName)
  : CDynamicFilter(pEventHandler, pSettingsStore, pInterface, iID, iType, szName) {
}

void CButtonMultiPress::KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog) {

  if (m_bKeyDown) return;
      
  // Check for multiple clicks
  if(iId == m_iQueueId) {
    while((m_deQueueTimes.size() > 0) && (iTime - m_deQueueTimes.front()) > GetLongParameter(LP_MULTIPRESS_TIME))
      m_deQueueTimes.pop_front();

    //if that's the final press...  
    if(m_deQueueTimes.size() == static_cast<unsigned int>(GetLongParameter(LP_MULTIPRESS_COUNT) - 1)) { 
      //undo the preceding ones...
      RevertPresses(GetLongParameter(LP_MULTIPRESS_COUNT) - 1);
      //execute the event
      Event(iTime, iId, 2, pModel, pUserLog);
      m_deQueueTimes.clear();
    }
    else {
      //not the final press; so record...
    m_deQueueTimes.push_back(iTime);
      //and process normally
      //(this may clear the queue if it changes the state)
      CDynamicFilter::KeyDown(iTime, iId, pView, pModel, pUserLog);
    }
  }
  else {
    //record as first press...
    m_deQueueTimes.clear();
    m_deQueueTimes.push_back(iTime);
    m_iQueueId = iId;
    //...and process normally; if it changes the state, pause()/reverse()'ll clear the queue
    CDynamicFilter::KeyDown(iTime, iId, pView, pModel, pUserLog);
  }
}

void CButtonMultiPress::pause()
{
  CDynamicFilter::pause();
  m_deQueueTimes.clear();
}

void CButtonMultiPress::reverse()
{
  CDynamicFilter::reverse();
  m_deQueueTimes.clear();
}

void CButtonMultiPress::run(int newState)
{
  int oldState;
  if (isRunning(oldState))
  {
    if (oldState == newState) return;
  }
  CDynamicFilter::run(newState);
  m_deQueueTimes.clear();
}