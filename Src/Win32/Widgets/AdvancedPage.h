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

#include "../../DasherCore/DasherInterfaceBase.h"
#include "../../DasherCore/ColourIO.h"

class CAdvancedPage:public CPrefsPageBase {
public:
  CAdvancedPage(HWND Parent, Dasher::CDasherInterfaceBase * DI, CAppSettings *pAppSettings);
  
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

protected:
private:
  HWND CustomBox;

  std::string GetControlText(HWND Dialog, int ControlID);

  std::vector < std::string > ColourList;
  std::string m_CurrentColours;
  Dasher::CColourIO::ColourInfo CurrentInfo;

  // Some status flags:
  void PopulateList();
  void InitCustomBox();
  bool UpdateInfo();
  bool Apply();

 TCHAR m_tcBuffer[1000];

};

#endif  /* #ifndef __AlphabetBox_h__ */
