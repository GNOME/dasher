// Canvas.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////



#include "Canvas.h"
using namespace Dasher;

CCanvas::CCanvas(HWND Parent, Dasher::CDasherWidgetInterface* WI, Dasher::CDasherAppInterface* AI)
	: dwThreadID(0), m_DasherWidgetInterface(WI), m_DasherAppInterface(AI),
	imousex(0), imousey(0), Parent(Parent)
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

		int i;

		switch (wParam) {

			if (keycontrol == true) {

				case VK_UP:

					m_DasherWidgetInterface->Unpause(GetTickCount());

					GetClientRect(Window, &coords);

					for (i=0; i<300; i+=10) {

						m_DasherWidgetInterface->TapOn(int(0.70*coords.right), int(0.20*coords.bottom), GetTickCount()+i);

					}

				 m_DasherWidgetInterface->PauseAt(150,150);

				 return 0;

				 break;

				case VK_DOWN:

					GetClientRect(Window, &coords);

					m_DasherWidgetInterface->Unpause(GetTickCount());

				    for (i=0; i<300; i+=10) {

						m_DasherWidgetInterface->TapOn(int(0.70*coords.right), int(0.790*coords.bottom), GetTickCount()+i);

				    }

					m_DasherWidgetInterface->PauseAt(150,150);

					return 0;

					break;

				case VK_LEFT:

					GetClientRect(Window, &coords);

					m_DasherWidgetInterface->Unpause(GetTickCount());

					for (i=0; i<300; i+=10) {

						m_DasherWidgetInterface->TapOn(int(0.25*coords.right),int(0.47*coords.bottom), GetTickCount()+i);

					}

					m_DasherWidgetInterface->PauseAt(150,150);

					return 0;

					break;

				}

			case VK_SPACE:

				if (startonspace == false) {

					return 0;

					break;

				} else {

					if (running==0) {

						SetCapture(Window);

						running=1;

						m_DasherWidgetInterface->Unpause(GetTickCount());

					} else {

						m_DasherWidgetInterface->PauseAt(0,0);

						running=0;

						ReleaseCapture();

					}

					return 0;

					break;

				}

			default:

			return 0;

			break;

		}

	case WM_LBUTTONDBLCLK:
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
			// theeditbox->unflush();
			// dashermodel->Reset_framerate();
			m_DasherWidgetInterface->Unpause(GetTickCount());
			
			// update the mouse coords
			imousex=LOWORD(lParam);
			imousey=HIWORD(lParam);
			
			ResumeThread( hThreadl );
			
		} else {
			// if dasher is running
			
			m_DasherWidgetInterface->PauseAt(imousex, imousey);
			//dasherview->Flush_at(imousex,imousey);
			//theeditbox->write_to_file();
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

//		if (imousey<-30000 || imousey>30000)
//			imousey=-30000;
//		if (imousex>30000)
//			imousex=0;
		m_DasherWidgetInterface->TapOn(imousex, imousey, GetTickCount());
		break;

	default:

		break;

	}
	return DefWindowProc(Window, message, wParam, lParam);
}
