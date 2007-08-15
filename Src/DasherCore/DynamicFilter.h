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
