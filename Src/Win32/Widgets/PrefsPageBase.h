// PrefsPageBase.h
// 
// A common base class for each page in the Win32 preferences property sheet.
// Seb Wills
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PrefsPageBase_h__
#define __PrefsPageBase_h__

#include "WinWrap.h"
#include "../resource.h"
#include "../AppSettings.h"

class CPrefsPageBase:public CWinWrap {
public:
	CPrefsPageBase(HWND Parent, CAppSettings *pAppSettings);

protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

  virtual void PopulateList() = 0;
  virtual bool Apply() = 0;

  // By defaut this always validates pages. If it's possible for things to be in an
  // inconsistent state, override this and return false, after telling the user 
  // what's up in order to prevent page turning.
  virtual bool Validate();
  
  CAppSettings * const m_pAppSettings;
  HWND m_hPropertySheet; // the property sheet of which we are one page
};

#endif  /* #ifndef __PrefsPageBase_h__ */
