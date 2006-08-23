#include "../Common/Common.h"

#include "TwoButtonDynamicFilter.h"
#include "DasherInterfaceBase.h"
#include "Event.h"

#include <iostream>

// TODO: Move a lot of this stuff into a base class, so that the
// single and double button dynamic modes can behave in essentially
// the same way.


static SModuleSettings sSettings[] = {
  {LP_TWO_BUTTON_OFFSET, T_LONG, 1024, 2048, 2048, 100, "Button offset"},
  {LP_HOLD_TIME, T_LONG, 100, 10000, 1000, 100, "Long press time"},
  {LP_MULTIPRESS_TIME, T_LONG, 100, 10000, 1000, 100, "Multiple press time"},
  {LP_MULTIPRESS_COUNT,T_LONG, 2, 10, 1, 1, "Multiple press count"},
  {BP_BACKOFF_BUTTON,T_BOOL, -1, -1, -1, -1, "Enable backoff and start/stop buttons"},
  {BP_TWOBUTTON_REVERSE,T_BOOL, -1, -1, -1, -1, "Reverse up and down buttons"},
  {BP_SLOW_START,T_BOOL, -1, -1, -1, -1, "Slow startup"},
  {LP_SLOW_START_TIME, T_LONG, 0, 10000, 1000, 100, "Startup time"}
};

CTwoButtonDynamicFilter::CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, long long int iID, int iType, const char *szName)
  : CInputFilter(pEventHandler, pSettingsStore, pInterface, iID, iType, szName) { 
  //14, 1, "Two Button Dynamic Mode") {
  m_iState = 0;
  m_iLastTime = -1;
  m_bDecorationChanged = true;
  m_bKeyDown = false;

  m_pLowerTree = NULL;
  m_pUpperTree = NULL;

  m_iLowerCount = 0;
  m_iUpperCount = 0;
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
  
  pScreen->Polyline(p, 2, 1, 2);

  iDasherX = -100;
  iDasherY = 2048 + GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[0].x, p[0].y);
  
  iDasherX = -1000;
  iDasherY = 2048 + GetLongParameter(LP_TWO_BUTTON_OFFSET);
  
  pView->Dasher2Screen(iDasherX, iDasherY, p[1].x, p[1].y);
  
  pScreen->Polyline(p, 2, 1, 2);

  bool bRV(m_bDecorationChanged);
  m_bDecorationChanged = false;
  return bRV;
}

bool CTwoButtonDynamicFilter::Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) {
  if(m_bKeyDown && !m_bKeyHandled && ((Time - m_iKeyDownTime) > GetLongParameter(LP_HOLD_TIME))) {
    Event(Time, m_iHeldId, 1, m_pDasherModel, m_pUserLog);
    m_bKeyHandled = true;
    return true;
  }

  if(m_iState == 2)
    return m_pDasherModel->Tap_on_display(41943,2048, Time, pAdded, pNumDeleted);
  else if(m_iState == 1)
    return TimerImpl(Time, m_pDasherView, m_pDasherModel, pAdded, pNumDeleted);
  else
    return false;
}

bool CTwoButtonDynamicFilter::TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) {
  return m_pDasherModel->Tap_on_display(100,2048, Time, pAdded, pNumDeleted);
}

void CTwoButtonDynamicFilter::KeyDown(int iTime, int iId, CDasherModel *pModel, CUserLog *pUserLog) {

  m_pUserLog = pUserLog;

  if(((iId == 0) || (iId == 1) || (iId == 100)) && !GetBoolParameter(BP_BACKOFF_BUTTON))
    return;

  if(m_bKeyDown)
    return;

  // Pass the basic key down event to the handler
  Event(iTime, iId, 0, pModel, pUserLog);
  
  // Store the key down time so that long presses can be determined
  // TODO: This is going to cause problems if multiple buttons are
  // held down at once
  m_iKeyDownTime = iTime;
  
  // Check for multiple clicks
  if(iId == m_iQueueId) {
    while((m_deQueueTimes.size() > 0) && (iTime - m_deQueueTimes.front()) > GetLongParameter(LP_HOLD_TIME))
      m_deQueueTimes.pop_front();

    if(m_deQueueTimes.size() == GetLongParameter(LP_MULTIPRESS_COUNT) - 1) { 
      Event(iTime, iId, 2, pModel, pUserLog);
      m_deQueueTimes.clear();
    }
    else
      m_deQueueTimes.push_back(iTime);
  }
  else {
    m_deQueueTimes.clear();
    m_deQueueTimes.push_back(iTime);
    m_iQueueId = iId;
  }

  m_iHeldId = iId;
  m_bKeyDown = true;
  m_bKeyHandled = false;
}

void CTwoButtonDynamicFilter::KeyUp(int iTime, int iId, CDasherModel *pModel) {
  m_bKeyDown = false;
}

void CTwoButtonDynamicFilter::Activate() {
  SetBoolParameter(BP_DELAY_VIEW, true);
}

void CTwoButtonDynamicFilter::Deactivate() {
  SetBoolParameter(BP_DELAY_VIEW, false);
}

