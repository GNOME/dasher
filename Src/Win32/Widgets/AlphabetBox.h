// AlphabetBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __AlphabetBox_h__
#define __AlphabetBox_h__

#include "../resource.h"
#include "PrefsPageBase.h"

class CAlphabetBox : public CPrefsPageBase {
public:
	CAlphabetBox(HWND Parent, CAppSettings *pAppSettings);
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  HWND m_hPropertySheet;

  std::vector < std::string > AlphabetList;
  std::string m_CurrentAlphabet;

  HWND slider;
  HWND uniformbox;

  TCHAR m_tcBuffer[1000];

  void PopulateList();
  bool Apply();
 
};

#endif /* #ifndef __AlphabetBox_h__ */
