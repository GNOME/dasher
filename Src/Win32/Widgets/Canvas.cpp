// Canvas.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "Canvas.h"
#include "Edit.h"
#include "../Dasher.h"
#include "../DasherInterface.h"
#include "../DasherMouseInput.h"
#include "../Sockets/SocketInput.h"


#define PRESSED		0x8000
#define REPEAT		0x40000000
using namespace Dasher;

#include "../TabletPC/SystemInfo.h"
#include "Screen.h"

/////////////////////////////////////////////////////////////////////////////

CCanvas::CCanvas(CDasherInterface *DI)
	:m_pDasherInterface(DI), imousex(0), imousey(0), buttonnum(0), mousepostime(0) ,
	m_dwTicksLastEvent(0), m_bButtonDown(false), m_pScreen(0),
	CDasherComponent(DI->GetEventHandler(), DI->CreateSettingsStore())

{

#ifndef _WIN32_WCE

#else
  WNDCLASS canvasclass;
  GetClassInfo(NULL, TEXT("STATIC"), &canvasclass);
  canvasclass.lpszClassName = TEXT("CANVAS");

#endif
}

/////////////////////////////////////////////////////////////////////////////

HWND CCanvas::Create(HWND hParent)
{
	HWND hWnd = CWindowImpl<CCanvas>::Create(hParent, NULL, NULL, WS_CHILD | WS_VISIBLE ,WS_EX_CLIENTEDGE);
	
	m_hdc = GetDC();
	HDC hdc2 = GetDC();
	HDC hdc3 = GetDC();

	// Create input device objects
	// NB We create the SocketInput object now, even if socket input is not enabled, because
	// we can't safely create it later in response to a parameter change event (because it itself
	// needs to register an event listener when it constructs itself).
	m_pSocketInput = new CSocketInput(m_pDasherInterface->GetEventHandler(), m_pDasherInterface->CreateSettingsStore()); // CreateSettingsStore only creates a new one if there isn't one there already
	m_pMouseInput = new CDasherMouseInput(hWnd);

	if(m_pDasherInterface->GetBoolParameter(BP_SOCKET_INPUT_ENABLE)) 
	{
		m_pSocketInput->StartListening();
		m_pDasherInterface->SetInput(m_pSocketInput);
	}
	else {
		m_pDasherInterface->SetInput(m_pMouseInput);
	}


	m_pScreen = new CScreen(m_hdc, 300, 300);
	//ReleaseDC(m_hwnd,m_hDC);
	m_pDasherInterface->ChangeScreen(m_pScreen);

	for(int i = 0; i < 18; i++) 
	{
		keycoords[i] = 0;
	}
	running = 0;
	previoustime = GetTickCount();
	direction = 0;

	return hWnd;
}

/////////////////////////////////////////////////////////////////////////////

void CCanvas::SetScreenInterface(CDasherInterface * dasherinterface)
{
    m_pScreen->SetInterface(dasherinterface);
}
  
/////////////////////////////////////////////////////////////////////////////

