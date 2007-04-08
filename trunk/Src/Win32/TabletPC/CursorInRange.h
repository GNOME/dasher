// CursorInRange.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __TabletPC_CursorInRange_h__
#define __TabletPC_CursorInRange_h__

// Sends CursorInRange / CursorOutOfRange events to a window using the
// TabletPC API

#include "InkCollectorEvents.h"

/////////////////////////////////////////////////////////

// User-defined events

#define WM_CURSOR_IN_RANGE WM_USER + 130
#define WM_CURSOR_OUT_OF_RANGE WM_USER + 131

/////////////////////////////////////////////////////////

// Helper class

class CMyInkEvents : public CInkCollectorEvents
{
public:
	CMyInkEvents();

    HRESULT Init(HWND hWnd);

	virtual void CursorInRange( IInkCursor* Cursor,
							    VARIANT_BOOL NewCursor,
								VARIANT ButtonsState);

    virtual void CursorOutOfRange( IInkCursor* Cursor);

private:
   
    HWND m_hWnd;
};

/////////////////////////////////////////////////////////

// Create one of these objects inside your window

class CCursorInRange
{
public:
	CCursorInRange();
	~CCursorInRange();

	// Initialize with a window to receive messsages (fails if Tablet subsystem isnt found)
	HRESULT Initialize(HWND hwnd);

private:

	// Pointer to automation interface
	IInkCollector* m_pInkCollector;

	CMyInkEvents m_Events;
	
};

/////////////////////////////////////////////////////////

#endif