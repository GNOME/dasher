// TwoButtonDynamicFilter.cpp
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

#include "TwoButtonDynamicFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

static SModuleSettings sSettings[] = {
  {LP_TWO_BUTTON_OFFSET, T_LONG, 1024, 2048, 2048, 100, _("Button offset")},
  /* TRANSLATORS: The time for which a button must be held before it counts as a 'long' (rather than short) press. */
  {LP_HOLD_TIME, T_LONG, 100, 10000, 1000, 100, _("Long press time")},
  /* TRANSLATORS: Multiple button presses are special (like a generalisation on double clicks) in some situations. This is the time in which the button must be pressed multiple times to count.*/
  {LP_MULTIPRESS_TIME, T_LONG, 100, 10000, 1000, 100, _("Multiple press time")},
  /* TRANSLATORS: Multiple button presses are special (like a generalisation on double clicks) in some situations. This is the number of times a button must be pressed to count as a multiple press.*/
  {LP_MULTIPRESS_COUNT,T_LONG, 2, 10, 1, 1, _("Multiple press count")}, 
  /* TRANSLATORS: Backoff = reversing in Dasher to correct mistakes. This allows a single button to be dedicated to activating backoff, rather than using multiple presses of other buttons, and another to be dedicated to starting and stopping. 'Button' in this context is a physical hardware device, not a UI element.*/
  {BP_BACKOFF_BUTTON,T_BOOL, -1, -1, -1, -1, _("Enable backoff and start/stop buttons")},
  /* TRANSLATORS: What is normally the up button becomes the down button etc. */
  {BP_TWOBUTTON_REVERSE,T_BOOL, -1, -1, -1, -1, _("Reverse up and down buttons")},
  {BP_SLOW_START,T_BOOL, -1, -1, -1, -1, _("Slow startup")},
  {LP_SLOW_START_TIME, T_LONG, 0, 10000, 1000, 100, _("Startup time")},
  {BP_TWOBUTTON_SPEED,T_BOOL, -1, -1, -1, -1, _("Auto speed control")},
  /* TRANSLATORS: The threshold time above which auto speed control is used. */
  {LP_DYNAMIC_MEDIAN_FACTOR, T_LONG, 10, 200, 100, 10, _("Auto speed threshold")}
};

CTwoButtonDynamicFilter::CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface)
  : CDynamicFilter(pEventHandler, pSettingsStore, pInterface, 14, 1, _("Two Button Dynamic Mode")) { 

  m_iLastTime = -1;

  m_pTree = new SBTree(2000);
  m_pTree->Add(2000);
  m_pTree->Add(2000);
}

CTwoButtonDynamicFilter::~CTwoButtonDynamicFilter() {
  if(m_pTree)
    delete m_pTree;
}

bool CTwoButtonDynamicFilter::DecorateView(CDasherView *pView) {
  CDasherScreen *pScreen(pView->Screen());

  CDasherScreen::point p[2];
  
  myint iDasherX;
  myint iDasherY;
  
  iDasherX = -100;
  iDasherY = 2048 - GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = 2048 - GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  pScreen->Polyline(p, 2, 3, 242);

  iDasherX = -100;
  iDasherY = 2048 + GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = 2048 + GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  pScreen->Polyline(p, 2, 3, 242);

  bool bRV(m_bDecorationChanged);
  m_bDecorationChanged = false;
  return bRV;
}

bool CTwoButtonDynamicFilter::TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) {
  return m_pDasherModel->OneStepTowards(100,2048, Time, pAdded, pNumDeleted);
}

void CTwoButtonDynamicFilter::Activate() {
  SetBoolParameter(BP_DELAY_VIEW, true);
}

void CTwoButtonDynamicFilter::Deactivate() {
  SetBoolParameter(BP_DELAY_VIEW, false);
}

void CTwoButtonDynamicFilter::ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLogBase *pUserLog) {
  int iFactor(1);

  if(GetBoolParameter(BP_TWOBUTTON_REVERSE))
    iFactor = -1;

  if(iButton == 2) {
    pModel->Offset(iFactor * GetLongParameter(LP_TWO_BUTTON_OFFSET));
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 3);
    if(GetBoolParameter(BP_TWOBUTTON_SPEED))
      AutoSpeedSample(iTime, pModel);
  }
  else if((iButton == 3) || (iButton == 4)) {
    pModel->Offset(iFactor * -GetLongParameter(LP_TWO_BUTTON_OFFSET));
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 4);
    if(GetBoolParameter(BP_TWOBUTTON_SPEED))
      AutoSpeedSample(iTime, pModel);
  }
  else {
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 0);
  }
}

