#ifndef __CIRCLE_START_HANDLER_H__
#define __CIRCLE_START_HANDLER_H__

#include "StartHandler.h"
/// \ingroup Start
/// @{
namespace Dasher {
class CCircleStartHandler : public CStartHandler {
public:
  CCircleStartHandler(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface);

  virtual bool DecorateView(CDasherView *pView);
  virtual void Timer(int iTime, dasherint iX, dasherint iY, CDasherView *pView);
  virtual void HandleEvent(Dasher::CEvent * pEvent);

protected:
  ///Time (as unix timestamp) when user entered circle; max() => already acted upon
  long m_iEnterTime;
  ///Whether the user was in the circle in last call to Timer
  bool m_bInCircle;
  ///Radius of circle in screen coordinates (-1 = needs recomputing)
  int m_iScreenRadius;
  ///Center of screen circle (needs recomputing if radius does)
  CDasherScreen::point m_screenCircleCenter;
  virtual void ComputeScreenLoc(CDasherView *pView);
};
}
/// @}

#endif
