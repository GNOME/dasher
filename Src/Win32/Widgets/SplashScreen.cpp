// SplashScreen.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "SplashScreen.h"

/////////////////////////////////////////////////////////////////////////////

LRESULT CSplash::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
	return TmpWndProc(Window, message, wParam, lParam);
}


LRESULT CALLBACK CSplash::TmpWndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	
	switch(message)
	{
	case WM_CREATE:
		return 0;
		break;
	case WM_PAINT:
		hdc = BeginPaint(Window, &ps);
		RECT rt;
		GetClientRect(Window, &rt);
		FillRect(hdc, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		DrawText(hdc, TEXT("Dasher Initializing - Please Wait"),-1, &rt, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
		EndPaint(Window, &ps);
		return 0;
		break;
	case WM_SIZE:
		// Center & size window
		int Width,Height;
		Width= 300;
		Height= 200;
		MoveWindow(Window, GetSystemMetrics(SM_CXSCREEN)/2-Width/2,
			GetSystemMetrics(SM_CYSCREEN)/2-Height/2, Width, Height, TRUE);
		return 0;
		
	case WM_LBUTTONDOWN:
		SendMessage(Window, WM_CLOSE, 0, 0);
		return 0;
	}
	return DefWindowProc(Window, message, wParam, lParam);
}


CSplash::CSplash(HWND Parent)
{
	WNDCLASSEX  wndclass;
	
	memset(&wndclass, 0, sizeof(WNDCLASSEX));
	wndclass.cbSize         = sizeof(wndclass);
	wndclass.lpfnWndProc    = TmpWndProc;
	wndclass.hInstance      = WinHelper::hInstApp;
	wndclass.hbrBackground  = (HBRUSH) GetStockObject(NULL_BRUSH);
	wndclass.lpszClassName  = TEXT("Splash");
	RegisterClassEx (&wndclass);
	m_hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		TEXT("Splash"), TEXT("Splash"), WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, Parent, NULL,
		WinHelper::hInstApp, NULL);
	
	WinWrapMap::add(m_hwnd, this);
	UpdateWindow(m_hwnd);
}