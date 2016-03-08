// Canvas.cpp
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

// 1 needed for WM_THEMECHANGED
// XXX Why lie? #define _WIN32_WINNT 0x0501


#include "..\Common\WinCommon.h"

#include <Vsstyle.h>
#include "../TabletPC/SystemInfo.h"

#include "Canvas.h"
#include "../Dasher.h"
#include "Screen.h"

#define PRESSED		0x8000
#define REPEAT		0x40000000

using namespace Dasher;

CCanvas::CCanvas(CDasher *DI) : m_pDasherInterface(DI) {
 
  m_pScreen = 0;
#ifndef HAVE_NO_THEME
  m_hTheme = NULL;
#endif
}

HWND CCanvas::Create(HWND hParent, const std::string &strFont) {
  HWND hWnd = CWindowImpl<CCanvas>::Create(hParent, NULL, NULL, WS_CHILD | WS_VISIBLE , 0);//WS_EX_CLIENTEDGE);

  m_hdc = GetDC();

  // TODO: Check out whether any of this needs to be reimplemented

  // TODO: Is this better placed in CDasher?
  m_pKeyboardHelper = new CKeyboardHelper;

  m_pScreen = new CScreen(m_hdc, m_hWnd, 300, 300, strFont);

  m_pDasherInterface->ChangeScreen(m_pScreen);
  
  return hWnd;
}
  
LRESULT CCanvas::OnCreate(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = TRUE;

  
#ifndef HAVE_NO_THEME
  m_hTheme = OpenThemeData(m_hWnd, L"Edit");
#endif

  // If we're a tablet, initialize the event-generator
  if(IsTabletPC()) {
    HRESULT h = m_CursorInRange.Initialize(m_hWnd);
    if(! SUCCEEDED(h)) {
      // detected tablet, but cant initialize the event-generator
      return -1;
    }
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CCanvas::OnDestroy(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  int iRC = ReleaseDC(m_hdc);

  if(!iRC) {
    // TODO: general error reporting code?    
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
		  NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		  (LPTSTR) & lpMsgBuf, 0, NULL);
    // Process any inserts in lpMsgBuf.
    // ...
    // Display the string.
    ::MessageBox(NULL, (LPCTSTR) lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION);
    // Free the buffer.
    LocalFree(lpMsgBuf);
  }

  bHandled = true;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////

CCanvas::~CCanvas() {
#ifndef HAVE_NO_THEME
  if(m_hTheme)
    CloseThemeData(m_hTheme);
#endif


  delete m_pScreen;

  if(m_pKeyboardHelper)
    delete m_pKeyboardHelper;
}

void CCanvas::Move(int x, int y, int Width, int Height) {
  MoveWindow(x, y, Width, Height, TRUE);
}

void CCanvas::Paint() {
 // InvalidateRect(NULL, FALSE);
 // UpdateWindow();
}

LRESULT CCanvas::OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = TRUE; 
  SendMessage(GetParent(), message, wParam, lParam);
  return 0;
}

LRESULT CCanvas::OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = TRUE; 
  SendMessage(GetParent(), WM_DASHER_FOCUS, 0, (LPARAM)&m_hWnd);
  return 0;
}

LRESULT CCanvas::OnPaint(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  PAINTSTRUCT ps;
  BeginPaint(&ps);

  RECT rc;
  GetClientRect(&rc);

  RECT rcContent;

#ifndef HAVE_NO_THEME
  if(m_hTheme) {
    DTBGOPTS oOpts;
    oOpts.dwSize = sizeof(DTBGOPTS);
    oOpts.dwFlags = DTBG_OMITCONTENT;
    oOpts.rcClip = rc;
  
    DrawThemeBackgroundEx(m_hTheme, ps.hdc, EP_EDITTEXT,
                          ETS_NORMAL, &rc, &oOpts);
  
    GetThemeBackgroundContentRect(m_hTheme, ps.hdc,
                                  EP_EDITTEXT,
                                  ETS_NORMAL, &rc, &rcContent);
  }
  else {
#endif
    DrawEdge(ps.hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);

    //rcContent.top = rc.top + 1;
    //rcContent.bottom = rc.bottom -1;
    //rcContent.left = rc.left + 1;
    //rcContent.right = rc.right - 1;
    rcContent = rc;
#ifndef HAVE_NO_THEME
  }
#endif

  m_pScreen->RealDisplay(ps.hdc, rcContent);
  
  
  EndPaint(&ps);
  
  bHandled = TRUE;
  return 0;
}

LRESULT CCanvas::OnKeyUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = TRUE;
  
  int iKeyVal(-1);
  
  if(m_pKeyboardHelper)
    iKeyVal = m_pKeyboardHelper->ConvertKeyCode(wParam);
  
  if(iKeyVal != -1) {
    m_pDasherInterface->KeyUp(GetTickCount(), iKeyVal);
  }
  return 0;		  


  // TODO: I believe all this is obsolete, but check  
