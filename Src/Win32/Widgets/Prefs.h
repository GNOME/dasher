// Prefs.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PrefsBox_h__
#define __PrefsBox_h__

#include "../resource.h"
#include "../../DasherCore/DasherTypes.h"

#include "../Dasher.h"

#include "AlphabetBox.h"
#include "ColourBox.h"
#include "ControlPage.h"
#include "ViewPage.h"
#include "AdvancedPage.h"
#include "LMPage.h"
#include "../AppSettings.h"


class CPrefs:public CWinWrap {
public:
  CPrefs(HWND hParent, CDasher * pDasher, CAppSettings *pAppSettings);
  ~CPrefs();
  void PopulateWidgets();
protected:
    LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  HWND m_hCustomBox;
  CDasher *m_pDasher;
  CAppSettings *m_pAppSettings;

  CAlphabetBox *m_pAlphabetBox;
  CColourBox *m_pColourBox;
  CControlPage *m_pControlPage;
  CViewPage *m_pViewPage;
  CAdvancedPage *m_pAdvancedPage;
  CLMPage *m_pLMPage;
};

#endif  /* #ifndef __PrefsBox_h__ */
