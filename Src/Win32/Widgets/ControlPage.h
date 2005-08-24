// ControlPage.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __ControlPage_h__
#define __ControlPage_h__

#include "../resource.h"

#include "../../DasherCore/Win32/DasherInterface.h"
#include "../../DasherCore/ColourIO.h"

class CAppSettings;

class CControlPage:public CWinWrap {
public:
  CControlPage(HWND Parent, CDasherInterface * DI, CAppSettings *pAppSettings);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  CDasherInterface * m_pDasherInterface;
  HWND CustomBox;
  CAppSettings *m_pAppSettings;

  std::vector < std::string > ColourList;
  std::string m_CurrentColours;
  Dasher::CColourIO::ColourInfo CurrentInfo;
  TCHAR m_tcBuffer[1000];
  HWND SB_slider;
  HWND speedbox;
  // Some status flags:
  void PopulateList();
  bool Apply();
  bool Validate();
};

#endif  // ndef
