#ifndef __TWO_BOX_START_HANDLER_H__
#define __TWO_BOX_START_HANDLER_H__

#include "StartHandler.h"
#include "SettingsStore.h"

namespace Dasher {
/// \ingroup Start
/// @{
class CTwoBoxStartHandler : public CStartHandler, public CSettingsUser {
public:
  CTwoBoxStartHandler(CDefaultFilter *pCreator);

  virtual bool DecorateView(CDasherView *pView);
  virtual void Timer(unsigned long iTime, dasherint iX, dasherint iY, CDasherView *pView);
  virtual void onPause();

 private:
  ///Box currently being displayed, _iff_ BP_DASHER_PAUSED is set
  bool m_bFirstBox;
  ///Time at which mouse entered whichever box is current, or numeric_limits::max() if it hasn't
  unsigned long m_iBoxEntered;
  ///Time at which second box was first displayed
  unsigned long m_iBoxStart;
};
}
/// @}

#endif
