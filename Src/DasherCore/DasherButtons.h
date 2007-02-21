
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

/// \ingroup Input
/// @{
class CDasherButtons : public CInputFilter
{
 public:
  CDasherButtons(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase *pInterface, int iNumBoxes, int iStyle, bool bMenu, long long int iID, const char *szName);


  ~CDasherButtons();

  virtual void HandleEvent(Dasher::CEvent * pEvent);
  
  bool DecorateView(CDasherView *pView);
  
  void KeyDown(int iTime, int iId, CDasherModel *pModel, CUserLogBase *pUserLog);
  bool Timer(int Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted);

  void SetupBoxes();

  struct SBoxInfo {
    int iTop;
    int iBottom;
    int iDisplayTop;
    int iDisplayBottom;
  };

  bool GetSettings(SModuleSettings **pSettings, int *iCount);

  void NewDrawGoTo(CDasherView *pView, myint iDasherMin, myint iDasherMax, bool bActive);

    
 private:
  CSettingsStore*  m_pSettingsStore;
  
  SBoxInfo *m_pBoxes;

  int m_iNumBoxes;
  int m_iStyle;
  bool m_bMenu;
  
  int iActiveBox;

  int m_iLastBox;

  int iTargetWidth;

  int m_iLastTime;
  bool m_bHighlight;
  bool m_bDecorationChanged;

};
/// @}

#endif
