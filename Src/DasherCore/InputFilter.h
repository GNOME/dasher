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
  CInputFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface, long long int iID, int iType, const char *szName)
    : CDasherModule(pEventHandler, pSettingsStore, iID, iType, szName) {
    m_pInterface = pInterface;
  };

  virtual void HandleEvent(Dasher::CEvent * pEvent) {};
  virtual bool DecorateView(CDasherView *pView) { return false; };
  virtual void KeyDown(int Time, int iId, CDasherModel *pModel) {};
  virtual void KeyUp(int Time, int iId, CDasherModel *pModel) {};
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) { return false; };

  virtual void Activate() {};
  virtual void Deactivate() {};
  
 protected:
  CDasherInterfaceBase *m_pInterface;
};

#endif
