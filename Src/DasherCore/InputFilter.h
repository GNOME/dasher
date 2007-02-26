#ifndef __INPUT_FILTER_H__
#define __INPUT_FILTER_H__

#include "DasherView.h"
#include "DasherModel.h"
#include "DasherModule.h"
#include "UserLogBase.h"

namespace Dasher {
  class CDasherInterfaceBase;
}

/// \defgroup InputFilter Input filters
/// @{
class CInputFilter : public CDasherModule {
 public:
  CInputFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, int iType, const char *szName)
    : CDasherModule(pEventHandler, pSettingsStore, iID, iType, szName) {
    m_pInterface = pInterface;
  };

  virtual void HandleEvent(Dasher::CEvent * pEvent) {};
  virtual bool DecorateView(CDasherView *pView) { return false; };
  virtual void KeyDown(int Time, int iId, CDasherModel *pModel, CUserLogBase *pUserLog) {};
  virtual void KeyUp(int Time, int iId, CDasherModel *pModel) {};
  virtual bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted) { return false; };

  virtual void Activate() {};
  virtual void Deactivate() {};

  virtual bool GetMinWidth(int &iMinWidth) {
    return false;
  }
  
 protected:
  CDasherInterfaceBase *m_pInterface;
};
/// @}

#endif
