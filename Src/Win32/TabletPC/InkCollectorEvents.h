// InkCollectorEvents.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __TabletPC_InkCollectorEvents_h__
#define __TabletPC_InkCollectorEvents_h__

#include <cassert>
// XXX what is this for? #define interface struct
#include <msinkaut.h>

/////////////////////////////////////////////////////////
//                                          
// class CInkCollectorEvents
// 
// Derive from CInkCollectorEvents
//
// Passes Ink Collector events from the Ink Collector to the derived class
//
// This object marshals itself free threaded so all implemented 
// event handlers need to be free threaded as well. 
// Windows APIs may cause a switch to another thread; 
// the event handler is not guaranteed to be running on the same 
// thread as the window connected to the Ink Collector.
//
/////////////////////////////////////////////////////////
class CInkCollectorEvents : public _IInkCollectorEvents
{
public:

	/////////////////////////////////////////////////////////
    // IUnknown Interface
	/////////////////////////////////////////////////////////
    
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    
	/////////////////////////////////////////////////////////
    // IDispatch Interface
	/////////////////////////////////////////////////////////

	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid,LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);

		
	STDMETHOD(Invoke)( DISPID dispidMember, REFIID riid, LCID lcid, WORD /*wFlags*/, DISPPARAMS* pdispparams, 
        VARIANT* pvarResult,  EXCEPINFO* /*pexcepinfo*/,   UINT* /*puArgErr*/);
    

	/////////////////////////////////////////////////////////////////////////////
    // Events - implement the virtual function to pick up these events
	/////////////////////////////////////////////////////////////////////////////

	// Stroke event - called at the end of the stroke
	virtual void Stroke( IInkCursor* Cursor, IInkStrokeDisp* Stroke,	VARIANT_BOOL *Cancel) { return;}

	virtual void CursorDown(IInkCursor* Cursor, IInkStrokeDisp* Stroke) { return ;}

	virtual void NewPackets(IInkCursor* Cursor, IInkStrokeDisp* Stroke, long PacketCount,  VARIANT* PacketData) {return;}

	virtual void DblClick(VARIANT_BOOL *Cancel) {return;}
    
	virtual void MouseMove(InkMouseButton Button, InkShiftKeyModifierFlags Shift, long pX, long pY, VARIANT_BOOL *Cancel) {return;} 
        
	virtual void MouseDown(InkMouseButton Button, InkShiftKeyModifierFlags Shift, long pX, long pY, VARIANT_BOOL *Cancel) {return;}
                
	virtual void MouseUp(InkMouseButton Button, InkShiftKeyModifierFlags Shift, long pX, long pY, VARIANT_BOOL *Cancel) {return;}
        
	virtual void MouseWheel(InkMouseButton Button, InkShiftKeyModifierFlags Shift, long Delta, long X, long Y, VARIANT_BOOL *Cancel) {return ;}

	virtual void NewInAirPackets(IInkCursor* Cursor,  long lPacketCount, VARIANT* PacketData) {return;}

	virtual void CursorButtonDown( IInkCursor* Cursor, IInkCursorButton* Button) {return;}

    virtual void CursorButtonUp(IInkCursor* Cursor, IInkCursorButton* Button) {return;}

	virtual void CursorInRange(IInkCursor* Cursor, VARIANT_BOOL NewCursor,  VARIANT ButtonsState) {return;}
 
	virtual void CursorOutOfRange(IInkCursor* Cursor) {return;}

    virtual void SystemGesture(IInkCursor* Cursor, InkSystemGesture Id, long X, long Y,
		long Modifier,  BSTR Character, long CursorMode) {return;}

	virtual void Gesture(IInkCursor* Cursor, IInkStrokes* Strokes,  VARIANT Gestures, VARIANT_BOOL* Cancel) {return;}

	virtual void TabletAdded( IInkTablet* Tablet) {return;}

	virtual void TabletRemoved(long TabletId) {return;}
    

	/////////////////////////////////////////////////////////

    CInkCollectorEvents();
    virtual ~CInkCollectorEvents();
    
    // Set up free threaded marshaller.
    HRESULT Init();
    
	// Set up connection between sink and Ink Collector
    HRESULT AdviseInkCollector(IInkCollector *pIInkCollector);
    
	// Remove the connection of the event sink to the Ink Collector
    HRESULT UnadviseInkCollector();
    
private:
    
    // Connection point on InkCollector
    IConnectionPoint *m_pIConnectionPoint;
    
    // Cookie returned from advise
    DWORD m_dwCookie;
    
    // Free threaded marshaler.
    IUnknown *m_punkFTM;
};

#endif // included
