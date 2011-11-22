#ifndef __START_HANDLER_H__
#define __START_HANDLER_H__

#include "DasherTypes.h"

namespace Dasher {
  class CDefaultFilter;
  class CDasherView;
/// \defgroup Start Start handlers
/// @{
class CStartHandler {
public:
  CStartHandler(CDefaultFilter *pFilter) : m_pFilter(pFilter) {
  };
  virtual ~CStartHandler() {
  }
  
  virtual bool DecorateView(CDasherView *pView) = 0;
  virtual void Timer(unsigned long iTime, dasherint iX, dasherint iY, CDasherView *pView) = 0;
  virtual void onRun(unsigned long iTime) {}
  virtual void onPause() {}
protected:
  CDefaultFilter * const m_pFilter;
};
}
/// @}

#endif
