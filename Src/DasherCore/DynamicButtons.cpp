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
  : CDynamicFilter(pCreator, pInterface, pFramerate, iID, szName), m_pModel(NULL) {
  m_bDecorationChanged = true;
  m_bKeyDown = false;
  pause();
}

void CDynamicButtons::Timer(unsigned long iTime, CDasherView *pDasherView, CDasherInput *pInput, CDasherModel *pModel, CExpansionPolicy **pol) {
  if (isPaused()) return;
  if (isReversing()) {
    OneStepTowards(pModel, 41943,2048, iTime, FrameSpeedMul(pModel, iTime));
  } else {
    //moving forwards. Check auto speed control...
    if (GetBoolParameter(BP_AUTO_SPEEDCONTROL) && m_uSpeedControlTime < iTime) {
        if (m_uSpeedControlTime > 0) //has actually been set?
          SetLongParameter(LP_MAX_BITRATE, GetLongParameter(LP_MAX_BITRATE) * (1.0 + GetLongParameter(LP_DYNAMIC_SPEED_INC)/100.0));
        m_uSpeedControlTime = iTime + 1000*GetLongParameter(LP_DYNAMIC_SPEED_FREQ);
    }
    TimerImpl(iTime, pDasherView, pModel, pol);
  }
}

void CDynamicButtons::KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {

  if(((iId == 0) || (iId == 1) || (iId == 100)) && !GetBoolParameter(BP_BACKOFF_BUTTON))
    return;

  if(m_bKeyDown)
    return;

  // Pass the basic key down event to the handler
  ButtonEvent(iTime, iId, 0, pModel);

  m_iHeldId = iId;
  m_bKeyDown = true;
}

void CDynamicButtons::KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  if (iId == m_iHeldId) m_bKeyDown = false;
}

void CDynamicButtons::ButtonEvent(unsigned long iTime, int iButton, int iType, CDasherModel *pModel) {
  
  // TODO: Check that state diagram implemented here is what we
  // decided upon

  // What happens next depends on the state:
  if (isPaused()) {
    //Any button causes a restart
    if(CUserLogBase *pUserLog=m_pInterface->GetUserLogPtr())
      pUserLog->KeyDown(iButton, iType, 1);
    run(iTime);
  } else if (isReversing()) {
    //Any button pauses
    if(CUserLogBase *pUserLog=m_pInterface->GetUserLogPtr())
      pUserLog->KeyDown(iButton, iType, 2);
    m_pInterface->Done();
    pause();
  } else {
    //running; examine event/button-press type
    switch(iType) {
    case 0: //single press
      if((iButton == 0) || (iButton == 100)) {
        //dedicated pause button
        if(CUserLogBase *pUserLog=m_pInterface->GetUserLogPtr())
          pUserLog->KeyDown(iButton, iType, 2);
        m_pInterface->Done();
        pause();
        break;
      }
      else if(iButton == 1) {
        //dedicated reverse button
        if(CUserLogBase *pUserLog=m_pInterface->GetUserLogPtr())
          pUserLog->KeyDown(iButton, iType, 6);
        reverse(iTime);
        break;
      }
      //else - any non-special button - fall through
    default: //or, Any special kind of event - long, double, triple, ... 
      ActionButton(iTime, iButton, iType, pModel);
    }
  }
}

void CDynamicButtons::pause() {
  CDynamicFilter::pause();
  if (m_pModel) m_pModel->AbortOffset();
}

void CDynamicButtons::reverse(unsigned long iTime) {
  m_bForwards=false;
  if (isPaused()) CDynamicFilter::run(iTime);
  if (GetBoolParameter(BP_AUTO_SPEEDCONTROL)) {
    //treat reversing as a sign of distress --> slow down!
    SetLongParameter(LP_MAX_BITRATE, GetLongParameter(LP_MAX_BITRATE) *
					 (1.0 - GetLongParameter(LP_DYNAMIC_SPEED_DEC)/100.0));
  }
}

void CDynamicButtons::run(unsigned long iTime) {
  m_bForwards=true;
  if (isPaused()) CDynamicFilter::run(iTime); //wasn't running previously
  m_uSpeedControlTime = 0; //will be set in Timer()
}

void CDynamicButtons::ApplyOffset(CDasherModel *pModel, int iOffset) {
  (m_pModel=pModel)->Offset(iOffset);
}
