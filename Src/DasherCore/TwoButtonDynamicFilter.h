#ifndef __TWO_BUTTON_DYNAMIC_FILTER_H__
#define __TWO_BUTTON_DYNAMIC_FILTER_H__

#include "InputFilter.h"

#include <deque>

class CTwoButtonDynamicFilter : public CInputFilter {
 public:
  CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);

  // Inherited methods
  virtual bool DecorateView(CDasherView *pView);
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel);
  virtual void KeyUp(int iTime, int iId, CDasherModel *pModel);
  virtual void Activate();
  virtual void Deactivate();

 private:
  void Event(int iTime, int iButton, int iType, CDasherModel *pModel);

  bool m_bDecorationChanged;
  int m_iLastButton;
  int m_iKeyDownTime;
  int m_iState; // 0 = paused, 1 = running 2 = backing off
  int m_iQueueId;
  std::deque<int> m_deQueueTimes;

  bool m_bKeyDown;
};

#endif
