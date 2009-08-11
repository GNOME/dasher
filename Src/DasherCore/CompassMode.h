
// DasherButtons.h 
// Copyright 2005 by Chris Ball

#ifndef __COMPASS_MODE_H__
#define __COMPASS_MODE_H__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Alphabet/Alphabet.h"
#include <algorithm>
#include "DasherComponent.h"
#include "Event.h"
#include "DasherButtons.h"


using namespace std;
namespace Dasher {
/// \ingroup Input
/// @{
class CCompassMode : public CDasherButtons
{
 public:
  CCompassMode(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface);

  virtual void HandleEvent(Dasher::CEvent * pEvent);
  
  bool DecorateView(CDasherView *pView);

  bool GetSettings(SModuleSettings **pSettings, int *iCount);

 protected:
  void SetupBoxes();
  
 private: 
  int iTargetWidth;
};
}
/// @}

#endif
