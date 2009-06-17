// OneButtonDynamicFilter.h
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

#ifndef __ONE_BUTTON_DYNAMIC_FILTER_H__
#define __ONE_BUTTON_DYNAMIC_FILTER_H__

#include "ButtonMultiPress.h"

/// \ingroup InputFilter
/// @{
class COneButtonDynamicFilter : public CButtonMultiPress {
 public:
  COneButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  ~COneButtonDynamicFilter();

  virtual bool DecorateView(CDasherView *pView);

  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);

 private:
  virtual bool TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted);
  virtual void ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog);
  
  int m_iTarget;

  int *m_iTargetX;
  int *m_iTargetY;
};
/// @}

#endif
