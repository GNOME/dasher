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
#include <algorithm>

// For WinCE
#ifndef MAKEPOINTS
#define MAKEPOINTS(l)   (*((POINTS FAR *) & (l)))
#endif

/////////////////////////////////////////////////////////////////////////////

CSplitter::CSplitter(CSplitterOwner* pOwner, int iPos)
          :	m_SplitStatus(None), m_iPos(iPos), m_pOwner(pOwner) 
{

}
int CSplitter::GetHeight()
{
  // (from MSDN) SM_CYSIZEFRAME: 
  // The thickness of the sizing border around the perimeter of a
  // window that can be resized, in pixels.
  // SM_CXSIZEFRAME is the width of the horizontal border, and
  // SM_CYSIZEFRAME is the height of the vertical border. 
  return std::max(6, GetSystemMetrics(SM_CYSIZEFRAME));
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

  if (m_SplitStatus == Sizing)
  {
    POINTS Tmp = MAKEPOINTS(lParam);
    POINT MousePos;
    MousePos.x = Tmp.x;
    MousePos.y = Tmp.y;
    MapWindowPoints(GetParent(), &MousePos, 1);
    m_iPos = MousePos.y - GetHeight() / 2;

    m_pOwner->Layout();
  }
  return 0;
}
/////////////////////////////////////////////////////////////////////////////
