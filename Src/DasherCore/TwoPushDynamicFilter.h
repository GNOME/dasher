// TwoPushDynamicFilter.h
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

#ifndef __TWO_PUSH_DYNAMIC_FILTER_H__
#define __TWO_PUSH_DYNAMIC_FILTER_H__

#include "DynamicFilter.h"
namespace Dasher {
/// \ingroup InputFilter
/// @{
class CTwoPushDynamicFilter : public CDynamicFilter /*long push, but do our own "multi-push" detection*/ {
 public:
  CTwoPushDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  
  // Inherited methods
  virtual bool DecorateView(CDasherView *pView);
 
  virtual void Activate();
  virtual void Deactivate();
  virtual bool GetMinWidth(int &iMinWidth);
  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);

  //override to get mouse clicks / taps back again...
  virtual void KeyDown(int Time, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY);
  virtual void KeyUp(int Time, int iId, CDasherView *pDasherView, CDasherModel *pModel, bool bPos, int iX, int iY);

 protected:
  virtual bool TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
  virtual void ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog);

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual void run();
  virtual void pause();
  virtual void reverse();

 private:
  double m_dLogUpMul, m_dLogDownMul, m_dLagBits;
  double m_dMinShortTwoPushTime, m_dMaxShortTwoPushTime,
    m_dMinLongTwoPushTime, m_dMaxLongTwoPushTime;
  int m_aiMarker[2];
  int m_iActiveMarker;
  int m_aiTarget[2];
  int m_aaiGuideAreas[2][2];
  
  double m_dNatsSinceFirstPush;
};
}
/// @}

#endif
