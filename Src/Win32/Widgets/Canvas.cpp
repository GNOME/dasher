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

#ifndef _WIN32_WCE
#include <Tmschema.h>
#include "../TabletPC/SystemInfo.h"
#endif

#include "Canvas.h"
#include "../Dasher.h"
#include "Screen.h"

#define PRESSED		0x8000
#define REPEAT		0x40000000

using namespace Dasher;

CCanvas::CCanvas(CDasher *DI, Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore)
  : CDasherComponent(pEventHandler, pSettingsStore) {

  m_pDasherInterface = DI;
  
 
  m_dwTicksLastEvent = 0;
  m_bButtonDown = false;
  m_pScreen = 0;
#ifndef HAVE_NO_THEME
  m_hTheme = NULL;
#endif

#ifndef _WIN32_WCE

#else
  WNDCLASS canvasclass;
  GetClassInfo(NULL, TEXT("STATIC"), &canvasclass);
  canvasclass.lpszClassName = TEXT("CANVAS");
#endif
}

HWND CCanvas::Create(HWND hParent) {
  HWND hWnd = CWindowImpl<CCanvas>::Create(hParent, NULL, NULL, WS_CHILD | WS_VISIBLE , 0);//WS_EX_CLIENTEDGE);

  m_hdc = GetDC();
  HDC hdc2 = GetDC();
  HDC hdc3 = GetDC();

  // TODO: Check out whether any of this needs to be reimplemented

  // Create input device objects
  // NB We create the SocketInput object now, even if socket input is not enabled, because
  // we can't safely create it later in response to a parameter change event (because it itself
  // needs to register an event listener when it constructs itself).
 

  // m_pSocketInput = (CSocketInput *)m_pDasherInterface->GetModule(1);
  // m_pSocketInput->Ref();

  // m_pMouseInput = (CDasherMouseInput *)m_pDasherInterface->GetModule(0); 
  // m_pMouseInput->Ref();
  
  // if(m_pDasherInterface->GetBoolParameter(BP_SOCKET_INPUT_ENABLE)) {
  //   m_pSocketInput->StartListening();
  //   m_pDasherInterface->SetInput(1);
  // }
  // else {
  //  m_pDasherInterface->SetInput(0);
  // }

  // TODO: Is this better placed in CDasher?
  m_pKeyboardHelper = new CKeyboardHelper;

  m_pScreen = new CScreen(m_hdc, m_hWnd, 300, 300);
  m_pScreen->SetFont(m_pDasherInterface->GetStringParameter(SP_DASHER_FONT));

  m_pDasherInterface->ChangeScreen(m_pScreen);

// 	for(int i = 0; i < 18; i++) 
// 	{
// 		keycoords[i] = 0;
// 	}

//  running = 0;
//  previoustime = GetTickCount();
//  direction = 0;
  
  return hWnd;
}

void CCanvas::SetScreenInterface(Dasher::CDasherInterfaceBase *dasherinterface) {
  m_pScreen->SetInterface(dasherinterface);
}
  
LRESULT CCanvas::OnCreate(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = TRUE;

  
#ifndef HAVE_NO_THEME
  m_hTheme = OpenThemeData(m_hWnd, L"Edit");
#endif

#ifndef _WIN32_WCE
  // If we're a tablet, initialize the event-generator
  if(IsTabletPC()) {
    HRESULT h = m_CursorInRange.Initialize(m_hWnd);
    if(! SUCCEEDED(h)) {
      // detected tablet, but cant initialize the event-generator
      return -1;
    }
  }
#endif

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
    return 0;
  }

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
#ifdef _DEBUG
  TCHAR tmpAutoOffset[128];
#endif
  
  bHandled = TRUE;
  
  int iKeyVal(-1);
  
  if(m_pKeyboardHelper)
    iKeyVal = m_pKeyboardHelper->ConvertKeyCode(wParam);
  
  if(iKeyVal != -1) {
    m_pDasherInterface->KeyDown(GetTickCount(), iKeyVal);
    return 0;
  }

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
 
#ifndef _WIN32_WCE
  // FIXME - what does this do - please document
  LPARAM lp = GetMessageExtraInfo();
  if (lp == 0xFF515702)
    return 0; 
  // ---
#endif
  
  int xPos = GET_X_LPARAM(lParam); 
  int yPos = GET_Y_LPARAM(lParam); 

  m_pDasherInterface->KeyDown(GetTickCount(), 100, true, xPos, yPos);
  
  // TODO: Reimplement
  //	else if ( m_pDasherInterface->GetBoolParameter(BP_START_STYLUS)  ) 
  //	{
  //		// DJW - for the time being we only do stylus mode if not BP_START_MOUSE 
  //
  //		if ( m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED) )
  //			m_pDasherInterface->Unpause(GetTickCount());
  //	}
  
  SetFocus();

  m_bButtonDown = true;
  return 0;
}

LRESULT CCanvas::OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	int xPos = GET_X_LPARAM(lParam);
	int yPos = GET_Y_LPARAM(lParam);

	m_pDasherInterface->KeyUp(GetTickCount(), 100, true, xPos, yPos);

  // TODO: Check whether this needs to be reimplemented
	//endturbo = GetTickCount();

	//if(endturbo - startturbo > 1) 
	//{
	//	TCHAR deb[80];
	//	wsprintf(deb, TEXT("start: %d\nend: %d\nduration: %d"), startturbo, endturbo, endturbo - startturbo);
	//	OutputDebugString(deb);
	//}
	//lbuttonheld = 0;

	//m_bButtonDown = false;

	return 0;

}

