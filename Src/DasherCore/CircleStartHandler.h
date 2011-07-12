#ifndef __CIRCLE_START_HANDLER_H__
#define __CIRCLE_START_HANDLER_H__

#include "StartHandler.h"
#include "SettingsStore.h"
#include "DasherScreen.h"

/// \ingroup Start
/// @{
namespace Dasher {
class CCircleStartHandler : public CStartHandler, public CSettingsUserObserver, public Observer<CDasherView *> {
public:
  CCircleStartHandler(CDefaultFilter *pCreator);
  ~CCircleStartHandler();
  virtual bool DecorateView(CDasherView *pView);
  virtual void Timer(int iTime, dasherint iX, dasherint iY, CDasherView *pView);
  virtual void HandleEvent(int iParameter);
  virtual void HandleEvent(CDasherView *pView);

protected:
  ///Time (as unix timestamp) when user entered circle; max() => already acted upon
  long m_iEnterTime;
  ///Whether the user was in the circle in last call to Timer
  bool m_bInCircle;
  ///Radius of circle in screen coordinates (-1 = needs recomputing)
  int m_iScreenRadius;
  ///Center of screen circle (needs recomputing if radius does)
  CDasherScreen::point m_screenCircleCenter;
  CDasherView *m_pView;
  virtual void ComputeScreenLoc(CDasherView *pView);
};
}
/// @}

#endif
