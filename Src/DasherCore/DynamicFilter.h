#ifndef __DYNAMIC_FILTER_H__
#define __DYNAMIC_FILTER_H__

#include "InputFilter.h"

class CDynamicFilter : public CInputFilter {
 public:
  CDynamicFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore);
  ~CDynamicFilter();

  virtual void Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void KeyDown(int iId, CDasherModel *pModel);
  
 private:
  int m_iTarget;

  int *m_iTargetX;
  int *m_iTargetY;
};

#endif
