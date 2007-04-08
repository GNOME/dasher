// InkCollectorEvents.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "..\Common\WinCommon.h"
#include "InkCollectorEvents.h"

/////////////////////////////////////////////////////////////////////////////

// IUnknown Interface

HRESULT __stdcall CInkCollectorEvents::QueryInterface(REFIID riid,  void **ppvObject)
{

	// Validate the input
	if (NULL == ppvObject)
	{
		return E_POINTER;
	}

	// This object only supports IDispatch/_IInkCollectorEvents
	if ((riid == IID_IUnknown)
		|| (riid == IID_IDispatch)
		|| (riid == DIID__IInkCollectorEvents))
	{
		*ppvObject = (IDispatch *) this;

		// Note: we do not AddRef here because the lifetime
		//  of this object does not depend on reference counting
		//  but on the duration of the connection set up by
		//  the user of this class.

		return S_OK;
	}
	else if (riid == IID_IMarshal)
	{
		// Assert that the free threaded marshaller has been
		// initialized.  It is necessary to call Init() before
		// invoking this method.
		assert(NULL != m_punkFTM);

		// Use free threaded marshalling.
		return m_punkFTM->QueryInterface(riid, ppvObject);
	}

	return E_NOINTERFACE;
}

/////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE CInkCollectorEvents::AddRef()
{
	// Note: we do not AddRef here because the lifetime
	// of this object depends on the duration of the connection 
	// set up by the derived class.
	return 1;
}

/////////////////////////////////////////////////////////////////////////////

ULONG STDMETHODCALLTYPE CInkCollectorEvents::Release()
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////

// IDispatch Interface

