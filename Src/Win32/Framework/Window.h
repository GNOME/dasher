// Window.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Framework_Window_h__
#define __Framework_Window_h__

#include <windows.h>

#pragma warning(disable:4311)
#pragma warning(disable:4312)

/////////////////////////////////////////////////////////////////////////////

// To wrap a non-dialog window class, derive from CWindow
// CreateWindow/CreateWindowEx should be passed (void*)this as its last parameter
// The result should be set into the protected m_hwnd

// All messages then get passed to the virtual WndProc function
// The bProcessed is not set to true, then DefWindowProc will be called for you

class CWindow 
{

public:
    CWindow();
	virtual ~CWindow() {}


protected:

   	HWND m_hwnd;

	static LRESULT CALLBACK BaseWndProc(HWND hwnd, UINT msg,
        WPARAM wParam, LPARAM lParam);
    
    virtual LRESULT WndProc(HWND hwnd, UINT msg,
        WPARAM wParam, LPARAM lParam, bool& bProcessed);
};  



/////////////////////////////////////////////////////////////////////////////

inline LRESULT CALLBACK CWindow::BaseWndProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam)
{
    //A pointer to the object is passed in the CREATESTRUCT
     
    bool bProcessed = false;
    LRESULT lResult;
    
    //Retrieve the pointer
    CWindow *pObj = 
        (CWindow *)GetWindowLong(hwnd, GWL_USERDATA);

    //Filter message through child classes
    if(pObj)
        lResult = pObj->WndProc(hwnd, msg, wParam, lParam,
            bProcessed);
#ifdef DASHER_WINCE
	else if(msg == WM_CREATE)
#else
	else if(msg == WM_NCCREATE)
#endif
	{
		void *p = ((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLong(hwnd, GWL_USERDATA, (LONG)p);
		pObj = (CWindow *)p;
		lResult = pObj->WndProc(hwnd, msg, wParam, lParam,
            bProcessed);
	}

    if(bProcessed)
        return lResult;

	//If message was unprocessed, send it back to Windows.
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////

inline LRESULT CWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& bProcessed)
{
    //This may be overridden to process messages.
    bProcessed = false;
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

inline CWindow::CWindow() : m_hwnd(0)
{

}

#endif