LRESULT CCanvas::OnCreate(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	// If we're a tablet, initialize the event-generator
	if (IsTabletPC())
	{
		HRESULT h = m_CursorInRange.Initialize(m_hWnd);
		if (! SUCCEEDED(h))
		{
			// detected tablet, but cant initialize the event-generator
			return -1;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CCanvas::OnDestroy(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int iRC = ReleaseDC(m_hdc);
	if(!iRC) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
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
  delete m_pScreen;
  if(m_pMouseInput != NULL) {
    delete m_pMouseInput;
  }
  if(m_pSocketInput != NULL) {
    delete m_pSocketInput;
  }
}

void CCanvas::Move(int x, int y, int Width, int Height) 
{
  MoveWindow(x, y, Width, Height, TRUE);
}

void CCanvas::Paint() 
{
  InvalidateRect(NULL, FALSE);
  UpdateWindow();
}


LRESULT CCanvas::OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE; 
	SendMessage(GetParent(), message, wParam, lParam);
    return 0;
}

LRESULT CCanvas::OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE; 
	SendMessage(GetParent(), WM_DASHER_FOCUS, 0, (LPARAM)&m_hWnd);
    return 0;
}

LRESULT CCanvas::OnPaint(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PAINTSTRUCT ps;
    BeginPaint(&ps);

    ///             Screen->SetNextHDC(hdc);
    //m_pDasherInterface->Redraw();
    m_pScreen->Display();
    //      if (firstwindow==true) 
    //      {
    //              m_pScreen->DrawMousePosBox(0,m_iMousePosDist);
    //      } 
    //      else if (secondwindow==true) 
    //      {
    //              m_pScreen->DrawMousePosBox(1,m_iMousePosDist);
    //      }
    EndPaint(&ps);
	bHandled = TRUE;
    return 0;
}


LRESULT CCanvas::OnKeyUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	switch(wParam) 
	{
	case VK_SHIFT:
		if(GetKeyState(VK_CONTROL) & PRESSED)
			m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 25);
		else
			m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 100);
		return 0;
	case VK_CONTROL:
		if(GetKeyState(VK_SHIFT) & PRESSED)
			m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 175);
		else
			m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 100);

		return 0;
	default:
		return 0;		  
	}

}

LRESULT CCanvas::OnKeyDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
#ifdef _DEBUG
  TCHAR tmpAutoOffset[128];
#endif


	bHandled = TRUE;

	switch (wParam) 
	{
		if(m_pDasherInterface->GetBoolParameter(BP_KEY_CONTROL) == true)
		{
	case VK_UP:
		if(forward == true) 
		{
			buttonnum++;
			if(buttonnum == 9) 
			{
				buttonnum = 0;
			}
			while(keycoords[buttonnum * 2] == NULL) 
			{
				buttonnum++;
				if(buttonnum == 9) 
				{
					buttonnum = 0;
				}
			}
			m_pDasherInterface->DrawGoTo(keycoords[buttonnum * 2], keycoords[buttonnum * 2 + 1]);
		}
		else 
		{
			m_pDasherInterface->GoTo(keycoords[0], keycoords[1]);
		}
		return 0;
		break;
	case VK_DOWN:
		if(backward == true) 
		{
			buttonnum--;
			if(buttonnum == -1) 
			{
				buttonnum = 8;
			}
			while(keycoords[buttonnum * 2] == NULL) 
			{
				buttonnum--;
				if(buttonnum == -1) 
				{
					buttonnum = 8;
				}
			}
			m_pDasherInterface->DrawGoTo(keycoords[buttonnum * 2], keycoords[buttonnum * 2 + 1]);
		}
		else 
		{
			m_pDasherInterface->GoTo(keycoords[2], keycoords[3]);
		}
		return 0;
	case VK_LEFT:
		if(select == true) 
		{
			m_pDasherInterface->GoTo(keycoords[buttonnum * 2], keycoords[buttonnum * 2 + 1]);
			m_pDasherInterface->DrawGoTo(keycoords[buttonnum * 2], keycoords[buttonnum * 2 + 1]);
		}
		else 
		{
			m_pDasherInterface->GoTo(keycoords[4], keycoords[5]);
		}
		return 0;
	case VK_RIGHT:
		m_pDasherInterface->GoTo(keycoords[6], keycoords[7]);
		return 0;
		break;
		}      // end if key control
	case VK_SPACE:
		startspace();
		return 0;
#ifdef _DEBUG
	case VK_F11:
		wsprintf(tmpAutoOffset, TEXT("yAutoValue: %d"), m_pDasherInterface->GetAutoOffset());
		MessageBox(tmpAutoOffset, TEXT("Auto-offset Value"), MB_OK);
		return 0;
#endif
	case VK_F12:
		centrecursor();
		return 0;
	case VK_SHIFT:

		if(lParam ^ REPEAT) // ignore auto-repeat
			m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 175);

		return 0;

	case VK_CONTROL:
		if(lParam ^ REPEAT) // ignore auto-repeat
			m_pDasherInterface->SetLongParameter(LP_BOOSTFACTOR, 25);
		return 0;
  case 0x41:
    m_pDasherInterface->KeyDown(1);
    return 0;
  case 0x53:
    m_pDasherInterface->KeyDown(2);
    return 0;
  case 0x57:
    m_pDasherInterface->KeyDown(3);
    return 0;
  case 0x58:
    m_pDasherInterface->KeyDown(4);
    return 0;
	default:
		return 0;
	}
}

