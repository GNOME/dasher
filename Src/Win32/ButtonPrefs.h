// Prefs.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __BUTTON_PREFS_H__
#define __BUTTON_PREFS_H__

#include "../resource.h"
#include "../../DasherCore/DasherTypes.h"

#include "../AppSettings.h"

#include "ButtonType.h"
#include "ButtonSettings.h"

namespace Dasher
{
	class CDasher;
}


class CButtonPrefs:public CWinWrap {
public:
	CButtonPrefs(HWND hParent, Dasher::CDasher *pDasher, CAppSettings *pAppSettings);
  ~CButtonPrefs();
  void PopulateWidgets();
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  HWND m_hCustomBox;
  Dasher::CDasher *m_pDasher;
  CAppSettings *m_pAppSettings;

  CButtonTypePage *m_pButtonTypePage;
  CButtonSettingsPage *m_pButtonSettingsPage;
};

#endif
