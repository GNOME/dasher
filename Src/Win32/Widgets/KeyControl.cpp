// AlphabetBox.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#include "KeyControl.h"
#include "../resource.h"
#include "../WinUTF8.h"
#include "../WinLocalisation.h"

#include <utility> // for std::pair

using namespace Dasher;
using namespace std;


CKeyBox::CKeyBox(HWND Parent, CCanvas* m_pCanvas)
	: m_hwnd(0), m_pCanvas(m_pCanvas)
{
	DialogBoxParam(WinHelper::hInstApp, (LPCTSTR)IDD_KEYCONTROL, Parent, (DLGPROC)WinWrapMap::WndProc, (LPARAM)this);
}

std::string CKeyBox::GetControlText(HWND Dialog, int ControlID)
{
	HWND Control = GetDlgItem(Dialog, ControlID);
	LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1; // +1 to allow for terminator
	TCHAR* Buffer = new TCHAR[BufferLength];
	SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM)Buffer);
	string ItemName;
	WinUTF8::Tstring_to_UTF8string(Buffer, &ItemName, GetACP());
	delete[] Buffer;
	return ItemName;
}


void CKeyBox::PopulateWidgets()
{
	int widgets[8];
	widgets[0]=IDC_UPX;
	widgets[1]=IDC_UPY;
	widgets[2]=IDC_DOWNX;
	widgets[3]=IDC_DOWNY;
	widgets[4]=IDC_LEFTX;
	widgets[5]=IDC_LEFTY;
	widgets[6]=IDC_RIGHTX;
	widgets[7]=IDC_RIGHTY;

	int* coords = m_pCanvas->getkeycoords();
	for (int i=0; i<8; i++) {
			keycoords[i]=coords[i];
	}
	for (int i=0; i<8; i++) {
	HWND EditBox = GetDlgItem(m_hwnd, widgets[i]);
	SendMessage(EditBox, LB_RESETCONTENT, 0, 0);
	
	char dummybuffer[256];
	wchar_t widebuffer[256];

	itoa(keycoords[i],dummybuffer,10);
	mbstowcs(widebuffer,dummybuffer,256);

	SendMessage(EditBox, WM_SETTEXT, 0, (LPARAM) widebuffer);
	}
}

LRESULT CKeyBox::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: 
	{
		if (!m_hwnd) { // If this is the initial dialog for the first time
			m_hwnd = Window;
			PopulateWidgets();
		}
		return TRUE;
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case (IDC_DISPLAY):
			if (HIWORD(wParam)==EN_CHANGE) {
				HWND Control = GetDlgItem(Window, IDC_DISPLAY);
				LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1; // +1 to allow for terminator
				TCHAR* Buffer = new TCHAR[BufferLength];
				SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM)Buffer);
				string ItemName;
				SendMessage(GetDlgItem(Window, IDC_TEXT), WM_SETTEXT, 0, (LPARAM)Buffer);
				delete[] Buffer;
			}
			break;
		case (IDOK_KEYCONT):
			keycoords[0]=atoi(GetControlText(Window,IDC_UPX).c_str());
			keycoords[1]=atoi(GetControlText(Window,IDC_UPY).c_str());
			keycoords[2]=atoi(GetControlText(Window,IDC_DOWNX).c_str());
			keycoords[3]=atoi(GetControlText(Window,IDC_DOWNY).c_str());
			keycoords[4]=atoi(GetControlText(Window,IDC_LEFTX).c_str());
			keycoords[5]=atoi(GetControlText(Window,IDC_LEFTY).c_str());
			keycoords[6]=atoi(GetControlText(Window,IDC_RIGHTX).c_str());
			keycoords[7]=atoi(GetControlText(Window,IDC_RIGHTY).c_str());
			EndDialog(Window, LOWORD(wParam));
			m_pCanvas->setkeycoords(keycoords);
			return TRUE;
			break;
		case (IDCANCEL):
		{
			EndDialog(Window, LOWORD(wParam));
			return TRUE;
		}
		case ID_CANCEL_KEYCONT:
			EndDialog(Window, LOWORD(wParam));
			return TRUE;
		break;
		}
	default:
	return FALSE;
	}
}