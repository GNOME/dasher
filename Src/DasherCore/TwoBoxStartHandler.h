#ifndef __TWO_BOX_START_HANDLER_H__
#define __TWO_BOX_START_HANDLER_H__

#include "StartHandler.h"

namespace Dasher {
/// \ingroup Start
/// @{
class CTwoBoxStartHandler : public CStartHandler {
public:
  CTwoBoxStartHandler(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface);

  virtual bool DecorateView(CDasherView *pView);
  virtual void Timer(int iTime, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);
  virtual void HandleEvent(Dasher::CEvent * pEvent);

 private:
  int m_iBoxEntered;
  int m_iBoxStart;
  bool m_bInBox;
};
}
/// @}

#endif
