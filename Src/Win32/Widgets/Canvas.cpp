#include "Canvas.h"
using namespace Dasher;

const UINT MY_TIMER=WM_USER+1;
bool running;


DWORD WINAPI Thread2 (PVOID pArg)
{
	// send MY_TIMER messages to the main thread
	// the main thread responds by advancing Dasher one frame
	do {
		SendMessage((HWND) pArg,MY_TIMER,0,0);
		Sleep(20); // Don't bother trying to get more than 50fps.
	} while (running);
	return 0x15;
}


CCanvas::CCanvas(HWND Parent, Dasher::CDasherWidgetInterface* WI, Dasher::CDasherAppInterface* AI)
	: dwThreadID(0), m_DasherWidgetInterface(WI), m_DasherAppInterface(AI),
	imousex(0), imousey(0), Parent(Parent)
{
	m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("STATIC"), NULL,
		WS_CHILD | WS_VISIBLE,
		0,0,0,0, Parent, NULL, WinHelper::hInstApp, NULL );
	
	WinWrapMap::add(m_hwnd, this);
	SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);
	ShowWindow(m_hwnd,SW_SHOW);
	
	Screen = new CScreen(m_hwnd, 300, 300);
	m_DasherAppInterface->ChangeScreen(Screen);
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
	case WM_LBUTTONDOWN:
		SetFocus(Window);
		
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
			
			// create a 2nd thread to send MY_TIMER messages to main thread
			hThreadl= CreateThread(NULL,0,Thread2,(PVOID)m_hwnd,0,&dwThreadID);
			CloseHandle(hThreadl);
			SetThreadPriority(hThreadl,THREAD_PRIORITY_BELOW_NORMAL);
			
		} else {
			// if dasher is running
			
			m_DasherWidgetInterface->PauseAt(imousex, imousey);
			//dasherview->Flush_at(imousex,imousey);
			//theeditbox->write_to_file();
			running=0;
			ReleaseCapture();
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
	case MY_TIMER: {
		if (imousey<0 || imousey>30000)
			imousey=0;
		if (imousex>30000)
			imousex=0;
		//HDC hdc = GetDC(Window);
		//Screen->SetHDC(hdc);
		m_DasherWidgetInterface->TapOn(imousex, imousey, GetTickCount());
		//ReleaseDC(Window,hdc);
		return 0;
		break;
	}
	default:
		break;
	}
	
	return DefWindowProc(Window, message, wParam, lParam);
}
