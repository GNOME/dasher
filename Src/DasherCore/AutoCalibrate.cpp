
#include "../Common/Common.h"

#include "AutoSpeedControl.h"

CAutoSpeedControl::CAutoSpeedControl(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore) 
  : CDasherComponent(pEventHandler, pSettingsStore) {
}
