
// DasherButtons.h 
// Copyright 2005 by Chris Ball

#ifndef __DASHER_BUTTONS_H__
#define __DASHER_BUTTONS_H__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Alphabet/Alphabet.h"
#include <algorithm>
#include "DasherComponent.h"
#include "Event.h"
#include "InputFilter.h"


using namespace std;
namespace Dasher {
/// \ingroup Input
/// @{
class CDasherButtons : public CInputFilter
{
 public:
  CDasherButtons(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface, bool bMenu, ModuleID_t iID, const char *szName);

  ~CDasherButtons();

  virtual bool DecorateView(CDasherView *pView)=0;
  
  void KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog);
  bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted, CExpansionPolicy **pol);
  void Activate();
  
  struct SBoxInfo {
    int iTop;
    int iBottom;
    int iDisplayTop;
    int iDisplayBottom;
  };

 protected:
  virtual void SetupBoxes()=0;
  void NewDrawGoTo(CDasherView *pView, myint iDasherMin, myint iDasherMax, bool bActive);
  bool m_bMenu;
  bool m_bDecorationChanged;
  SBoxInfo *m_pBoxes;
  int m_iNumBoxes, iActiveBox;
  int m_iScanTime;
  
  virtual void DirectKeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog);
};
}
/// @}

#endif