LRESULT CCanvas::OnLButtonDblClk(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
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

	OutputDebugString(TEXT("Canvas::LButtonDown\n"));

	//   startturbo = GetTickCount();
	
	//SetFocus();
	
	/*
	lbuttonheld = 1;
	lastlbutton = GetTickCount();
	direction = !direction;
	enabletime = 1;
	if(startonleft == false) {
	return 0;
	}
	StartStop();*/

	LPARAM lp = GetMessageExtraInfo();
	if (lp == 0xFF515702)
		return 0;

	if ( m_pDasherInterface->GetBoolParameter(BP_START_MOUSE) ) 
	{
		if (m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED))
			m_pDasherInterface->Unpause(GetTickCount());
		else
			m_pDasherInterface->PauseAt(0, 0);
	}
//	else if ( m_pDasherInterface->GetBoolParameter(BP_START_STYLUS)  ) 
//	{
//		// DJW - for the time being we only do stylus mode if not BP_START_MOUSE 
//
//		if ( m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED) )
//			m_pDasherInterface->Unpause(GetTickCount());
//	}

	m_bButtonDown = true;
	return 0;

}

LRESULT CCanvas::OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	endturbo = GetTickCount();

	if(endturbo - startturbo > 1) 
	{
		TCHAR deb[80];
		wsprintf(deb, TEXT("start: %d\nend: %d\nduration: %d"), startturbo, endturbo, endturbo - startturbo);
		OutputDebugString(deb);
	}
	lbuttonheld = 0;

	m_bButtonDown = false;

	return 0;

}

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

	imousex = LOWORD(lParam);
	imousey = HIWORD(lParam);
	m_dwTicksLastEvent	= GetTickCount();
//	if( m_pDasherInterface->GetBoolParameter(BP_START_STYLUS)  ) 
//	{
//		if( m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED) )
//			m_pDasherInterface->Unpause(GetTickCount());
//	}

	return 0;
}

LRESULT CCanvas::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	if(m_pScreen != 0)
	{
		delete m_pScreen;
		m_pScreen = 0;
	}
	if (LOWORD(lParam)>0 && HIWORD(lParam) >0)
	{
		m_pScreen = new CScreen(m_hdc, LOWORD(lParam), HIWORD(lParam));
		m_pDasherInterface->ChangeScreen(m_pScreen);
		InvalidateRect( NULL, FALSE);
	}
	return 0;
}

/*LRESULT CCanvas::OnTimer(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) */

