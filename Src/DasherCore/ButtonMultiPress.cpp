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

using namespace Dasher;

CButtonMultiPress::CButtonMultiPress(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName)
  : CDynamicButtons(pCreator, pInterface, pFramerate, iID, szName) {
}

void CButtonMultiPress::Timer(unsigned long iTime, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CExpansionPolicy **pol) {
  if(m_bKeyDown && !m_bKeyHandled && ((iTime - m_iKeyDownTime) > GetLongParameter(LP_HOLD_TIME))) {
    ButtonEvent(iTime, m_iHeldId, 1, pModel);
    m_bKeyHandled = true;
  }
  CDynamicButtons::Timer(iTime,pView,pInput,pModel,pol);
}

void CButtonMultiPress::KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {

  if (m_bKeyDown) return;

  // Check for multiple clicks
  if(iId == m_iQueueId && m_deQueueTimes.size()) {
    if ( (iTime - m_deQueueTimes.back()) > GetLongParameter(LP_MULTIPRESS_TIME) )
      m_deQueueTimes.clear(); //and fall through to record+process normally, below
    else
    {
      //previous presses should not be treated as such....
      RevertPresses(m_deQueueTimes.size());
      //...but should be combined with this one into a new event (type = #presses)
      ButtonEvent(iTime, iId, m_deQueueTimes.size()+1, pModel);
      if (m_deQueueTimes.size() >= maxClickCount() - 1)
	m_deQueueTimes.clear(); //final press
      else //may still be more presses to come
	m_deQueueTimes.push_back(iTime);
      return; //we've called Event ourselves, so finished.
    }
  } else {
    m_deQueueTimes.clear(); //clear record of previous, different, button
    m_iQueueId = iId;
  }

  // Record press...
  m_deQueueTimes.push_back(iTime);
  // ... and process normally; if it changes the state, pause()/reverse()'ll clear the queue
  CDynamicButtons::KeyDown(iTime, iId, pView, pInput, pModel);
  
  // Store the key down time so that long presses can be determined
  // TODO: This is going to cause problems if multiple buttons are
  // held down at once
  m_iKeyDownTime = iTime;
  
  m_bKeyHandled = false;
}


void CButtonMultiPress::pause() {
  CDynamicButtons::pause();
  m_deQueueTimes.clear();
}

void CButtonMultiPress::reverse(unsigned long iTime) {
  CDynamicButtons::reverse(iTime);
  m_deQueueTimes.clear();
}

void CButtonMultiPress::run(unsigned long iTime) {
  if (!isRunning()) m_deQueueTimes.clear();
  CDynamicButtons::run(iTime);
}