bool CTwoButtonDynamicFilter::GetSettings(SModuleSettings **pSettings, int *iCount) {
  *pSettings = sSettings;
  *iCount = sizeof(sSettings) / sizeof(SModuleSettings);

  return true;
};

void CTwoButtonDynamicFilter::AutoSpeedSample(int iTime, CDasherModel *pModel) {
  if(m_iLastTime == -1) {
    m_iLastTime = iTime;
    return;
  }

  if(pModel->IsSlowdown(iTime))
    return;

  int iDiff(iTime - m_iLastTime);
  m_iLastTime = iTime;

  if(m_pTree) {
    int iMedian(m_pTree->GetOffset(m_pTree->GetCount() / 2));
    
    if((iDiff <= 300) || (iDiff < (iMedian * GetLongParameter(LP_DYNAMIC_MEDIAN_FACTOR)) / 100)) {
      pModel->TriggerSlowdown();
    }
  }
  
  if(!m_pTree)
    m_pTree = new SBTree(iDiff);
  else
    m_pTree->Add(iDiff);

  m_deOffsetQueue.push_back(iDiff);

  while(m_deOffsetQueue.size() > 10) {
    m_pTree = m_pTree->Delete(m_deOffsetQueue.front());
    m_deOffsetQueue.pop_front();
  }
}

void CTwoButtonDynamicFilter::AutoSpeedUndo(int iCount) {
  for(int i(0); i < iCount; ++i) {
    if(m_deOffsetQueue.size() == 0)
      return;

    if(m_pTree)
      m_pTree = m_pTree->Delete(m_deOffsetQueue.back());
    m_deOffsetQueue.pop_back();
  }
}

CTwoButtonDynamicFilter::SBTree::SBTree(int iValue) {
  m_iValue = iValue;
  m_pLeft = NULL;
  m_pRight = NULL;
  m_iCount = 1;
}

CTwoButtonDynamicFilter::SBTree::~SBTree() {
  if(m_pLeft)
    delete m_pLeft;

  if(m_pRight)
    delete m_pRight;
}

void CTwoButtonDynamicFilter::SBTree::Add(int iValue) {
  ++m_iCount;

  if(iValue > m_iValue) {
    if(m_pRight)
      m_pRight->Add(iValue);
    else
      m_pRight = new SBTree(iValue);
  }
  else {
    if(m_pLeft)
      m_pLeft->Add(iValue);
    else
      m_pLeft = new SBTree(iValue);
  }
}

CTwoButtonDynamicFilter::SBTree* CTwoButtonDynamicFilter::SBTree::Delete(int iValue) {
  // Hmm... deleting is awkward in binary trees

  if(iValue == m_iValue) {
    if(!m_pLeft) {
      SBTree *pOldRight = m_pRight;
      m_pRight = NULL;
      delete this;
      return pOldRight;
    }
    else {
      SBTree *pOldLeft = m_pLeft;
      pOldLeft->SetRightMost(m_pRight);
      m_pLeft = NULL;
      m_pRight = NULL;
      delete this;
      return pOldLeft;
    }
  }
  else if(iValue > m_iValue) {
    --m_iCount;
    m_pRight = m_pRight->Delete(iValue);
  }
  else {
    --m_iCount;
    m_pLeft = m_pLeft->Delete(iValue);
  }

  return this;
}

void CTwoButtonDynamicFilter::SBTree::SetRightMost(CTwoButtonDynamicFilter::SBTree* pNewTree) {
  if(pNewTree)
    m_iCount += pNewTree->GetCount();

  if(m_pRight)
    m_pRight->SetRightMost(pNewTree);
  else
    m_pRight = pNewTree;
}

int CTwoButtonDynamicFilter::SBTree::GetOffset(int iOffset) {
  if(m_pLeft && (m_pLeft->GetCount() > iOffset))
    return m_pLeft->GetOffset(iOffset);
  else if((m_pLeft && (m_pLeft->GetCount() == iOffset)) || (!m_pLeft && (iOffset == 0)))
    return m_iValue;
  else if(m_pLeft)
    return m_pRight->GetOffset(iOffset - m_pLeft->GetCount() - 1);
  else
    return m_pRight->GetOffset(iOffset - 1);
}

void CTwoButtonDynamicFilter::RevertPresses(int iCount) {
  if(GetBoolParameter(BP_TWOBUTTON_SPEED))
    AutoSpeedUndo(iCount);
}
