// Window.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003 David Ward
//
/////////////////////////////////////////////////////////////////////////////

// A very simple application framework - based on
// A Lightweight Window Wrapper - Steve Hanov, C++ User's Journalaugst 2002

#include <windows.h>

class CWindow
{

public:
    CWindow();
    HWND Create(int x, int y, int nWidth, int nHeight,
            HWND hParent, HMENU hMenu, HINSTANCE hInstance);

    HWND m_hwnd;

protected:
    static LRESULT CALLBACK BaseWndProc(HWND hwnd, UINT msg,
        WPARAM wParam, LPARAM lParam);
    
    virtual LRESULT WindowProc(HWND hwnd, UINT msg,
        WPARAM wParam, LPARAM lParam, PBOOL pbProcessed);

    WNDCLASSEX m_WndClass;
    DWORD m_dwExtendedStyle;
    DWORD m_dwStyle;
    TCHAR* m_pszClassName;
    TCHAR* m_pszTitle;
};  



/////////////////////////////////////////////////////////////////////////////

inline LRESULT CALLBACK CWindow::BaseWndProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam)
{
    //A pointer to the object is passed in the CREATESTRUCT
    if(msg == WM_NCCREATE)
        SetWindowLongPtr(hwnd, GWLP_USERDATA,
        (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
    
    BOOL bProcessed = FALSE;
    LRESULT lResult;
    
    //Retrieve the pointer
    CWindow *pObj = 
        (CWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    //Filter message through child classes
    if(pObj)
        lResult = pObj->WindowProc(hwnd, msg, wParam, lParam,
            &bProcessed);

    if(bProcessed)
        return lResult;
    else
    {
        //If message was unprocessed, send it back to Windows.
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

/////////////////////////////////////////////////////////////////////////////

inline LRESULT CWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam,
    LPARAM lParam, PBOOL pbProcessed)
{
    //This may be overridden to process messages.
    *pbProcessed = FALSE;
    return NULL;
}

