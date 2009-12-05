// ControlPage.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __ControlPage_h__
#define __ControlPage_h__

#include "PrefsPageBase.h"

#include "../resource.h"

#include "../../DasherCore/DasherInterfaceBase.h"
#include "../../DasherCore/ColourIO.h"
#include "../ModuleSettings.h"

class CAppSettings;

class CControlPage:public CPrefsPageBase {
public:
  CControlPage(HWND Parent, Dasher::CDasherInterfaceBase * DI, CAppSettings *pAppSettings);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  HWND CustomBox;
  
  std::vector < std::string > ColourList;
  std::string m_CurrentColours;
  Dasher::CColourIO::ColourInfo CurrentInfo;
  TCHAR m_tcBuffer[1000];
  
  // Handles to important widgets
  HWND SB_slider;
  HWND speedbox;
  HWND m_hMousePosStyle;
  
  void PopulateList();
  bool Apply();
 
  CModuleSettings *m_pModuleSettingsDialogue;
};

#endif  // ndef