void CTwoButtonDynamicFilter::Event(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLog *pUserLog) {
  // Types:
  // 0 = ordinary click
  // 1 = long click
  // 2 = multiple click

  
  // First sanity check - if Dasher is paused then jump to the
  // appropriate state
  if(GetBoolParameter(BP_DASHER_PAUSED))
    m_iState = 0;

  // TODO: Check that state diagram implemented here is what we
  // decided upon

  // What happens next depends on the state:
  switch(m_iState) {
  case 0: // Any button when paused causes a restart
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 1);
    m_pInterface->Unpause(iTime);
    SetBoolParameter(BP_DELAY_VIEW, true);
    m_iState = 1;
    break;
  case 1:
    switch(iType) {
    case 0:
      if((iButton == 0) || (iButton == 100)) {
	if(pUserLog)
	  pUserLog->KeyDown(iButton, iType, 2);
	m_iState = 0;
	SetBoolParameter(BP_DELAY_VIEW, false);
	m_pInterface->PauseAt(0, 0);
      }
      else if(iButton == 1) {
	if(pUserLog)
	  pUserLog->KeyDown(iButton, iType, 6);
	SetBoolParameter(BP_DELAY_VIEW, false);
	m_iState = 2;
      }
      else {
	ActionButton(iTime, iButton, iType, pModel, pUserLog);
      }
      break;
    case 1: // Delibarate fallthrough
    case 2: 
      if((iButton >= 2) && (iButton <= 4)) {
	if(pUserLog)
	  pUserLog->KeyDown(iButton, iType, 6);
	SetBoolParameter(BP_DELAY_VIEW, false);
	m_iState = 2;
       }
      else {
	if(pUserLog)
	  pUserLog->KeyDown(iButton, iType, 0);
      }
      break;
    }
    break;
  case 2:
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 2);
    
    m_iState = 0;
    m_pInterface->PauseAt(0, 0);
    break;
  }

  m_iLastButton = iButton;
}

void CTwoButtonDynamicFilter::ActionButton(int iTime, int iButton, int iType, CDasherModel *pModel, CUserLog *pUserLog) {
  int iFactor(1);

  if(GetBoolParameter(BP_TWOBUTTON_REVERSE))
    iFactor = -1;

  if(iButton == 2) {
    pModel->Offset(iFactor * GetLongParameter(LP_TWO_BUTTON_OFFSET));
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 3);
  }
  else if((iButton == 3) || (iButton == 4)) {
    pModel->Offset(iFactor * -GetLongParameter(LP_TWO_BUTTON_OFFSET));
    if(pUserLog)
      pUserLog->KeyDown(iButton, iType, 4);
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

bool CTwoButtonDynamicFilter::GetMinWidth(int &iMinWidth) {
  iMinWidth = 1024;
  return true;
}

void CTwoButtonDynamicFilter::AutoSpeedSample(int iTime) {
  if(m_iLastTime == -1) {
    m_iLastTime = iTime;
    return;
  }
    
  int iDiff(iTime - m_iLastTime);

  if(!m_pUpperTree) { // Special case: intial data point
    m_pUpperTree = new SBTree(iDiff);
    m_iUpperMin = iDiff;
    m_iUpperCount = 1;
  }
  else {
    if(iDiff >= m_iUpperMin) {
      m_pUpperTree->Add(iDiff);
      ++m_iUpperCount;
    }
    else {
      if(!m_pLowerTree) {
	m_pLowerTree = new SBTree(iDiff);
	m_iLowerMax = iDiff;
	m_iLowerCount = 1;
      }
      else {
	m_pLowerTree->Add(iDiff);
	++m_iLowerCount;
	if(iDiff > m_iLowerMax)
	  m_iLowerMax = iDiff;
      }
    }
  }

  // Renormalise the trees - assume that we only need to make one change here

  if(m_iLowerCount < m_iUpperCount / 3) {
    if(m_pLowerTree)
      m_pLowerTree->Add(m_iUpperMin);
    else
      m_pLowerTree = new SBTree(m_iUpperMin);

    m_pUpperTree->Delete(m_iUpperMin);

    --m_iUpperCount;
    ++m_iLowerCount;

    m_iLowerMax = m_iUpperMin;
    m_iUpperMin = m_pUpperTree->GetMin();
  }
  else if(m_iLowerCount > m_iUpperCount / 3) {
    if(m_iLowerCount > 1)
      m_pLowerTree->Delete(m_iLowerMax);
    else {
      delete m_pLowerTree;
      m_pLowerTree = NULL;
    }
    
    if(m_pUpperTree)
      m_pUpperTree->Add(m_iLowerMax);
    else
      m_pUpperTree = new SBTree(m_iLowerMax);

    --m_iLowerCount;
    ++m_iUpperCount;

    m_iUpperMin = m_iLowerMax;
    if(m_pLowerTree)
      m_iLowerMax = m_pLowerTree->GetMax();
  }
}

CTwoButtonDynamicFilter::SBTree::SBTree(int iValue) {
  m_iValue = iValue;
  m_pLeft = NULL;
  m_pRight = NULL;
}

void CTwoButtonDynamicFilter::SBTree::Add(int iValue) {
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
    // Delete me
  }
  else if(iValue > m_iValue) {
    m_pRight = m_pRight->Delete(iValue);
  }
  else {
    m_pLeft = m_pLeft->Delete(iValue);
  }

  return this;
}

int CTwoButtonDynamicFilter::SBTree::GetMax() {
  if(m_pRight)
    return m_pRight->GetMax();
  else
    return m_iValue;
}

int CTwoButtonDynamicFilter::SBTree::GetMin() {
  if(m_pLeft)
    return m_pLeft->GetMax();
  else
    return m_iValue;
}
