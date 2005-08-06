// AlphabetBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __AdvancedPage_h__
#define __AdvancedPage_h__

#include "../resource.h"
#include "../AppSettings.h"

#include "../../DasherCore/Win32/DasherInterface.h"
#include "../../DasherCore/ColourIO.h"

class CAdvancedPage:public CWinWrap {
public:
  CAdvancedPage(HWND Parent, CDasherInterface * DI, CAppSettings *pAppSettings);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  CDasherInterface * m_pDasherInterface;
  CAppSettings *m_pAppSettings;
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
  bool Validate();

 TCHAR m_tcBuffer[1000];

};

#endif  /* #ifndef __AlphabetBox_h__ */
