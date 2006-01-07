// AlphabetBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __ColourBox_h__
#define __ColourBox_h__

#include "../resource.h"

#include "../DasherInterface.h"
#include "../../DasherCore/ColourIO.h"
#include "WinWrap.h"

class CColourBox:public CWinWrap {
public:
  CColourBox(HWND Parent, CDasherInterface * DI);
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
   bool Apply();
  bool Validate();
};

#endif  /* #ifndef __AlphabetBox_h__ */
