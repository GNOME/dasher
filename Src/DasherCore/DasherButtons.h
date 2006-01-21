
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

class CDasherButtons : public CInputFilter
{
 public:
  CDasherButtons(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface, int iNumBoxes, int iStyle, bool bMenu);


  ~CDasherButtons();

  virtual void HandleEvent(Dasher::CEvent * pEvent);
  
  void DecorateView(CDasherView *pView);
  
  void KeyDown(int iTime, int iId, CDasherModel *pModel);
  void Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel);

  void SetupBoxes();

  struct SBoxInfo {
    int iTop;
    int iBottom;
    int iDisplayTop;
    int iDisplayBottom;
  };
    
 private:
  CSettingsStore*  m_pSettingsStore;
  
  SBoxInfo *m_pBoxes;

  int m_iNumBoxes;
  int m_iStyle;
  bool m_bMenu;
  
  int iActiveBox;

  int m_iLastBox;

  int iTargetWidth;

};

#endif
