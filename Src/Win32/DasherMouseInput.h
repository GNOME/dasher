#pragma once
#include "../DasherCore/DasherInput.h"

namespace Dasher {
  class CDasherMouseInput;
} 

class Dasher::CDasherMouseInput : public CScreenCoordInput {
public:
  CDasherMouseInput(HWND _hwnd);
  ~CDasherMouseInput(void);

  virtual bool GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView);

private:
  HWND m_hwnd;
};
