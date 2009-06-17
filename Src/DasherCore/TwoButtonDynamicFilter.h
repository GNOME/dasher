// TwoButtonDynamicFilter.h
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

#ifndef __TWO_BUTTON_DYNAMIC_FILTER_H__
#define __TWO_BUTTON_DYNAMIC_FILTER_H__

#include "ButtonMultiPress.h"

#include <deque>

/// \ingroup InputFilter
/// @{
class CTwoButtonDynamicFilter : public CButtonMultiPress {
 public:
  CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  ~CTwoButtonDynamicFilter();

  // Inherited methods
  virtual bool DecorateView(CDasherView *pView);
 
  virtual void Activate();
  virtual void Deactivate();

  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);

  virtual bool GetMinWidth(int &iMinWidth);
  
 private:
  virtual bool TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted);
  virtual void ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog);

  virtual void RevertPresses(int iCount);

  void AutoSpeedSample(int iTime, CDasherModel *pModel);
  void AutoSpeedUndo(int iCount);

  class SBTree {
  public:
    SBTree(int iValue);
    ~SBTree();

    void Add(int iValue);
    SBTree* Delete(int iValue);

    int GetCount() {
      return m_iCount;
    };

    int GetOffset(int iOffset);

    void SetRightMost(SBTree* pNewTree);

  private:
    int m_iValue;
    SBTree *m_pLeft;
    SBTree *m_pRight;
    int m_iCount;
  };

  int m_iLastTime;
  SBTree *m_pTree;
  std::deque<int> m_deOffsetQueue;
};
/// @}

#endif