#ifndef _WIN32_WCE
LRESULT CCanvas::OnCursorInRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OutputDebugString(TEXT("CursorInRange\n"));

	bHandled = TRUE;

	//SetFocus();

	if ( m_pDasherInterface->GetBoolParameter(BP_START_MOUSE) ) 
	{
		if (m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED))
			m_pDasherInterface->Unpause(GetTickCount());
	}

	return 0;
}


LRESULT CCanvas::OnCursorOutOfRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OutputDebugString(TEXT("CursorOutOfRange\n"));

	bHandled = TRUE;
	if ( m_pDasherInterface->GetBoolParameter(BP_START_MOUSE) ) 
	{
		if (!m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED))
			m_pDasherInterface->PauseAt(0, 0);
	}

	return 0;
}
#endif

LRESULT CCanvas::OnRButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
//	bHandled = TRUE;
//	if( m_pDasherInterface->GetBoolParameter(BP_START_STYLUS)  ) 
//	{
//		if( m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED) )
//			m_pDasherInterface->Unpause(GetTickCount());
//	}
//	m_bButtonDown = true;
	return 0;
}


LRESULT CCanvas::OnRButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	m_bButtonDown = false;
    return 0;
}

LRESULT CCanvas::OnMouseMove(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = true;

// TODO: Is any of this needed?

  // TODO: Reimplement pause on idle in the core

//	imousex = LOWORD(lParam);
//	imousey = HIWORD(lParam);
	m_dwTicksLastEvent	= GetTickCount();
//	if( m_pDasherInterface->GetBoolParameter(BP_START_STYLUS)  ) 
//	{
//		if( m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED) )
//			m_pDasherInterface->Unpause(GetTickCount());
//	}

	return 0;
}

LRESULT CCanvas::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(m_pScreen != 0) {
		delete m_pScreen;
		m_pScreen = 0;
	}

	if (LOWORD(lParam)>0 && HIWORD(lParam) >0) {
		m_pScreen = new CScreen(m_hdc, m_hWnd, LOWORD(lParam), HIWORD(lParam));
    m_pScreen->SetFont(m_pDasherInterface->GetStringParameter(SP_DASHER_FONT));
		m_pDasherInterface->ChangeScreen(m_pScreen);
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

void CCanvas::DoFrame()
{
	//POINT mousepos2;
	//GetCursorPos(&mousepos2);

	//ScreenToClient(&mousepos2);

	//POINT mousepos;
	//GetCursorPos(&mousepos);

  DWORD dwTicks = GetTickCount();
	// If not paused need to consider stop on idle
	if( !m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED)  ) 
	{
	  // TODO: Pause on idle needs to be moved into the core

		// only pause if button is not down
		if( !m_bButtonDown && m_pDasherInterface->GetBoolParameter(BP_STOP_IDLE)  ) 
		{
      // TODO: Brink this back into core
			if (dwTicks - m_dwTicksLastEvent > m_pDasherInterface->GetLongParameter(LP_STOP_IDLETIME) )
			{
				// idle time exceeded
				m_pDasherInterface->PauseAt(0, 0);
			}
		}
	}
}

// void CCanvas::centrecursor() {
//   POINT mousepos;
//   mousepos.x = m_pScreen->GetWidth() / 2;

//   mousepos.y = m_pScreen->GetHeight() / 2;

//   ClientToScreen( &mousepos);

//   SetCursorPos(mousepos.x, mousepos.y);

// };

// void CCanvas::MousePosStart(bool Value) {
//   if(Value == false) {
//     firstwindow = false;
//     secondwindow = false;
//   }
// }

/////////////////////////////////////////////////////////////////////////////

// void CCanvas::StartStop() {

//   if(running == 0) {
//     SetCapture();
//     running = 1;
//     m_pDasherInterface->Unpause(GetTickCount());
//     firstwindow = false;
//     secondwindow = false;
//     mousepostime = 0;

//   }
//   else {
//     m_pDasherInterface->PauseAt(0, 0);
//     running = 0;
// //              if (speakonstop==true) { // FIXME - reimplement this
// //                      m_DasherEditBox->speak(2);
// //              }
//     ReleaseCapture();
//   }
// }

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


void CCanvas::HandleEvent(Dasher::CEvent *pEvent) {

  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    switch (pEvt->m_iParameter) {
      case SP_DASHER_FONT:
        m_pScreen->SetFont(m_pDasherInterface->GetStringParameter(SP_DASHER_FONT));
        break;
  /*  case BP_SOCKET_INPUT_ENABLE:
      OutputDebugString(TEXT("Processing BP_SOCKET_INPUT_ENABLE change\n"));
      if(GetBoolParameter(BP_SOCKET_INPUT_ENABLE)) {
        if(!m_pSocketInput->isListening()) {
	        m_pSocketInput->StartListening();
        } 
        m_pDasherInterface->SetInput(1);
      } 
      else {
        if(m_pSocketInput != NULL) {
	        m_pSocketInput->StopListening();
        }
        m_pDasherInterface->SetInput(0);
      }
      break;*/
    default:
      break;
    }
  }
}
