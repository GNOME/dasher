// Splitter.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "Splitter.h"

// For WinCE
#ifndef MAKEPOINTS
#define MAKEPOINTS(l)   (*((POINTS FAR *) & (l))) 
#endif


CSplitter::CSplitter(HWND Parent, int Pos, CSplitterOwner* NewOwner, bool Visible)
	: SplitStatus(None), m_Pos(Pos), Parent(Parent), Owner(NewOwner), Visible(Visible)
{
	m_Height = GetSystemMetrics(SM_CYSIZEFRAME);
	
	Tstring WndClassName = CreateMyClass();
	
	m_hwnd = CreateWindowEx(0, WndClassName.c_str(), NULL,
		WS_CHILD | WS_VISIBLE,
		0,0,0,0, Parent, NULL, WinHelper::hInstApp, NULL );
	
	WinWrapMap::add(m_hwnd, this);
}


void CSplitter::Move(int Pos, int Width)
{
	m_Pos = Pos;
	MoveWindow(m_hwnd, 0, Pos, Width, m_Height, TRUE);
}


void CSplitter::SetVisible(bool Value)
{
	Visible = Value;
	if (Visible)
		m_Height = GetSystemMetrics(SM_CYSIZEFRAME);
	else
		m_Height = 0;
}


Tstring CSplitter::CreateMyClass()
{
	TCHAR* WndClassName = TEXT("HSplitter");
	
	if (WinWrapMap::Register(WndClassName)) {
		WNDCLASSEX wndclass;
		memset(&wndclass, 0, sizeof(WNDCLASSEX));
		wndclass.cbSize = sizeof(WNDCLASSEX);
		wndclass.style          = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc    = WinWrapMap::WndProc;
		wndclass.hInstance      = WinHelper::hInstApp;
		wndclass.hCursor        = LoadCursor(NULL, IDC_SIZENS);
		wndclass.hbrBackground  = (HBRUSH)(COLOR_ACTIVEBORDER+1); // Must add one to the value we want for some unknown reason
		wndclass.lpszMenuName   = NULL;
		wndclass.lpszClassName  = WndClassName;
		
		RegisterClassEx(&wndclass);
	}
	
	return Tstring(WndClassName);
}


LRESULT CSplitter::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
		{
			SplitStatus = Sizing;
			SetCapture(m_hwnd);
		}
		break;
	case WM_LBUTTONUP:
		if (SplitStatus==Sizing) {
			SplitStatus=None;
			ReleaseCapture();
		}
		break;
	case WM_MOUSEMOVE:
		{
			POINTS Tmp = MAKEPOINTS(lParam);
			POINT MousePos;
			MousePos.x = Tmp.x;
			MousePos.y = Tmp.y;
			MapWindowPoints(m_hwnd, Parent, &MousePos, 1);
			if (SplitStatus==Sizing) {
				RECT ParentRect, MyRect;
				GetWindowRect(Parent, &ParentRect);
				GetWindowRect(m_hwnd, &MyRect);
				m_Pos = MousePos.y - m_Height/2;
				// Layout();
				Owner->Layout();
			}
			break;
		}
		break;
	case WM_DESTROY:
	{
		OutputDebugString(TEXT("Splitter WM_DESTROY\n"));
		return DefWindowProc(Window, message, wParam, lParam);
	}
	
	default:
		return DefWindowProc(Window, message, wParam, lParam);
	}
	return 0;
}