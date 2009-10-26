// DynamicFilter.h
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

#ifndef __DynamicFilter_h__
#define __DynamicFilter_h__

#include "InputFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
///filter with three states: paused, reversing, running. Hold any button down to reverse.
class CDynamicFilter : public CInputFilter {
 public:
  CDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, int iType, const char *szName);
  
  ///when reversing, backs off; when paused, does nothing; when running, delegates to TimerImpl
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol); 

  virtual void KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog);
  virtual void KeyUp(int iTime, int iId, CDasherView *pView, CDasherModel *pModel);


 protected:
  virtual void ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog) = 0;
  virtual void Event(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog);

  bool m_bKeyDown;
  bool m_bKeyHandled;
  bool m_bDecorationChanged;
  bool isPaused() {return m_iState == 0;}
  bool isReversing() {return m_iState == 1;}
  bool isRunning() {return m_iState==2;}
  virtual void pause() {m_iState = 0;}
  virtual void reverse();
  virtual void run();

  virtual bool TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol) = 0;

  private:
    int m_iState; // 0 = paused, 1 = reversing, >=2 = running (extensible by subclasses)
    int m_iHeldId;
    int m_iKeyDownTime;
    unsigned int m_uSpeedControlTime;
	
    CUserLogBase *m_pUserLog;
};
}
#endif
