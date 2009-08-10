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
  virtual void Timer(int iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void HandleEvent(Dasher::CEvent * pEvent);

private:
  int m_iStatus;
  int m_iChangeTime;
  int m_iCircleRadius;
  int m_iScreenRadius;
};
}
/// @}

#endif
