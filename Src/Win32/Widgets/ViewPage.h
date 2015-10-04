// ViewPage.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __ViewPage_h__
#define __ViewPage_h__

#include "PrefsPageBase.h"

#include "../resource.h"
#include "../AppSettings.h"

#include "../Dasher.h"
#include "../../DasherCore/ColourIO.h"

class CViewPage:public CPrefsPageBase {
public:
	CViewPage(HWND Parent, CAppSettings *pAppSettings);
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

private:
  std::vector < std::string > ColourList;
  std::string m_CurrentColours;
  Dasher::CColourIO::ColourInfo CurrentInfo;

  // Some status flags:
  void PopulateList();
  void InitCustomBox();
  bool UpdateInfo();
  bool Apply();

};

#endif  /* #ifndef */
