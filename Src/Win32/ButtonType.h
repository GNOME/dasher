// ControlPage.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __BUTTON_TYPE_PAGE_H__
#define __BUTTON_TYPE_PAGE_H__

#include "Widgets/PrefsPageBase.h"

#include "../resource.h"

#include "../DasherInterface.h"

class CAppSettings;

class CButtonTypePage:public CPrefsPageBase {
public:
  CButtonTypePage(HWND Parent, CDasherInterface * DI, CAppSettings *pAppSettings);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  HWND CustomBox;
 
  // Some status flags:
  void PopulateList();
  bool Apply();
  bool Validate();

  HWND m_hSlider;
};

#endif  // ndef
