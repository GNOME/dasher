// Prefs.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#include "../resource.h"
#include "../WinUTF8.h"
#include "../WinLocalisation.h"
#include "Prefs.h"

#include <utility> // for std::pair

using namespace Dasher;
using namespace std;


CPrefs::CPrefs(HWND Parent, CCanvas* m_pCanvas, CDasherWindow* m_pWindow, CDasherSettingsInterface* m_pSettings, CDasherWidgetInterface* m_pWidget)
	: m_hwnd(0), m_pCanvas(m_pCanvas), m_pWindow(m_pWindow), m_pSettings(m_pSettings), m_pWidget(m_pWidget)
{
	DialogBoxParam(WinHelper::hInstApp, (LPCTSTR)IDD_PREFS, Parent, (DLGPROC)WinWrapMap::WndProc, (LPARAM)this);
}

void CPrefs::PopulateWidgets()
{	
	if (m_pWindow->GetStartOnLeft()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_LEFT), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetStartOnSpace()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_SPACE), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetWindowPause()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_WINDOWPAUSE), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetMouseposStart()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_MOUSEPOS), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetDasherDimensions()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_1D), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetDasherEyetracker()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_EYETRACKER), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetKeyControl()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_BUTTON), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetOutlines()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_OUTLINE), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetTimeStamp()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_TIMESTAMP), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetCopyAll()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_COPYALLONSTOP), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetDrawMouse()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_DRAWMOUSE), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetDrawMouseLine()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_DRAWMOUSELINE), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetSpeech()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_SPEECH), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pWindow->GetPaletteChange()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_COLOURSCHEME), BM_SETCHECK, BST_CHECKED, 0);
	}
	m_pWidget->Redraw();
}

LRESULT CPrefs::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
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
		case (IDOK):
			if (SendMessage(GetDlgItem(Window,IDC_LEFT), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->StartOnLeft(true);
			} else {
				m_pSettings->StartOnLeft(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_SPACE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->StartOnSpace(true);
			} else {
				m_pSettings->StartOnSpace(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_WINDOWPAUSE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->WindowPause(true);
			} else {
				m_pSettings->WindowPause(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_MOUSEPOS), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->MouseposStart(true);
			} else {
				m_pSettings->MouseposStart(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_SPEECH), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->Speech(true);
			} else {
				m_pSettings->Speech(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_1D), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->SetDasherDimensions(true);
			} else {
				m_pSettings->SetDasherDimensions(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_EYETRACKER), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->SetDasherEyetracker(true);
			} else {
				m_pSettings->SetDasherEyetracker(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_BUTTON), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->KeyControl(true);
			} else {
				m_pSettings->KeyControl(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_OUTLINE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->OutlineBoxes(true);
			} else {
				m_pSettings->OutlineBoxes(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_TIMESTAMP), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->TimeStampNewFiles(true);
			} else {
				m_pSettings->TimeStampNewFiles(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_COPYALLONSTOP), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->CopyAllOnStop(true);
			} else {
				m_pSettings->CopyAllOnStop(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_DRAWMOUSE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->DrawMouse(true);
			} else {
				m_pSettings->DrawMouse(false);
			}
			if (SendMessage(GetDlgItem(Window,IDC_DRAWMOUSELINE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->DrawMouseLine(true);
			} else {
				m_pSettings->DrawMouseLine(false);
			}

			if (SendMessage(GetDlgItem(Window,IDC_COLOURSCHEME), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pSettings->PaletteChange(true);
			} else {
				m_pSettings->PaletteChange(false);
			}


			EndDialog(Window, LOWORD(wParam));
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