//   switch(wParam) 
//     {
//     case VK_SHIFT:
//       if(GetKeyState(VK_CONTROL) & PRESSED)
// 	m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 25);
//       else
// 	m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 100);
//       return 0;
//     case VK_CONTROL:
//       if(GetKeyState(VK_SHIFT) & PRESSED)
// 	m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 175);
//       else
// 	m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 100);
      
//       return 0;
//     default:
//       return 0;		  
//     }
}

LRESULT CCanvas::OnKeyDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = TRUE;
  
  int iKeyVal(-1);
  
  if(m_pKeyboardHelper)
    iKeyVal = m_pKeyboardHelper->ConvertKeyCode(wParam);
  
  if(iKeyVal != -1) {
    m_pDasherInterface->KeyDown(GetTickCount(), iKeyVal);
  }
  return 0;

  // TODO: Also believed to be obsolete
  
//   switch (wParam) {
//     // Space, for start/stop events etc.
//   case VK_SPACE:
//     m_pDasherInterface->KeyDown(GetTickCount(), 0);
//     return 0;
//   case VK_F12:
//     centrecursor();
//     return 0;
    
//     // Boost keys
//   case VK_SHIFT:
// 		if(lParam ^ REPEAT) // ignore auto-repeat
// 			m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 175);
// 		return 0;
// 	case VK_CONTROL:
// 		if(lParam ^ REPEAT) // ignore auto-repeat
// 			m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 25);
// 		return 0;

//   // Button mode keys
//   case 0x41:
//     m_pDasherInterface->KeyDown(GetTickCount(), 1);
//     return 0;
//   case 0x53:
//     m_pDasherInterface->KeyDown(GetTickCount(), 2);
//     return 0;
//   case 0x57:
//     m_pDasherInterface->KeyDown(GetTickCount(), 3);
//     return 0;
//   case 0x58:
//     m_pDasherInterface->KeyDown(GetTickCount(), 4);
//     return 0;
// 	default:
// 		return 0;
// 	}
}

LRESULT CCanvas::OnLButtonDblClk(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  // TODO: No need to handle this

//	bHandled =  TRUE;
//	if( m_pDasherInterface->GetBoolParameter(BP_START_STYLUS)  ) 
//	{
//	    m_pDasherInterface->PauseAt(0, 0);
//	}
	return 0;
}

LRESULT CCanvas::OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  bHandled = TRUE;
  m_pDasherInterface->KeyDown(GetTickCount(), 100);
  SetFocus();
  return 0;
}

LRESULT CCanvas::OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	m_pDasherInterface->KeyUp(GetTickCount(), 100);
	return 0;

}

// PRLW: These two functions are called on Tablet PCs when the stylus comes
// within or leaves the detection range of screen. Make these appear as mouse
// clicks to start/stop on mouse click. (XXX Ideally we would prefer start/stop
// outside of canvas...)
LRESULT CCanvas::OnCursorInRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OutputDebugString(TEXT("CursorInRange\n"));

	bHandled = TRUE;

	m_pDasherInterface->KeyDown(GetTickCount(), 100);

	return 0;
}


LRESULT CCanvas::OnCursorOutOfRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OutputDebugString(TEXT("CursorOutOfRange\n"));

	bHandled = TRUE;

	m_pDasherInterface->KeyUp(GetTickCount(), 100);

	return 0;
}

LRESULT CCanvas::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (LOWORD(lParam)>0 && HIWORD(lParam) >0) {
		m_pScreen->resize(LOWORD(lParam), HIWORD(lParam));
		m_pDasherInterface->ScreenResized(m_pScreen);
		InvalidateRect( NULL, FALSE);
	}

	return 0;
}

#ifndef HAVE_NO_THEME
LRESULT CCanvas::OnThemeChanged(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  if(m_hTheme)
    CloseThemeData(m_hTheme);

  m_hTheme = OpenThemeData(m_hWnd, L"Edit");
  
  return 0;
}
#endif

LRESULT CCanvas::OnTimer(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = true;

  m_pDasherInterface->Main();

  return 0;
}

/////////////////////////////////////////////////////////////////////////////

// Gets the size of the canvas in screen coordinates.  Need if we
// want to log mouse positions normalized by the size of the
// canvas.
bool CCanvas::GetCanvasSize(int& iTop, int& iLeft, int& iBottom, int& iRight) {
  
  RECT sWindowRect;
  
  if (GetWindowRect( &sWindowRect)) {
    iTop = sWindowRect.top;
    iLeft = sWindowRect.left;
    iBottom = sWindowRect.bottom;
    iRight = sWindowRect.right;
    return true;
  }

  return false;
}


void CCanvas::SetFont(const std::string &strFont) {
    m_pScreen->SetFont(strFont);
}