HRESULT CInkCollectorEvents::GetTypeInfoCount(UINT* pctinfo)
{
	// Not needs for processing events.
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CInkCollectorEvents::GetTypeInfo(  UINT itinfo, LCID lcid,  ITypeInfo** pptinfo)
{
	// Not needs for processing events.
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CInkCollectorEvents::GetIDsOfNames( REFIID riid, LPOLESTR* rgszNames, UINT cNames,
						 LCID lcid,	 DISPID* rgdispid)
{
	// Not needs for processing events.
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

// Translates from IDispatch to the virtual functions

HRESULT CInkCollectorEvents::Invoke(  DISPID dispidMember,  REFIID riid,  LCID lcid, 
				  WORD /*wFlags*/,	  DISPPARAMS* pdispparams,  VARIANT* pvarResult,
				  EXCEPINFO* /*pexcepinfo*/,   UINT* /*puArgErr*/)
{
	switch(dispidMember)
	{
	case DISPID_ICEStroke:
		Stroke(
			(IInkCursor*) pdispparams->rgvarg[2].pdispVal,
			(IInkStrokeDisp*) pdispparams->rgvarg[1].pdispVal,
			(VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
		break;

	case DISPID_ICECursorDown:
		CursorDown(
			(IInkCursor*) pdispparams->rgvarg[1].pdispVal,
			(IInkStrokeDisp*) pdispparams->rgvarg[0].pdispVal);               
		break;

	case DISPID_ICENewPackets:
		NewPackets(
			(IInkCursor*) pdispparams->rgvarg[3].pdispVal,
			(IInkStrokeDisp*) pdispparams->rgvarg[2].pdispVal,
			pdispparams->rgvarg[1].lVal,
			pdispparams->rgvarg[0].pvarVal);
		break;

	case DISPID_IPEDblClick:
		DblClick(
			(VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
		break;

	case DISPID_IPEMouseMove:
		MouseMove(
			(InkMouseButton) pdispparams->rgvarg[4].lVal, 
			(InkShiftKeyModifierFlags) pdispparams->rgvarg[3].lVal, 
			pdispparams->rgvarg[2].lVal,
			pdispparams->rgvarg[1].lVal,
			(VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal); 
		break;

	case DISPID_IPEMouseDown:
		MouseDown(
			(InkMouseButton) pdispparams->rgvarg[4].lVal, 
			(InkShiftKeyModifierFlags) pdispparams->rgvarg[3].lVal, 
			pdispparams->rgvarg[2].lVal,
			pdispparams->rgvarg[1].lVal,
			(VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal); 
		break;

	case DISPID_IPEMouseUp:
		MouseUp(
			(InkMouseButton) pdispparams->rgvarg[4].lVal, 
			(InkShiftKeyModifierFlags) pdispparams->rgvarg[3].lVal, 
			pdispparams->rgvarg[2].lVal,
			pdispparams->rgvarg[1].lVal,
			(VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal); 
		break;

	case DISPID_IPEMouseWheel:
		MouseWheel(
			(InkMouseButton) pdispparams->rgvarg[5].lVal, 
			(InkShiftKeyModifierFlags) pdispparams->rgvarg[4].lVal, 
			pdispparams->rgvarg[3].lVal,
			pdispparams->rgvarg[2].lVal,
			pdispparams->rgvarg[1].lVal,
			(VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal); 
		break;

	case DISPID_ICENewInAirPackets:
		NewInAirPackets(
			(IInkCursor*) pdispparams->rgvarg[2].pdispVal,
			pdispparams->rgvarg[1].lVal,
			pdispparams->rgvarg[0].pvarVal);
		break;

	case DISPID_ICECursorButtonDown:
		CursorButtonDown(
			(IInkCursor*) pdispparams->rgvarg[1].pdispVal,
			(IInkCursorButton*) pdispparams->rgvarg[0].pdispVal);
		break;

	case DISPID_ICECursorButtonUp:
		CursorButtonUp(
			(IInkCursor*) pdispparams->rgvarg[1].pdispVal,
			(IInkCursorButton*) pdispparams->rgvarg[0].pdispVal);
		break;

	case DISPID_ICECursorInRange:
		CursorInRange(
			(IInkCursor*) pdispparams->rgvarg[2].pdispVal,
			(VARIANT_BOOL) pdispparams->rgvarg[1].iVal,
			pdispparams->rgvarg[0]);
		break;

	case DISPID_ICECursorOutOfRange:
		CursorOutOfRange(
			(IInkCursor*) pdispparams->rgvarg[0].pdispVal);
		break;

	case DISPID_ICESystemGesture:
		SystemGesture(
			(IInkCursor*) pdispparams->rgvarg[6].pdispVal,
			(InkSystemGesture) pdispparams->rgvarg[5].lVal, 
			pdispparams->rgvarg[4].lVal, 
			pdispparams->rgvarg[3].lVal,
			pdispparams->rgvarg[2].lVal,
			pdispparams->rgvarg[1].bstrVal,
			pdispparams->rgvarg[0].lVal);
		break;

	case DISPID_ICEGesture:
		Gesture(
			(IInkCursor*) pdispparams->rgvarg[3].pdispVal,
			(IInkStrokes*) pdispparams->rgvarg[2].pdispVal,
			pdispparams->rgvarg[1],
			(VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
		break;

	case DISPID_ICETabletAdded:
		TabletAdded(
			(IInkTablet*) pdispparams->rgvarg[0].pdispVal);
		break;

	case DISPID_ICETabletRemoved:
		TabletRemoved(
			pdispparams->rgvarg[0].lVal);
		break;

	default:
		break;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

CInkCollectorEvents::CInkCollectorEvents() :	m_pIConnectionPoint(NULL),
												m_punkFTM(NULL)
{
}

/////////////////////////////////////////////////////////////////////////////

CInkCollectorEvents::~CInkCollectorEvents()
{
	UnadviseInkCollector();

	if (m_punkFTM != NULL)
		m_punkFTM->Release();
}

/////////////////////////////////////////////////////////////////////////////

// Set up free threaded marshaller.

HRESULT CInkCollectorEvents::Init()
{
	return CoCreateFreeThreadedMarshaler(this, &m_punkFTM);
}

/////////////////////////////////////////////////////////////////////////////

// Set up connection between sink and Ink Collector
HRESULT CInkCollectorEvents::AdviseInkCollector( IInkCollector *pIInkCollector)
{
	HRESULT hr = S_OK;

	// Check to ensure that the sink is not currently connected
	// with another Ink Collector...
	if (NULL == m_pIConnectionPoint)
	{
		// Get the connection point container
		IConnectionPointContainer *pIConnectionPointContainer;
		hr = pIInkCollector->QueryInterface(
			IID_IConnectionPointContainer, 
			(void **) &pIConnectionPointContainer);

		if (FAILED(hr))
		{
			return hr;
		}

		// Find the connection point for Ink Collector events
		hr = pIConnectionPointContainer->FindConnectionPoint(
			__uuidof(_IInkCollectorEvents), &m_pIConnectionPoint);

		if (SUCCEEDED(hr))
		{
			// Hook up sink to connection point
			hr = m_pIConnectionPoint->Advise(this, &m_dwCookie);
		}

		if (FAILED(hr))
		{
			// Clean up after an error.
			if (m_pIConnectionPoint)
			{
				m_pIConnectionPoint->Release();
				m_pIConnectionPoint = NULL;
			}
		}

		// We don't need the connection point container any more.
		pIConnectionPointContainer->Release();
	}
	// If the sink is already connected to an Ink Collector, return a 
	// failure; only one Ink Collector can be attached at any given time.
	else
	{
		hr = E_FAIL;
	}

	return hr;
}


/////////////////////////////////////////////////////////////////////////////

// Remove the connection of the event sink to the Ink Collector

HRESULT CInkCollectorEvents::UnadviseInkCollector()
{
	HRESULT hr = S_OK;

	// Unadvise if connected
	if (m_pIConnectionPoint != NULL)
	{
		hr = m_pIConnectionPoint->Unadvise(m_dwCookie);
		m_pIConnectionPoint->Release();
		m_pIConnectionPoint = NULL;
	}

	return hr;
}

/////////////////////////////////////////////////////////////////////////////
