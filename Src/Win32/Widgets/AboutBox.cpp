// AboutBox.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#include "AboutBox.h"
#include "../resource.h"


CAboutbox::CAboutbox(HWND Parent)
{
	DialogBox(WinHelper::hInstApp, (LPCTSTR)IDD_ABOUTBOX, Parent, (DLGPROC)About);
}


LRESULT CALLBACK CAboutbox::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}