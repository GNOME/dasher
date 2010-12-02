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
  virtual void Timer(int iTime, dasherint iX, dasherint iY, CDasherView *pView) = 0;

protected:
  CDasherInterfaceBase *m_pInterface;
};
}
/// @}

#endif
