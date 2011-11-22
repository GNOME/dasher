
// DasherButtons.h 
// Copyright 2005 by Chris Ball

#ifndef __DASHER_BUTTONS_H__
#define __DASHER_BUTTONS_H__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "Event.h"
#include "InputFilter.h"


using namespace std;
namespace Dasher {
/// \ingroup Input
/// @{
class CDasherButtons : public CStaticFilter
{
 public:
  CDasherButtons(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, bool bMenu, ModuleID_t iID, const char *szName);

  ~CDasherButtons();

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput)=0;
  
  void KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  void Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol);
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
  unsigned long m_iScanTime;
  
  virtual void DirectKeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherModel *pModel);
};
}
/// @}

#endif
