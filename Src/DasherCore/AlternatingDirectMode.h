
// DasherButtons.h
// Copyright 2005 by Chris Ball

#ifndef __ALTERNATING_DIRECT_MODE_H__
#define __ALTERNATING_DIRECT_MODE_H__

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

//TODO maybe some kind of scanning/menu option here, too, tho slightly more complicated than for direct/menu mode?

  class CAlternatingDirectMode : public CDasherButtons
{
 public:
  CAlternatingDirectMode(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface);

  bool DecorateView(CDasherView *pView, CDasherInput *pInput);

  bool GetSettings(SModuleSettings **pSettings, int *iCount);

 protected:
  void SetupBoxes();

 private:
  void DirectKeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherModel *pModel);

  int m_iLastBox;
};
}
/// @}

#endif
