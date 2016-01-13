// AdvancedPage.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __AdvancedPage_h__
#define __AdvancedPage_h__

#include "PrefsPageBase.h"

#include "../resource.h"
#include "../AppSettings.h"

class CAdvancedPage:public CPrefsPageBase {
public:
  CAdvancedPage(HWND Parent, CAppSettings *pAppSettings);
  
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

protected:
private:

  void PopulateList();
  bool Apply();

 TCHAR m_tcBuffer[1000];
 std::vector<std::string> m_ControlBoxItems;
};

#endif  /* #ifndef __AlphabetBox_h__ */
