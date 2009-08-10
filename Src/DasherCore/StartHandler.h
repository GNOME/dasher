#ifndef __START_HANDLER_H__
#define __START_HANDLER_H__

#include "DasherInterfaceBase.h"
#include "DasherComponent.h"
namespace Dasher {
/// \defgroup Start Start handlers
/// @{
class CStartHandler : public CDasherComponent {
public:
  CStartHandler(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface) 
    : CDasherComponent(pEventHandler, pSettingsStore) {
    m_pInterface = pInterface;
  };

  virtual bool DecorateView(CDasherView *pView) = 0;
  virtual void Timer(int iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel) = 0;
  virtual void HandleEvent(Dasher::CEvent * pEvent) = 0;

protected:
  CDasherInterfaceBase *m_pInterface;
};
}
/// @}

#endif
