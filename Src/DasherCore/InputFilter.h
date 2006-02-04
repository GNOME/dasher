#ifndef __INPUT_FILTER_H__
#define __INPUT_FILTER_H__

#include "DasherView.h"
#include "DasherModel.h"

#include "DasherModule.h"

namespace Dasher {
  class CDasherInterfaceBase;
}

class CInputFilter : public CDasherModule {
 public:
  CInputFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface, long long int iID, int iType)
    : CDasherModule(pEventHandler, pSettingsStore, iID, iType) {
    m_pInterface = pInterface;
  };

  virtual void HandleEvent(Dasher::CEvent * pEvent) {};
  virtual void DecorateView(CDasherView *pView) {};
  virtual void KeyDown(int Time, int iId, CDasherModel *pModel) {};
  virtual void KeyUp(int Time, int iId, CDasherModel *pModel) {};
  virtual void Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) {};
  
 protected:
  CDasherInterfaceBase *m_pInterface;
};

#endif
