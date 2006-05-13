// ControlPage.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __BUTTON_SETTINGS_PAGE_H__
#define __BUTTON_SETTINGS_PAGE_H__

#include "Widgets/PrefsPageBase.h"

#include "../resource.h"

#include "../DasherInterface.h"

class CAppSettings;

class CButtonSettingsPage:public CPrefsPageBase {
public:
  CButtonSettingsPage(HWND Parent, CDasherInterface * DI, CAppSettings *pAppSettings);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  HWND CustomBox;
 
  // Some status flags:
  void PopulateList();
  bool Apply();
  bool Validate();

  TCHAR m_tcBuffer[1000];
  
  HWND m_hZoomStepsSlider;
  HWND m_hRightZoomSlider;
  HWND m_hNoBoxesSlider;
  HWND m_hSafetySlider;
  HWND m_hNonUniformSlider;

  HWND m_hZoomStepsBox;
  HWND m_hRightZoomBox;
  HWND m_hNoBoxesBox;
  HWND m_hSafetyBox;
  HWND m_hNonUniformBox;
};

#endif  // ndef