void CCanvas::DoFrame()
{
	POINT mousepos2;
	GetCursorPos(&mousepos2);

	ScreenToClient(&mousepos2);

	POINT mousepos;
	GetCursorPos(&mousepos);

	//  if(running == 0) {
	//    ScreenToClient(m_hwnd, &mousepos);
	//    if(m_pDasherInterface->GetBoolParameter(BP_MOUSEPOS_MODE) == true) // configuration option
	//    {
	//      //                      DASHER_TRACEOUTPUT("first:%d second:%d %d\n",firstwindow,secondwindow,m_iMousePosDist);
	//      if(firstwindow != true && secondwindow != true) {
	//        m_pDasherInterface->SetLongParameter(LP_MOUSE_POS_BOX, 1);
	//        firstwindow = true;
	//      }
	//      int mouseDist = m_pDasherInterface->GetLongParameter(LP_MOUSEPOSDIST);
	//      if(mousepos.y > m_pScreen->GetHeight() / 2 - mouseDist - 50 && mousepos.y < m_pScreen->GetHeight() / 2 - mouseDist + 50 && firstwindow == true) {
	//        // Mouse is in the top box
	//        if(mousepostime == 0) {
	//          mousepostime = GetTickCount();
	//        }
	//        else if((GetTickCount() - mousepostime) > 2000) {
	//          firstwindow = false;
	//          secondwindow = true;
	//          mousepostime = 0;
	//          m_pDasherInterface->SetLongParameter(LP_MOUSE_POS_BOX, 2);
	//        }
	//      }
	//      else if(firstwindow == true) {
	//        mousepostime = 0;
	//      }
	//      if(secondwindow == true) {
	//        if(mousepos.y < m_pScreen->GetHeight() / 2 + mouseDist + 50 && mousepos.y > (m_pScreen->GetHeight() / 2 + mouseDist - 50)) {
	//          // In second window
	//          if(mousepostime == 0) {
	//            mousepostime = GetTickCount();
	//          }
	//          else if((GetTickCount() - mousepostime) > 2000) {
	//            m_pDasherInterface->SetLongParameter(LP_MOUSE_POS_BOX, -1);
	//            StartStop();
	//          }
	//        }
	//        else if(mousepostime > 0) {
	//          secondwindow = false;
	//          firstwindow = true;
	//          m_pDasherInterface->SetLongParameter(LP_MOUSE_POS_BOX, 1);
	//          mousepostime = 0;
	//        }
	//      }
	//    }
	//    imousey = mousepos.y;
	//
	//    imousex = mousepos.x;
	//
	//    if(m_pDasherInterface->GetBoolParameter(BP_NUMBER_DIMENSIONS) == true) {
	//      double scalefactor;
	//      if(yscaling == 0) {
	//        scalefactor = 2.0;
	//      }
	//      else {
	//        scalefactor = m_pScreen->GetHeight() / yscaling;
	//      }
	//      imousey -= m_pScreen->GetHeight() / 2;
	//      imousey *= scalefactor;
	//      imousey += m_pScreen->GetHeight() / 2;
	//    }
	//
	//    if((GetTickCount() - previoustime) > 200) {
	//      if(firstwindow == true) {
	//        m_pDasherInterface->SetLongParameter(LP_MOUSE_POS_BOX, 1);
	//      }
	//      else if(secondwindow == true) {
	//        m_pDasherInterface->SetLongParameter(LP_MOUSE_POS_BOX, 2);
	//      }
	//
	//      previoustime = GetTickCount();
	//    }
	//
	//    return 0;
	//  }
	//
	////  if(m_pDasherInterface->GetBoolParameter(BP_WINDOW_PAUSE) == true) {
	//  //  RECT windowrect;
	//
	////    GetWindowRect(m_hwnd, &windowrect);
	// //   if(mousepos.y > windowrect.bottom || mousepos.y < windowrect.top || mousepos.x > windowrect.right || mousepos.x < windowrect.left)
	////      return 0;
	// // }
	//
	//  ScreenToClient(m_hwnd, &mousepos);
	//  imousey = mousepos.y;
	//  imousex = mousepos.x;
	//
	//  if(m_pDasherInterface->GetBoolParameter(BP_NUMBER_DIMENSIONS) == true) {
	//    double scalefactor;
	//    if(yscaling == 0) {
	//      scalefactor = 2;
	//    }
	//    else {
	//      scalefactor = m_pScreen->GetHeight() / yscaling;
	//    }
	//    imousey -= m_pScreen->GetHeight() / 2;
	//    imousey *= scalefactor;
	//    imousey += m_pScreen->GetHeight() / 2;
	//  }

	DWORD dwTicks = GetTickCount();

	// If not paused need to consider stop on idle
	if( !m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED)  ) 
	{
		// only pause if button is not down
		if( !m_bButtonDown && m_pDasherInterface->GetBoolParameter(BP_STOP_IDLE)  ) 
		{
			if (dwTicks - m_dwTicksLastEvent > m_pDasherInterface->GetLongParameter(LP_STOP_IDLETIME) )
			{
				// idle time exceeded
				m_pDasherInterface->PauseAt(0, 0);
			}
		}
	}

	m_pDasherInterface->NewFrame(dwTicks);//TapOn(imousex, imousey, GetTickCount());
}

