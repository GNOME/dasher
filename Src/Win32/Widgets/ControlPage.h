// AlphabetBox.h
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

class CControlPage:public CWinWrap {
public:
  CControlPage(HWND Parent, CDasherInterface * DI);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  CDasherInterface * m_pDasherInterface;
  HWND CustomBox;

  std::vector < std::string > ColourList;
  std::string m_CurrentColours;
  Dasher::CColourIO::ColourInfo CurrentInfo;

  // Some status flags:
  void PopulateList();
  void InitCustomBox();
  bool UpdateInfo();
};

#endif  /* #ifndef __AlphabetBox_h__ */
