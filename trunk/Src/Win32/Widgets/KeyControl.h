// AlphabetBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __KeyBox_h__
#define __KeyBox_h__

#include "../resource.h"
#include "WinWrap.h"

namespace Dasher
{
	class CDasher;
}


class CKeyBox: public CWinWrap 
{
public:
	CKeyBox(HWND Parent, Dasher::CDasher * pDasher);
  void PopulateWidgets();
    std::string GetControlText(HWND Dialog, int ControlID);
protected:
    LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	Dasher::CDasher * m_pDasher;

  HWND CustomBox;
  HWND slider;
  HWND uniformbox;
  int keycoords[18];
  int ypixels;
  int mouseposdist;
  double NewUniform;

  // Some status flags:
  bool Editing;
  bool SelectionSet;

};

#endif  /* #ifndef __AlphabetBox_h__ */
