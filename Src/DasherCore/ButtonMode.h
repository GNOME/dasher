
// ButtonMode.h 
// Copyright 2009 by Alan Lawrence

#ifndef __BUTTON_MODE_H__
#define __BUTTON_MODE_H__

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
/// Handles the "menu mode" and "direct mode" input filters, according to the bMenu constructor parameter.
class CButtonMode : public CDasherButtons
{
 public:
  CButtonMode(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface, bool bMenu, int iID, const char *szName);

  virtual void HandleEvent(Dasher::CEvent * pEvent);
  bool Timer(int Time, CDasherView *pView, CDasherModel *pModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
  bool DecorateView(CDasherView *pView);

  //override to get mouse clicks/taps back again
  virtual void KeyDown(int Time, int iId, CDasherView *pDasherView, CDasherModel *pModel, CUserLogBase *pUserLog, bool bPos, int iX, int iY);
  
  bool GetSettings(SModuleSettings **pSettings, int *iCount);
 protected: 
  void SetupBoxes();
  void DirectKeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog);
 private:
  bool m_bHighlight;
  int m_iLastTime;
};
}
/// @}

#endif
