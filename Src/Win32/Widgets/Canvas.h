// Canvas.h
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __Canvas_h__
#define __Canvas_h__

#include <windows.h>
#include <winuser.h>
#ifndef HAVE_NO_THEME
#include <uxtheme.h>
#define WM_THEMECHANGED                 0x031A
#endif

#include "../TabletPC/CursorInRange.h"
#include "../../DasherCore/DasherTypes.h"
#include "../KeyboardHelper.h"

namespace Dasher {
  class CDasherMouseInput;
  class CSocketInput;
  class CDasher;
  class CDasherInterfaceBase;
}

class CEdit;
class CScreen;
  
class CCanvas : public ATL::CWindowImpl<CCanvas> {
 public:
  static ATL::CWndClassInfo& GetWndClassInfo() { 
    static ATL::CWndClassInfo wc = {
      { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc,
	0, 0, NULL, NULL, NULL, NULL, NULL, _T("CANVAS"), NULL },
      NULL, NULL, MAKEINTRESOURCE(IDC_CROSS), TRUE, 0, _T("") 
    };
    return wc;
  }
  
  BEGIN_MSG_MAP( CCanvas )
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
    MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
    MESSAGE_HANDLER(WM_CURSOR_IN_RANGE, OnCursorInRange)
    MESSAGE_HANDLER(WM_CURSOR_OUT_OF_RANGE, OnCursorOutOfRange)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
#ifndef HAVE_NO_THEME
    MESSAGE_HANDLER(WM_THEMECHANGED , OnThemeChanged)
#endif
  END_MSG_MAP()

  CCanvas(Dasher::CDasher *DI);
  ~CCanvas();

  HWND Create(HWND hParent, const std::string &strFont);
  
  LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#ifndef HAVE_NO_THEME
  LRESULT OnThemeChanged(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif
  LRESULT OnCursorInRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCursorOutOfRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnLButtonDblClk(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnPaint(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCreate(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnDestroy(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnTimer(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnKeyUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnKeyDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  
  void Move(int x, int y, int Width, int Height);
  void Paint();
    
  HWND getwindow() {
    return m_hWnd;
  } 
  
  int OnTimer();

  bool GetCanvasSize(int& pTop, int& pLeft, int& pBottom, int& pRight);

  void SetFont(const std::string &strFont);

private:

  HDC m_hdc;

  CScreen *m_pScreen;
  Dasher::CDasher * m_pDasherInterface;

#ifndef HAVE_NO_THEME
  HTHEME m_hTheme;
#endif

  // Enables tablet pc events
  CCursorInRange m_CursorInRange;

  CKeyboardHelper *m_pKeyboardHelper;

};

#endif /* #ifndef __Canvas_h__ */
