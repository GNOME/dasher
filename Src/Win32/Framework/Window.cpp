// Window.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "Window.h"

/////////////////////////////////////////////////////////////////////////////

CWindow::CWindow()
{
    //Set the default data for the window class.
    //These can be reset in the derived class's constructor.

    m_WndClass.cbSize = sizeof(m_WndClass);
    m_WndClass.style = CS_DBLCLKS;
    m_WndClass.lpfnWndProc = BaseWndProc;
    m_WndClass.cbClsExtra = 0;
    m_WndClass.cbWndExtra = 0;
    m_WndClass.hInstance = NULL; 
    m_WndClass.hIcon = NULL;
    m_WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    m_WndClass.hbrBackground = 
        (HBRUSH)GetStockObject(WHITE_BRUSH);
    m_WndClass.lpszMenuName = NULL;
    m_WndClass.hIconSm = NULL;

    m_dwExtendedStyle = NULL;
    m_dwStyle = NULL;
    m_pszClassName = TEXT("CWindow");
    m_pszTitle = TEXT("");
}

/////////////////////////////////////////////////////////////////////////////

HWND CWindow::Create(int x, int y, int nWidth, int nHeight,
    HWND hParent, HMENU hMenu, HINSTANCE hInstance)
{
    m_WndClass.lpszClassName = m_pszClassName;
    m_WndClass.hInstance = hInstance;
    
    //If we're already registered, this call will fail.
    RegisterClassEx(&m_WndClass);

    m_hwnd = CreateWindowEx(m_dwExtendedStyle, m_pszClassName,
        m_pszTitle, m_dwStyle, x, y, nWidth, nHeight,
        hParent, hMenu, hInstance, (void*)this);

    return m_hwnd;
}