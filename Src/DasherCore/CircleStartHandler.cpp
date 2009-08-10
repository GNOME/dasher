// CircleStartHandler.cpp
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

#include "CircleStartHandler.h"
#include "Event.h"

using namespace Dasher;

CCircleStartHandler::CCircleStartHandler(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface) 
  : CStartHandler(pEventHandler, pSettingsStore, pInterface) {
  m_iStatus = -1;
  m_iChangeTime = 0;
  m_iCircleRadius = GetLongParameter(LP_MAX_Y) * GetLongParameter(LP_CIRCLE_PERCENT) / 100;
  m_iScreenRadius = 0;
}

bool CCircleStartHandler::DecorateView(CDasherView *pView) {
  screenint iCX;
  screenint iCY;

  pView->Dasher2Screen(2048, 2048, iCX, iCY);

  screenint iCX2;
  screenint iCY2;
 
  pView->Dasher2Screen(2048, 2048 + m_iCircleRadius, iCX2, iCY2);

  int iDirection = GetLongParameter(LP_REAL_ORIENTATION);

  if((iDirection == 2) || (iDirection == 3)) {
    m_iScreenRadius = iCX2 - iCX;
  }
  else {
    m_iScreenRadius = iCY2 - iCY;
  }

  if((m_iStatus == 0) || (m_iStatus == 2))
    pView->Screen()->DrawCircle(iCX, iCY, m_iScreenRadius, 2, 242, 1, true);
  else if((m_iStatus == 1) || (m_iStatus == 3))
    pView->Screen()->DrawCircle(iCX, iCY, m_iScreenRadius, 240, 0, 1, false);
  else if(m_iStatus == 5)
    pView->Screen()->DrawCircle(iCX, iCY, m_iScreenRadius, 2, 241, 1, true);
  else
    pView->Screen()->DrawCircle(iCX, iCY, m_iScreenRadius, 240, 0, 3, false);

  return true;
}

void CCircleStartHandler::Timer(int iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {
  myint iDasherX;
  myint iDasherY;
  m_pDasherView->GetCoordinates(iDasherX, iDasherY);

  screenint iCX;
  screenint iCY;
  m_pDasherView->Dasher2Screen(2048, 2048, iCX, iCY);
  
  screenint iCursorX;
  screenint iCursorY;
  m_pDasherView->Dasher2Screen(iDasherX, iDasherY, iCursorX, iCursorY);

  double dR;

  dR = sqrt(pow(static_cast<double>(iCX - iCursorX), 2.0) + pow(static_cast<double>(iCY - iCursorY), 2.0));

  int iNewStatus(-1);

  // Status flags:
  // -1 undefined
  // 0 = out of circle, stopped
  // 1 = out of circle, started
  // 2 = in circle, stopped
  // 3 = in circle, started
  // 4 = in circle, stopping
  // 5 = in circle, starting

  // TODO - need to check that these respond correctly to (eg) external pauses

  if(dR < m_iScreenRadius) {
    switch(m_iStatus) {
    case -1:
      if(m_pInterface->GetBoolParameter(BP_DASHER_PAUSED))
	iNewStatus = 2;
      else
	iNewStatus = 3;
      break;
    case 0:
      iNewStatus = 5;
      break;
    case 1:
      iNewStatus = 4;
      break;
    case 2:
    case 3:
    case 4:
    case 5:
      iNewStatus = m_iStatus;
      break;
    }
  }
  else {
    switch(m_iStatus) {
    case -1:
      if(m_pInterface->GetBoolParameter(BP_DASHER_PAUSED))
	iNewStatus = 0;
      else
	iNewStatus = 1;
      break;
    case 0:
    case 1:
      iNewStatus = m_iStatus;
      break;
    case 2:
      iNewStatus = 0;
      break;
    case 3:
      iNewStatus = 1;
      break;
    case 4:
      iNewStatus = 1;
      break;
    case 5:
      iNewStatus = 0;
      break;
    }
  }

  if(iNewStatus != m_iStatus) {
    m_iChangeTime = iTime;
  }

  if(iTime - m_iChangeTime > 1000) {
    if(iNewStatus == 4) {
      iNewStatus = 2;
      m_pInterface->PauseAt(0, 0);
    } 
    else if(iNewStatus == 5) {
      iNewStatus = 3;
      m_pInterface->Unpause(iTime);
    }
  }

  m_iStatus = iNewStatus;

}

void CCircleStartHandler::HandleEvent(Dasher::CEvent * pEvent) {
  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
   
    switch (pEvt->m_iParameter) {
    case BP_DASHER_PAUSED:
      m_iStatus = -1;
      break;
    }
  }
}
