// CursorInRange.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "..\Common\WinCommon.h"

#include "CursorInRange.h"
#include <msinkaut_i.c>

/////////////////////////////////////////////////////////////////////////////

HRESULT CMyInkEvents::Init(HWND hWnd)
{
    m_hWnd = hWnd;
    return CInkCollectorEvents::Init();
}  

/////////////////////////////////////////////////////////////////////////////

CMyInkEvents::CMyInkEvents() : m_hWnd(NULL)
{
}


/////////////////////////////////////////////////////////////////////////////

void CMyInkEvents::CursorInRange( IInkCursor* Cursor,
							    VARIANT_BOOL NewCursor,
								VARIANT ButtonsState)
{
	PostMessage(m_hWnd, WM_CURSOR_IN_RANGE, 0,0);
}

/////////////////////////////////////////////////////////////////////////////

void CMyInkEvents::CursorOutOfRange( IInkCursor* Cursor)
{
	PostMessage(m_hWnd, WM_CURSOR_OUT_OF_RANGE, 0,0);
}

/////////////////////////////////////////////////////////////////////////////

CCursorInRange::CCursorInRange()
	: m_pInkCollector(0)
{
}

/////////////////////////////////////////////////////////////////////////////

CCursorInRange::~CCursorInRange()
{
	if (m_pInkCollector != NULL)
	{
		m_Events.UnadviseInkCollector();
		m_pInkCollector->put_Enabled(VARIANT_FALSE);
		m_pInkCollector->Release();
	}
}

/////////////////////////////////////////////////////////////////////////////

// Handle all initializaton

HRESULT CCursorInRange::Initialize(HWND hWnd)
{
	// Initialize event sink. This consists of setting up the free threaded marshaler.
	HRESULT hr = m_Events.Init(hWnd);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create the ink collector
	hr = CoCreateInstance(CLSID_InkCollector, NULL, CLSCTX_ALL,
		IID_IInkCollector, (void **) &m_pInkCollector);

	if (FAILED(hr))
	{
		return hr;
	}

	// Set up connection between Ink Collector and our event sink        
	hr = m_Events.AdviseInkCollector(m_pInkCollector);

	if (FAILED(hr))
	{
		return hr;
	}

	// Attach Ink Collector to window
	hr = m_pInkCollector->put_hWnd((long) hWnd);

	if (FAILED(hr))
	{
		return hr;
	}

	// only collect gestures
	hr = m_pInkCollector->put_CollectionMode( ICM_GestureOnly ); 
	if (FAILED(hr))
	{
		return hr;
	}

	// Switch off drawing of strokes
	hr = m_pInkCollector->put_DynamicRendering( FALSE );
	if (FAILED(hr))
	{
		return hr;
	}

	// dont want to pick up the mouse
	hr = m_pInkCollector->SetAllTabletsMode( FALSE );
	if (FAILED(hr))
	{
		return hr;
	}


	// Allow Ink Collector to receive input.
	return m_pInkCollector->put_Enabled(VARIANT_TRUE);
}

/////////////////////////////////////////////////////////////////////////////
