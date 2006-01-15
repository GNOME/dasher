#ifndef __INPUT_FILTER_H__
#define __INPUT_FILTER_H__

#include "DasherView.h"
#include "DasherModel.h"

#include "DasherComponent.h"

class CInputFilter : public CDasherComponent {
 public:
  CInputFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore)
    : CDasherComponent(pEventHandler, pSettingsStore) {};

  virtual void HandleEvent(Dasher::CEvent * pEvent) {};
  virtual void DecorateView(CDasherView *pView) {};
  virtual void KeyDown(int iId, CDasherModel *pModel) {};
  virtual void Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {};
};

#endif
