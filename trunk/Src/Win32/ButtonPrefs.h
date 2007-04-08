// Prefs.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __BUTTON_PREFS_H__
#define __BUTTON_PREFS_H__

#include "Common/WinWrap.h"

#include "../resource.h"
#include "../../DasherCore/DasherTypes.h"

#include "../AppSettings.h"

namespace Dasher
{
	class CDasher;
}


class CButtonPrefs:public CWinWrap {
public:
	CButtonPrefs(HWND hParent, Dasher::CDasher *pDasher, CAppSettings *pAppSettings);
  ~CButtonPrefs();
  
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
  void PopulateWidgets();
  bool Apply();

private:
  Dasher::CDasher *m_pDasher;
  CAppSettings *m_pAppSettings;

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

#endif
