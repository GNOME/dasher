// Canvas.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "Canvas.h"

#include "Edit.h"

using namespace Dasher;

CCanvas::CCanvas(HWND Parent, Dasher::CDasherWidgetInterface* WI, Dasher::CDasherAppInterface* AI, CEdit* EB)
	: m_DasherWidgetInterface(WI), m_DasherAppInterface(AI),
	m_DasherEditBox(EB), imousex(0), imousey(0), Parent(Parent), buttonnum(0), mousepostime(0)
{

#ifndef _WIN32_WCE
	WNDCLASSEX canvasclass;
	canvasclass.cbSize = sizeof( WNDCLASSEX );
	GetClassInfoEx(NULL,TEXT("STATIC"),&canvasclass);
	canvasclass.lpszClassName=TEXT("CANVAS");
	canvasclass.lpfnWndProc= WinWrapMap::WndProc;
	canvasclass.hCursor=LoadCursor(NULL,IDC_CROSS);
//	canvasclass.style=canvasclass.style|WS_EX_TOOLWINDOW;
	canvasclass.style=CS_OWNDC;	// give this window its own Private DC
	if (RegisterClassEx(&canvasclass)==0)
		exit(0);
#else
	WNDCLASS canvasclass;
	GetClassInfo(NULL,TEXT("STATIC"),&canvasclass);
	canvasclass.lpszClassName=TEXT("CANVAS");
	
#endif


	m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("CANVAS"), NULL,
		WS_CHILD | WS_VISIBLE,
		0,0,0,0, Parent, NULL, WinHelper::hInstApp, NULL );

 	
	WinWrapMap::add(m_hwnd, this);
//	SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);
	ShowWindow(m_hwnd,SW_SHOW);
	
	m_hdc = GetDC(m_hwnd);
	HDC hdc2 = GetDC(m_hwnd);
	HDC hdc3 = GetDC(m_hwnd);

	m_pScreen = new CScreen(m_hdc, 300, 300);
	//ReleaseDC(m_hwnd,m_hDC);
	m_DasherAppInterface->ChangeScreen(m_pScreen);

	for (int i=0; i<18; i++) {
		keycoords[i]=0;
	}
	running = 0;
	previoustime=GetTickCount();
	direction=0;
}


CCanvas::~CCanvas()
{
	int iRC = ReleaseDC(m_hwnd,m_hdc);
	if (!iRC)
	{
		LPVOID lpMsgBuf;
FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
);
// Process any inserts in lpMsgBuf.
// ...
// Display the string.
MessageBox( NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION );
// Free the buffer.
LocalFree( lpMsgBuf );


	}


	delete m_pScreen;
}


void CCanvas::Move(int x, int y, int Width, int Height)
{
	MoveWindow(m_hwnd, x, y, Width, 
		Height, TRUE);
}


void CCanvas::Paint()
{
	InvalidateRect(m_hwnd,NULL, FALSE);
	UpdateWindow(m_hwnd);
}


