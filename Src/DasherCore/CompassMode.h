
// DasherButtons.h
// Copyright 2005 by Chris Ball

#ifndef __COMPASS_MODE_H__
#define __COMPASS_MODE_H__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "Event.h"
#include "DasherButtons.h"


using namespace std;
namespace Dasher {
/// \ingroup Input
/// @{
class CCompassMode : public CDasherButtons
{
 public:
  CCompassMode(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface);

  virtual void HandleEvent(int iParameter);

  bool DecorateView(CDasherView *pView, CDasherInput *pInput);

  bool GetSettings(SModuleSettings **pSettings, int *iCount);

 protected:
  void SetupBoxes();

 private:
  int iTargetWidth;
};
}
/// @}

#endif
