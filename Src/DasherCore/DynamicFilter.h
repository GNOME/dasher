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
class CDynamicFilter : public CInputFilter {
 public:
  CDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, int iType, const char *szName);
  
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted); 

  virtual bool GetMinWidth(int &iMinWidth);

  virtual void KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog);
  virtual void KeyUp(int iTime, int iId, CDasherView *pView, CDasherModel *pModel);

 protected:
  bool m_bDecorationChanged;

 private:
  virtual bool TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) = 0;
  virtual void ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog) = 0;
  void Event(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog);
  virtual void RevertPresses(int iCount) {};

  bool m_bKeyDown;
  bool m_bKeyHandled;
  int m_iHeldId;
  int m_iLastButton;
  int m_iKeyDownTime;
  int m_iState; // 0 = paused, 1 = running 2 = backing off
  int m_iQueueId;
  std::deque<int> m_deQueueTimes;
 
  CUserLogBase *m_pUserLog;
};

#endif
