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
	WNDCLASSEX canvasclass;

	canvasclass.cbSize = sizeof( WNDCLASSEX );

	GetClassInfoEx(NULL,TEXT("STATIC"),&canvasclass);

	canvasclass.lpszClassName=TEXT("CANVAS");
	canvasclass.hCursor=LoadCursor(NULL,IDC_CROSS);

	if (RegisterClassEx(&canvasclass)==0)
		exit(0);

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
		if (startonspace == false) {
			return 0;
		}
		if (wParam != VkKeyScan(' ')) {
				return 0;
		}
		if (running==0) {
			SetCapture(Window);

			running=1;
			m_DasherWidgetInterface->Unpause(GetTickCount());
			ResumeThread( hThreadl );
		} else {
			m_DasherWidgetInterface->PauseAt(0,0);
			running=0;
			ReleaseCapture();

			SuspendThread( hThreadl );
		}
		return 0;
		break;
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
		if (imousey<0 || imousey>30000)
			imousey=0;
		if (imousex>30000)
			imousex=0;
		m_DasherWidgetInterface->TapOn(imousex, imousey, GetTickCount());
		break;
	default:
		break;
	}
	return DefWindowProc(Window, message, wParam, lParam);
}
