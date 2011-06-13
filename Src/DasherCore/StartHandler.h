#ifndef __START_HANDLER_H__
#define __START_HANDLER_H__

#include "DasherInterfaceBase.h"

namespace Dasher {
/// \defgroup Start Start handlers
/// @{
class CStartHandler {
public:
  CStartHandler(CDasherInterfaceBase *pInterface) : m_pInterface(pInterface) {
  };
  virtual ~CStartHandler() {
  }
  
  virtual bool DecorateView(CDasherView *pView) = 0;
  virtual void Timer(int iTime, dasherint iX, dasherint iY, CDasherView *pView) = 0;

protected:
  CDasherInterfaceBase *m_pInterface;
};
}
/// @}

#endif
