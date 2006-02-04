#pragma once
#include "../DasherCore/DasherInput.h"

namespace Dasher {
  class CDasherMouseInput;
} 

class Dasher::CDasherMouseInput : public CDasherInput {
public:
  CDasherMouseInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, HWND _hwnd);
  ~CDasherMouseInput(void);

  virtual int GetCoordinates(int iN, myint * pCoordinates);

  // Get the number of co-ordinates that this device supplies

  virtual int GetCoordinateCount() {
    return 2;
  };

private:
  HWND m_hwnd;
};
