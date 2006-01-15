#ifndef __AUTO_SPEED_CONTROL_H__
#define __AUTO_SPEED_CONTROL_H__

#include "DasherComponent.h"

class CAutoSpeedControl : public CDasherComponent {
  CAutoSpeedControl(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore);
};

#endif