LRESULT CCanvas::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
	
    TCHAR tmpAutoOffset[25];
	TCHAR tmpOneButton[25];

	switch (message) {


	case WM_COMMAND:
		SendMessage(Parent, message, wParam, lParam);
		return 0;
		break;

	case WM_KEYDOWN:
		switch (wParam) {
			if (keycontrol == true) {
		case VK_UP:
			if (forward==true) {
				buttonnum++;
				if (buttonnum==9) {
					buttonnum=0;
				}
				while (keycoords[buttonnum*2]==NULL) {
					buttonnum++;
					if (buttonnum==9) {
						buttonnum=0;
					}
				}
				m_DasherWidgetInterface->DrawGoTo(keycoords[buttonnum*2],keycoords[buttonnum*2+1]);
			} else {
				m_DasherWidgetInterface->GoTo(keycoords[0], keycoords[1]);
			}
			return 0;
			break;
		case VK_DOWN:	
			if (backward==true) {
				buttonnum--;
				if (buttonnum==-1) {
					buttonnum=8;
				}
				while (keycoords[buttonnum*2]==NULL) {
					buttonnum--;
					if (buttonnum==-1) {
						buttonnum=8;
					}
				}
				m_DasherWidgetInterface->DrawGoTo(keycoords[buttonnum*2],keycoords[buttonnum*2+1]);
			} else {
				m_DasherWidgetInterface->GoTo(keycoords[2], keycoords[3]);
			}
			return 0;
			break;
		case VK_LEFT:
			if (select==true) {
				m_DasherWidgetInterface->GoTo(keycoords[buttonnum*2],keycoords[buttonnum*2+1]);
				m_DasherWidgetInterface->DrawGoTo(keycoords[buttonnum*2],keycoords[buttonnum*2+1]);
			} else {
				m_DasherWidgetInterface->GoTo(keycoords[4],keycoords[5]);
			}
			return 0;
			break;
		case VK_RIGHT:
			m_DasherWidgetInterface->GoTo(keycoords[6],keycoords[7]);
			return 0;
			break;
			}
		case VK_SPACE:
			startspace();
			return 0;
			break;
		case VK_F9:
			wsprintf(tmpOneButton, TEXT("yOneButton: %d"), m_DasherAppInterface->GetOneButton());
			MessageBox(Window, tmpOneButton, NULL, 1);
			return 0;
			break;
		case VK_F11:
			wsprintf(tmpAutoOffset, TEXT("yAutoValue: %d"), m_DasherAppInterface->GetAutoOffset());
			MessageBox(Window, tmpAutoOffset, NULL, 1);
			return 0;
			break;
		case VK_F12:
			centrecursor();
			return 0;
			break;
		default:
			return 0;
			break;
		}
	case WM_LBUTTONDBLCLK:
		// fall through
	case WM_LBUTTONDOWN:
		startturbo=GetTickCount();
		SetFocus(Window);

		lbuttonheld=1;
		lastlbutton = GetTickCount();
		direction = !direction;
		enabletime=1;
		if (startonleft==false) {
			return 0;
		}
		StartStop();
		return 0;
		break;
	case WM_LBUTTONUP:
		endturbo = GetTickCount();

		if (endturbo-startturbo > 1) {
			
			TCHAR deb[80];
			wsprintf(deb,TEXT("start: %d\nend: %d\nduration: %d"), startturbo, endturbo, endturbo-startturbo);
			OutputDebugString(deb);
		}
		lbuttonheld=0;
		return 0;
		break;
	case WM_MOUSEMOVE:
		imousex=LOWORD(lParam);
		imousey=HIWORD(lParam);
		return 0;
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(Window, &ps);
	
///		Screen->SetNextHDC(hdc);
		m_DasherWidgetInterface->Redraw();
		if (firstwindow==true) {
			m_pScreen->DrawMousePosBox(0);
		} else if (secondwindow==true) {
			m_pScreen->DrawMousePosBox(1);
		}
		EndPaint(Window, &ps);
	
		return 0;
		break;
	}
	case WM_SIZE:
		if (m_pScreen!=0)
			delete m_pScreen;
		m_pScreen = new CScreen(m_hdc, LOWORD(lParam), HIWORD(lParam));
		m_DasherAppInterface->ChangeScreen(m_pScreen);
		InvalidateRect(Window, NULL, FALSE);
		break;

	case WM_TIMER:
		POINT mousepos;		
		GetCursorPos(&mousepos);

		if (running==0) {
			ScreenToClient(Window,&mousepos);			
			if (mouseposstart==true) { // configuration option
				if (firstwindow!=true && secondwindow!=true) {			
					m_DasherWidgetInterface->Redraw();
					m_pScreen->DrawMousePosBox(0);
					firstwindow=true;
				}
				if (mousepos.y>m_pScreen->GetHeight()/2-mouseposdist-50 && mousepos.y<m_pScreen->GetHeight()/2-mouseposdist+50 && firstwindow==true) {
					// Mouse is in the top box
					if (mousepostime==0) {
						mousepostime=GetTickCount();
					} else if ((GetTickCount()-mousepostime)>2000) {
						firstwindow=false;
						secondwindow=true;
						mousepostime=0;
						m_DasherWidgetInterface->Redraw();
						m_pScreen->DrawMousePosBox(1);
					}
				} else if (firstwindow==true) {
					mousepostime=0;
				}
				if (secondwindow==true) {
					if (mousepos.y< m_pScreen->GetHeight()/2+mouseposdist+50 && mousepos.y>(m_pScreen->GetHeight()/2+mouseposdist-50)) {
						// In second window
						if (mousepostime==0) {
							mousepostime=GetTickCount();
						} else if ((GetTickCount()-mousepostime)>2000) {
							StartStop();
						}
					} else if (mousepostime>0) {
						secondwindow=false;
						firstwindow=true;
						m_DasherWidgetInterface->Redraw();
						m_pScreen->DrawMousePosBox(0);
						mousepostime=0;
					}
				}
			}
			imousey=mousepos.y;

			imousex=mousepos.x;

			if (oned==true) {
				double scalefactor;
				if (yscaling==0) {
					scalefactor=2.0;
				} else {
					scalefactor=m_pScreen->GetHeight()/yscaling;
				}
				imousey-=m_pScreen->GetHeight()/2;
				imousey*=scalefactor;
				imousey+=m_pScreen->GetHeight()/2;
			}



			if ((GetTickCount()-previoustime)>200) {
				m_DasherWidgetInterface->DrawMousePos(imousex,imousey);
				if (firstwindow==true) {
					m_pScreen->DrawMousePosBox(0);
				} else if (secondwindow==true) {
					m_pScreen->DrawMousePosBox(1);
				}
				previoustime=GetTickCount();
			}

			return 0;
		}

		// One-button mode.
        if (direction==TRUE) {
			if (lbuttonheld && (GetTickCount()-lastlbutton) > 250) {
//				double BitRate = m_DasherAppInterface->GetMaxBitRate();
			//	TCHAR deb[80];
			//	wsprintf(deb,TEXT("bitrate: %d\n"), BitRate);
			//	OutputDebugString(deb);
				m_DasherAppInterface->SetOneButton(125);
			}
			else {
				m_DasherAppInterface->SetOneButton(50);
			}
        }
        if (direction==FALSE) {
			if (lbuttonheld && (GetTickCount()-lastlbutton) > 250) {
				m_DasherAppInterface->SetOneButton(-125);
			}
			else {
				m_DasherAppInterface->SetOneButton(-50);
			}
        }

		if (windowpause==true) {
			RECT windowrect;

			GetWindowRect(m_hwnd, &windowrect);
			if (mousepos.y>windowrect.bottom || mousepos.y<windowrect.top || mousepos.x >windowrect.right || mousepos.x < windowrect.left)
				return 0;
		}

		ScreenToClient(Window,&mousepos);			
		imousey=mousepos.y;
		imousex=mousepos.x;
	
		if (oned==true) {
			double scalefactor;
			if (yscaling==0) {
				scalefactor=2;
			} else {
				scalefactor=m_pScreen->GetHeight()/yscaling;
			}	
			imousey-=m_pScreen->GetHeight()/2;
			imousey*=scalefactor;
			imousey+=m_pScreen->GetHeight()/2;
		}
		m_DasherWidgetInterface->TapOn(imousex, imousey, GetTickCount());
		break;
	default:
		break;
	}
	return DefWindowProc(Window, message, wParam, lParam);
}

void CCanvas::startspace()
{
	if (startonspace == false) {
		return;
	} else {
		StartStop();
	}
}

void CCanvas::centrecursor() 
{
	POINT mousepos;
	mousepos.x=m_pScreen->GetWidth()/2;

	mousepos.y=m_pScreen->GetHeight()/2;

	ClientToScreen(m_hwnd,&mousepos);

	SetCursorPos(mousepos.x,mousepos.y);

};



void CCanvas::MousePosStart(bool Value) 
{
	if (Value==false) {
		firstwindow=false;
		secondwindow=false;
	}
	mouseposstart=Value;
}

/////////////////////////////////////////////////////////////////////////////

void CCanvas::StartStop() {

	if (running==0) {
		SetCapture(m_hwnd);		
		running=1;
		m_DasherWidgetInterface->Unpause(GetTickCount());
		firstwindow=false;
		secondwindow=false;
		mousepostime=0;

	} else {
		m_DasherWidgetInterface->PauseAt(0,0);
		running=0;
		if (speakonstop==true) {
			m_DasherEditBox->speak(2);
		}
		ReleaseCapture();
	}
}

/////////////////////////////////////////////////////////////////////////////

