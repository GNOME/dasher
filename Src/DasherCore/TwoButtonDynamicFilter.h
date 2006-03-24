#ifndef __TWO_BUTTON_DYNAMIC_FILTER_H__
#define __TWO_BUTTON_DYNAMIC_FILTER_H__

#include "InputFilter.h"

class CTwoButtonDynamicFilter : public CInputFilter {
 public:
  CTwoButtonDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  ~CTwoButtonDynamicFilter();

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual bool DecorateView(CDasherView *pView);
  virtual void Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void KeyDown(int iTime, int iId, CDasherModel *pModel);
  virtual void KeyUp(int iTime, int iId, CDasherModel *pModel);
  
 private:
  int m_iTarget;

  int *m_iTargetX;
  int *m_iTargetY;

  int m_iStyle;

  int m_iKeyTime;
  bool bStarted;
  bool bBackOff;
};

#endif
