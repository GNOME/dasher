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

// XXX why lie? #define _WIN32_WINNT 0x0501

// aiming for _WIN32_WINNT < 0x501 => HAVE_NO_THEME
#ifdef _WIN32_WCE
#define HAVE_NO_THEME
#endif

#include <windows.h>
#include <winuser.h>
#ifndef HAVE_NO_THEME
#include <uxtheme.h>
#define WM_THEMECHANGED                 0x031A
#endif

#ifndef _WIN32_WCE
#include "../TabletPC/CursorInRange.h"
#endif 

#include "../../DasherCore/DasherComponent.h"
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
  
class CCanvas : public ATL::CWindowImpl<CCanvas>, public Dasher::CDasherComponent {
 public:
  static ATL::CWndClassInfo& GetWndClassInfo() { 
#ifndef _WIN32_WCE
    static ATL::CWndClassInfo wc = {
      { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc,
	0, 0, NULL, NULL, NULL, NULL, NULL, _T("CANVAS"), NULL },
      NULL, NULL, MAKEINTRESOURCE(IDC_CROSS), TRUE, 0, _T("") 
    };
#else
  static ATL::CWndClassInfo wc = {
      {CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc,
	0, 0, NULL, NULL, NULL, NULL, NULL, _T("CANVAS") },
      NULL, NULL, MAKEINTRESOURCE(IDC_CROSS), TRUE, 0, _T("") 
    };
#endif
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
#ifndef _WIN32_WCE
    MESSAGE_HANDLER(WM_CURSOR_IN_RANGE, OnCursorInRange)
    MESSAGE_HANDLER(WM_CURSOR_OUT_OF_RANGE, OnCursorOutOfRange)
#endif
    MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
    MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
#ifndef HAVE_NO_THEME
    MESSAGE_HANDLER(WM_THEMECHANGED , OnThemeChanged)
#endif
  END_MSG_MAP()

  CCanvas(Dasher::CDasher *DI, Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore);
  ~CCanvas();

  HWND Create(HWND hParent);
  
  void DoFrame();
  
  LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#ifndef HAVE_NO_THEME
  LRESULT OnThemeChanged(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif
  LRESULT OnMouseMove(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnRButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnRButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#ifndef _WIN32_WCE
  LRESULT OnCursorInRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCursorOutOfRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif
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
/*   void StartOnLeftClick(bool Value) { */
/*     startonleft = Value; */
/*   } */


  //  void MousePosStart(bool Value);

/*   void setkeycoords(int coords[18]) { */
/*     for(int i = 0; i < 18; i++) { */
/*       keycoords[i] = coords[i]; */
/*     }; */
/*   } */
/*   int *getkeycoords() { */
/*     return keycoords; */
/*   } */

/*   void setforward(bool value) { */
/*     forward = value; */
/*   } */
/*   void setbackward(bool value) { */
/*     backward = value; */
/*   } */
/*   void setselect(bool value) { */
/*     select = value; */
/*   } */
/*   bool getforward() { */
/*     return forward; */
/*   } */
/*   bool getbackward() { */
/*     return backward; */
/*   } */
/*   bool getselect() { */
/*     return select; */
/*   } */
 
//  void centrecursor();
  //  void StartStop();

/*   void Pause() { */
/*     running = 0; */
/*   } */
/*   bool Running() { */
/*     return running; */
/*   } */

  void SetScreenInterface(Dasher::CDasherInterfaceBase * dasherinterface);
  
  int OnTimer();

  bool GetCanvasSize(int& pTop, int& pLeft, int& pBottom, int& pRight);

  void HandleEvent(Dasher::CEvent *pEvent);

private:

  HDC m_hdc;
  //  int keycoords[18], buttonnum, yscaling;
  // bool forward, backward, select;

  CScreen *m_pScreen;
  Dasher::CDasher * m_pDasherInterface;

#ifndef HAVE_NO_THEME
  HTHEME m_hTheme;
#endif

  // Input devices:

  //Dasher::CDasherMouseInput * m_pMouseInput;
  //Dasher::CSocketInput * m_pSocketInput;

  // TODO: A lot, if not all, of these variables are obsolete

  //Dasher::screenint imousex, imousey;
  UINT MY_TIMER;
  //bool startonleft;
  //  bool direction;
  //  bool running;
  //bool firstwindow;
  //bool secondwindow;
  //bool lbuttonheld;
  //bool inturbo;
  //bool enabletime;
  //DWORD startturbo;
  //DWORD endturbo;
  //DWORD lastlbutton;
  //DWORD mousepostime;
  //  DWORD previoustime;
  //RECT coords;

  // Indicates that a button is depressed, so we dont stop on idle
  bool m_bButtonDown;
  // Ticks as last event, for stop on idle
  DWORD m_dwTicksLastEvent;

#ifndef _WIN32_WCE
  // Enables tablet pc events
  CCursorInRange m_CursorInRange;
#endif

  CKeyboardHelper *m_pKeyboardHelper;

};

#endif /* #ifndef __Canvas_h__ */
