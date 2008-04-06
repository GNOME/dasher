// Splitter.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"
#include "Splitter.h"

#include <iostream>
#include <cstring>

// For WinCE
#ifndef MAKEPOINTS
#define MAKEPOINTS(l)   (*((POINTS FAR *) & (l)))
#endif

/////////////////////////////////////////////////////////////////////////////

CSplitter::CSplitter(CSplitterOwner* pOwner, int iPos)
          :	m_SplitStatus(None), m_iPos(iPos), m_pOwner(pOwner) 
{

}

/////////////////////////////////////////////////////////////////////////////

HWND CSplitter::Create(HWND hParent)
{	
	return CWindowImpl<CSplitter>::Create(hParent,NULL,NULL,WS_CHILD | WS_VISIBLE);
}

/////////////////////////////////////////////////////////////////////////////

void CSplitter::Move(int iPos, int Width) 
{
	m_iPos = iPos;
	MoveWindow(0, m_iPos, Width, GetHeight(), TRUE);
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CSplitter::OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	m_SplitStatus = Sizing;
    SetCapture();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CSplitter::OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	if( m_SplitStatus == Sizing) 
	{
		m_SplitStatus = None;
		ReleaseCapture();
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CSplitter::OnMouseMove(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	POINTS Tmp = MAKEPOINTS(lParam);
	POINT MousePos;
	MousePos.x = Tmp.x;
	MousePos.y = Tmp.y;
	MapWindowPoints( GetParent(), &MousePos, 1);
	
	if( m_SplitStatus == Sizing) 
	{
		RECT ParentRect, MyRect;
		::GetWindowRect( GetParent(), &ParentRect);
		GetWindowRect(&MyRect);
#ifndef _WIN32_WCE
		m_iPos = MousePos.y - GetSystemMetrics(SM_CYSIZEFRAME) / 2;
#else
		// TODO: Fix this on Windows CE
		m_iPos = MousePos.y - 4;
#endif

//		WCHAR wszDebugText[128];
//		_snwprintf(wszDebugText, 128, L"Setting size: %d\n", m_iPos);
//		OutputDebugStringW(wszDebugText);

		m_pOwner->Layout();
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
