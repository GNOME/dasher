// Canvas.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////



#include "Canvas.h"
using namespace Dasher;

CCanvas::CCanvas(HWND Parent, Dasher::CDasherWidgetInterface* WI, Dasher::CDasherAppInterface* AI, CEdit* EB)
	: dwThreadID(0), m_DasherWidgetInterface(WI), m_DasherAppInterface(AI),
	m_DasherEditBox(EB), imousex(0), imousey(0), Parent(Parent), buttonnum(0)
{

#ifndef _WIN32_WCE
	WNDCLASSEX canvasclass;
	canvasclass.cbSize = sizeof( WNDCLASSEX );
	GetClassInfoEx(NULL,TEXT("STATIC"),&canvasclass);
	canvasclass.lpszClassName=TEXT("CANVAS");
	canvasclass.hCursor=LoadCursor(NULL,IDC_CROSS);
	canvasclass.style=canvasclass.style|WS_EX_TOOLWINDOW;
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
	SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);
	ShowWindow(m_hwnd,SW_SHOW);
	
	Screen = new CScreen(m_hwnd, 300, 300);
	m_DasherAppInterface->ChangeScreen(Screen);

	for (int i=0; i<18; i++) {
		keycoords[i]=0;
	}
	yscaling=0;
	running = 0;
}


CCanvas::~CCanvas()
{
	DestroyWindow(m_hwnd);
}


void CCanvas::Move(int x, int y, int Width, int Height)
{
	MoveWindow(m_hwnd, x, y, Width, Height, TRUE);
}


void CCanvas::Paint()
{
	InvalidateRect(m_hwnd,NULL, FALSE);
	UpdateWindow(m_hwnd);
}


LRESULT CCanvas::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
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
		SetFocus(Window);
		if (startonleft==false) {
			return 0;
		}
		if (running==0) {
			// if dasher is idle
			
			// capture the mouse
			SetCapture(Window);
			
			running=1;
			m_DasherWidgetInterface->Unpause(GetTickCount());
			
			// update the mouse coords
			imousex=LOWORD(lParam);
			imousey=HIWORD(lParam);
			
			ResumeThread( hThreadl );
			
		} else {
			// if dasher is running			
			m_DasherWidgetInterface->PauseAt(imousex, imousey);
			m_DasherEditBox->speak();
			running=0;
			ReleaseCapture();

			SuspendThread( hThreadl );
		}
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
		Screen->SetNextHDC(hdc);
		m_DasherWidgetInterface->Redraw();
		EndPaint(Window, &ps);
		
		return 0;
		break;
	}
	case WM_SIZE:
		if (Screen!=0)
			delete Screen;
		Screen = new CScreen(m_hwnd, LOWORD(lParam), HIWORD(lParam));
		m_DasherAppInterface->ChangeScreen(Screen);
		InvalidateRect(Window, NULL, FALSE);
		break;

	case WM_TIMER:
		if (running==0)
			return 0;

		POINT mousepos;		
		GetCursorPos(&mousepos);

		if (windowpause==true) {
			RECT windowrect;
			GetWindowRect(m_hwnd, &windowrect);
			if (mousepos.y>windowrect.bottom || mousepos.y<windowrect.top || mousepos.x >windowrect.right || mousepos.x < windowrect.left)
				return 0;
		}

		ScreenToClient(Window,&mousepos);
		imousey=mousepos.y;
		imousex=mousepos.x;

		if (yscaling!=0) {
			float scalefactor=Screen->GetHeight()/yscaling;
			imousey-=Screen->GetHeight()/2;
			imousey*=scalefactor;
			imousey+=Screen->GetHeight()/2;
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
		if (running==0) {
			SetCapture(m_hwnd);		
			running=1;
			m_DasherWidgetInterface->Unpause(GetTickCount());
		} else {
			m_DasherWidgetInterface->PauseAt(0,0);
			running=0;
			m_DasherEditBox->speak();
			ReleaseCapture();
		}
	}
}

void CCanvas::centrecursor() {
	POINT mousepos;
	mousepos.x=Screen->GetWidth()/2;
	mousepos.y=Screen->GetHeight()/2;
	ClientToScreen(m_hwnd,&mousepos);
	SetCursorPos(mousepos.x,mousepos.y);
};