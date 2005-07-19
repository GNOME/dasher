#pragma once
#include "../DasherCore/DasherInput.h"
#include <windows.h>

namespace Dasher {
  class CDasherMouseInput;
} class Dasher::CDasherMouseInput:public CDasherInput {
public:
  CDasherMouseInput(HWND _hwnd);
  ~CDasherMouseInput(void);

  virtual int GetCoordinates(int iN, myint * pCoordinates);

  // Get the number of co-ordinates that this device supplies

  virtual int GetCoordinateCount() {
    return 2;
  };

private:
  HWND m_hwnd;
};
