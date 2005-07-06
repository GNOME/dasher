// Prefs.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "Prefs.h"
#include "../../DasherCore/Parameters.h"

#include "../resource.h"
#include <utility> // for std::pair

using namespace Dasher;
using namespace std;


CPrefs::CPrefs(HWND hParent, CDasher *pDasher )
	: m_pDasher( pDasher )
{
	m_hwnd=0;
	DialogBoxParam(WinHelper::hInstApp, (LPCTSTR)IDD_PREFS, hParent, (DLGPROC)WinWrapMap::WndProc, (LPARAM)this);
}

void CPrefs::PopulateWidgets()
{	
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher

	if ( m_pDasher->GetBoolParameter( BP_START_MOUSE )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_LEFT), BM_SETCHECK, BST_CHECKED, 0);
	}
	if ( m_pDasher->GetBoolParameter( BP_START_SPACE )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_SPACE), BM_SETCHECK, BST_CHECKED, 0);
	}
	if ( m_pDasher->GetBoolParameter( BP_WINDOW_PAUSE )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_WINDOWPAUSE), BM_SETCHECK, BST_CHECKED, 0);
	}
	if( m_pDasher->GetBoolParameter( BP_MOUSEPOS_MODE )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_MOUSEPOS), BM_SETCHECK, BST_CHECKED, 0);
	}
	if ( m_pDasher->GetBoolParameter( BP_NUMBER_DIMENSIONS )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_1D), BM_SETCHECK, BST_CHECKED, 0);
	}
	if ( m_pDasher->GetBoolParameter( BP_EYETRACKER_MODE )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_EYETRACKER), BM_SETCHECK, BST_CHECKED, 0);
	}
	if ( m_pDasher->GetBoolParameter( BP_KEY_CONTROL )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_BUTTON), BM_SETCHECK, BST_CHECKED, 0);
	}
	if ( m_pDasher->GetBoolParameter( BP_OUTLINE_MODE )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_OUTLINE), BM_SETCHECK, BST_CHECKED, 0);
	}
  
  // FIXME - These shouldn't be handled through the interface

	//if (m_pWindow->GetTimeStamp()==true) {
	//	SendMessage(GetDlgItem(m_hwnd, IDC_TIMESTAMP), BM_SETCHECK, BST_CHECKED, 0);
	//}
	//if (m_pWindow->GetCopyAll()==true) {
	//	SendMessage(GetDlgItem(m_hwnd, IDC_COPYALLONSTOP), BM_SETCHECK, BST_CHECKED, 0);
	//}
	
  if( m_pDasher->GetBoolParameter( BP_DRAW_MOUSE )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_DRAWMOUSE), BM_SETCHECK, BST_CHECKED, 0);
	}
	if(m_pDasher->GetBoolParameter( BP_DRAW_MOUSE_LINE )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_DRAWMOUSELINE), BM_SETCHECK, BST_CHECKED, 0);
	}

  // FIXME - This shouldn't be handled through the interface

	//if (m_pWindow->GetSpeech()==true) {
	//	SendMessage(GetDlgItem(m_hwnd, IDC_SPEECH), BM_SETCHECK, BST_CHECKED, 0);
	//}

	if (m_pDasher->GetBoolParameter( BP_PALETTE_CHANGE )) {
		SendMessage(GetDlgItem(m_hwnd, IDC_COLOURSCHEME), BM_SETCHECK, BST_CHECKED, 0);
	}
	
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

      // FIXME - these also need to use the new system

			if (SendMessage(GetDlgItem(Window,IDC_LEFT), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_START_MOUSE, true );
			} else {
				m_pDasher->SetBoolParameter( BP_START_MOUSE, false );
			}
			
      if (SendMessage(GetDlgItem(Window,IDC_SPACE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_START_SPACE, true );
			} else {
				m_pDasher->SetBoolParameter( BP_START_SPACE, false );
			}
			
			if (SendMessage(GetDlgItem(Window,IDC_WINDOWPAUSE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_WINDOW_PAUSE, true );
			} else {
				m_pDasher->SetBoolParameter( BP_WINDOW_PAUSE, false );
			}
			
			if (SendMessage(GetDlgItem(Window,IDC_MOUSEPOS), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_MOUSEPOS_MODE, true );
			} else {
				m_pDasher->SetBoolParameter( BP_MOUSEPOS_MODE, false );
			}
			
      // FIXME - This shouldn't go through the interface

			//if (SendMessage(GetDlgItem(Window,IDC_SPEECH), BM_GETCHECK, 0, 0)==BST_CHECKED) {
			//	m_pSettings->Speech(true);
			//} else {
			//	m_pSettings->Speech(false);
			//}

			if (SendMessage(GetDlgItem(Window,IDC_1D), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_NUMBER_DIMENSIONS, true );
			} else {
				m_pDasher->SetBoolParameter( BP_NUMBER_DIMENSIONS, false );
			}
			
			if (SendMessage(GetDlgItem(Window,IDC_EYETRACKER), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_EYETRACKER_MODE, true );
			} else {
				m_pDasher->SetBoolParameter( BP_EYETRACKER_MODE, false );
			}
			
			if (SendMessage(GetDlgItem(Window,IDC_BUTTON), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_KEY_CONTROL, true );
			} else {
				m_pDasher->SetBoolParameter( BP_KEY_CONTROL, false );
			}
			
			if (SendMessage(GetDlgItem(Window,IDC_OUTLINE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_OUTLINE_MODE, true );
			} else {
				m_pDasher->SetBoolParameter( BP_OUTLINE_MODE, false );
			}
			
      // FIXME - These shouldn't go through the interface

			//if (SendMessage(GetDlgItem(Window,IDC_TIMESTAMP), BM_GETCHECK, 0, 0)==BST_CHECKED) {
			//	m_pSettings->TimeStampNewFiles(true);
			//} else {
			//	m_pSettings->TimeStampNewFiles(false);
			//}
			//if (SendMessage(GetDlgItem(Window,IDC_COPYALLONSTOP), BM_GETCHECK, 0, 0)==BST_CHECKED) {
			//	m_pSettings->CopyAllOnStop(true);
			//} else {
			//	m_pSettings->CopyAllOnStop(false);
			//}

			if (SendMessage(GetDlgItem(Window,IDC_DRAWMOUSE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_DRAW_MOUSE, true );
			} else {
				m_pDasher->SetBoolParameter( BP_DRAW_MOUSE, false );
			}
			
			if (SendMessage(GetDlgItem(Window,IDC_DRAWMOUSELINE), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_DRAW_MOUSE_LINE, true );
			} else {
				m_pDasher->SetBoolParameter( BP_DRAW_MOUSE_LINE, false );
			}

			if (SendMessage(GetDlgItem(Window,IDC_COLOURSCHEME), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pDasher->SetBoolParameter( BP_PALETTE_CHANGE, true );
			} else {
				m_pDasher->SetBoolParameter( BP_PALETTE_CHANGE, false );
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