void CCanvas::startspace() {
   if(m_pDasherInterface->GetBoolParameter(BP_START_SPACE) == false) {
    return;
  }
  else {
    if(m_pDasherInterface->GetBoolParameter(BP_DASHER_PAUSED))
      m_pDasherInterface->Unpause(GetTickCount());
    else
      m_pDasherInterface->PauseAt(0, 0);
  }
}

void CCanvas::centrecursor() {
  POINT mousepos;
  mousepos.x = m_pScreen->GetWidth() / 2;

  mousepos.y = m_pScreen->GetHeight() / 2;

  ClientToScreen( &mousepos);

  SetCursorPos(mousepos.x, mousepos.y);

};

void CCanvas::MousePosStart(bool Value) {
  if(Value == false) {
    firstwindow = false;
    secondwindow = false;
  }
}

/////////////////////////////////////////////////////////////////////////////

void CCanvas::StartStop() {

  if(running == 0) {
    SetCapture();
    running = 1;
    m_pDasherInterface->Unpause(GetTickCount());
    firstwindow = false;
    secondwindow = false;
    mousepostime = 0;

  }
  else {
    m_pDasherInterface->PauseAt(0, 0);
    running = 0;
//              if (speakonstop==true) { // FIXME - reimplement this
//                      m_DasherEditBox->speak(2);
//              }
    ReleaseCapture();
  }
}

/////////////////////////////////////////////////////////////////////////////

// Gets the size of the canvas in screen coordinates.  Need if we
// want to log mouse positions normalized by the size of the
// canvas.
bool CCanvas::GetCanvasSize(int& iTop, int& iLeft, int& iBottom, int& iRight)
{

	RECT sWindowRect;

	if (GetWindowRect( &sWindowRect))
	{
		iTop    = sWindowRect.top;
		iLeft   = sWindowRect.left;
		iBottom = sWindowRect.bottom;
		iRight  = sWindowRect.right;
		return true;
	}
	else
		return false;
}


void CCanvas::HandleEvent(Dasher::CEvent *pEvent) {

  if(pEvent->m_iEventType == 1) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    switch (pEvt->m_iParameter) {
    case BP_SOCKET_INPUT_ENABLE:
      OutputDebugString(TEXT("Processing BP_SOCKET_INPUT_ENABLE change\n"));
      if(GetBoolParameter(BP_SOCKET_INPUT_ENABLE)) {
        if(m_pSocketInput == NULL) { // shouldn't occur
          OutputDebugString(TEXT("Created new SocketInput instance\n"));
	        m_pSocketInput = new CSocketInput(m_pDasherInterface->GetEventHandler(), m_pDasherInterface->CreateSettingsStore()); // CreateSettingsStore only creates a new one if there isn't one there already
        }
        if(!m_pSocketInput->isListening()) {
	        m_pSocketInput->StartListening();
        } 
        m_pDasherInterface->SetInput(m_pSocketInput);
      } 
      else {
        if(m_pMouseInput == NULL) { // shouldn't occur
	        m_pMouseInput = new CDasherMouseInput(m_hWnd);
        }
        if(m_pSocketInput != NULL) {
	        m_pSocketInput->StopListening();
        }
        m_pDasherInterface->SetInput(m_pMouseInput);
      }
      break;
    default:
      break;
    }
  }
}