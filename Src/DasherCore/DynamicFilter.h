#ifndef __DYNAMIC_FILTER_H__
#define __DYNAMIC_FILTER_H__

#include "TwoButtonDynamicFilter.h"

class CDynamicFilter : public CTwoButtonDynamicFilter {
 public:
  CDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface);
  ~CDynamicFilter();

  virtual bool DecorateView(CDasherView *pView);

 protected:
  virtual bool TimerImpl(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void ActionButton(int iTime, int iButton, CDasherModel *pModel);
  
 private:
  int m_iTarget;

  int *m_iTargetX;
  int *m_iTargetY;
};

#endif
