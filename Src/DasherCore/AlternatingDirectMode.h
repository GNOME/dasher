
// DasherButtons.h 
// Copyright 2005 by Chris Ball

#ifndef __ALTERNATING_DIRECT_MODE_H__
#define __ALTERNATING_DIRECT_MODE_H__

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

//TODO maybe some kind of scanning/menu option here, too, tho slightly more complicated than for direct/menu mode?

  class CAlternatingDirectMode : public CDasherButtons
{
 public:
  CAlternatingDirectMode(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface);
  
  bool DecorateView(CDasherView *pView);

  bool GetSettings(SModuleSettings **pSettings, int *iCount);

 protected:
  void SetupBoxes();
  
 private:
  void DirectKeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog);
    
  int m_iLastBox;
};
}
/// @}

#endif
