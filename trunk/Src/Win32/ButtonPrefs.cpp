// ButtonPrefs.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"
#include "Prsht.h"

#include "ButtonPrefs.h"
#include "../../DasherCore/Parameters.h"

#include "../resource.h"
#include <utility>              // for std::pair
#include "../Dasher.h"

using namespace Dasher;
using namespace std;

CButtonPrefs::CButtonPrefs(HWND hParent, CDasher *pDasher, CAppSettings *pAppSettings)
:m_pDasher(pDasher), m_pAppSettings(pAppSettings) {
  m_hwnd = 0;

  // FIXME - is hParent still needed here?

 //  m_pButtonSettingsPage = new CButtonSettingsPage(hParent, pDasher, pAppSettings);

 //   PROPSHEETPAGE psp[1];
 //   PROPSHEETHEADER psh;

 ///*   psp[0].dwSize = sizeof(PROPSHEETPAGE);
 //   psp[0].dwFlags = PSP_USEICONID | PSP_USETITLE;
 //   psp[0].hInstance = WinHelper::hInstApp;
 //   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_BUTTON_PREFS);
 //   psp[0].pszIcon = NULL;
 //   psp[0].pfnDlgProc = (DLGPROC) WinWrapMap::PSWndProc;
 //   psp[0].pszTitle =  MAKEINTRESOURCE(IDS_BUTTON_PREFS_TITLE);
 //   psp[0].lParam = (LPARAM) m_pButtonTypePage;
 //   psp[0].pfnCallback = NULL;*/

 //   psp[0].dwSize = sizeof(PROPSHEETPAGE);
 //   psp[0].dwFlags = PSP_USEICONID | PSP_USETITLE;
 //   psp[0].hInstance = WinHelper::hInstApp;
 //   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_BUTTON_SETTINGS);
 //   psp[0].pszIcon = NULL;
 //   psp[0].pfnDlgProc = (DLGPROC) WinWrapMap::PSWndProc;
 //   psp[0].pszTitle =  MAKEINTRESOURCE(IDS_BUTTON_SETTINGS_TITLE);
 //   psp[0].lParam = (LPARAM) m_pButtonSettingsPage;
 //   psp[0].pfnCallback = NULL;
 //   
 //   psh.dwSize = sizeof(PROPSHEETHEADER);
 //   psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE;
 //   psh.hwndParent = hParent;
 //   psh.hInstance = WinHelper::hInstApp;
 //   psh.pszIcon = NULL;
 //   psh.pszCaption =  MAKEINTRESOURCE(IDS_BUTTON_PREFS_DIALOGUE_TITLE);
 //   psh.nPages = sizeof(psp) /
 //      sizeof(PROPSHEETPAGE);
 //   psh.nStartPage = 0;
 //   psh.ppsp = (LPCPROPSHEETPAGE) &psp;
 //   psh.pfnCallback = NULL;
//    PropertySheet(&psh);
//    return;


   DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_BUTTON_SETTINGS, hParent, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
  
   PopulateWidgets();
}

CButtonPrefs::~CButtonPrefs()
{
  //if (m_pAlphabetBox != NULL) {
  //  delete m_pAlphabetBox;
  //  m_pAlphabetBox  = NULL;
  //}
}



void CButtonPrefs::PopulateWidgets() {
   // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher

  m_hZoomStepsSlider = GetDlgItem(m_hwnd, IDC_ZOOMSTEPS_SLIDER);
  SendMessage(m_hZoomStepsSlider, TBM_SETPAGESIZE, 0L, 1); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hZoomStepsSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(1, 64));
  SendMessage(m_hZoomStepsSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_ZOOMSTEPS));

  m_hZoomStepsBox = GetDlgItem(m_hwnd, IDC_ZOOMSTEPS_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(m_pAppSettings->GetLongParameter(LP_ZOOMSTEPS)));
  SendMessage(m_hZoomStepsBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);


  m_hRightZoomSlider = GetDlgItem(m_hwnd, IDC_RIGHTZOOM_SLIDER);
  SendMessage(m_hRightZoomSlider, TBM_SETPAGESIZE, 0L, 1); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hRightZoomSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(1, 9));
  SendMessage(m_hRightZoomSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_RIGHTZOOM)/1024.0);

  m_hRightZoomBox = GetDlgItem(m_hwnd, IDC_RIGHTZOOM_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%0.2f"), m_pAppSettings->GetLongParameter(LP_RIGHTZOOM)/1024.0);
  SendMessage(m_hRightZoomBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

  m_hNoBoxesSlider = GetDlgItem(m_hwnd, IDC_NOBOXES_SLIDER);
  SendMessage(m_hNoBoxesSlider, TBM_SETPAGESIZE, 0L, 1); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hNoBoxesSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(2, 10));
  SendMessage(m_hNoBoxesSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_B));

  m_hNoBoxesBox = GetDlgItem(m_hwnd, IDC_NBOXES_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(m_pAppSettings->GetLongParameter(LP_B)));
  SendMessage(m_hNoBoxesBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

  m_hSafetySlider = GetDlgItem(m_hwnd, IDC_SAFETY_SLIDER);
  SendMessage(m_hSafetySlider, TBM_SETPAGESIZE, 0L, 10); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hSafetySlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(0, 256));
  SendMessage(m_hSafetySlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_S));

  m_hSafetyBox = GetDlgItem(m_hwnd, IDC_SAFETY_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(m_pAppSettings->GetLongParameter(LP_S)));
  SendMessage(m_hSafetyBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

  m_hNonUniformSlider = GetDlgItem(m_hwnd, IDC_NONUNIFORM_SLIDER);
  SendMessage(m_hNonUniformSlider, TBM_SETPAGESIZE, 0L, 50); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hNonUniformSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(-400, 400));
  SendMessage(m_hNonUniformSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_R));

  m_hNonUniformBox = GetDlgItem(m_hwnd, IDC_NU_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(m_pAppSettings->GetLongParameter(LP_R)));
  SendMessage(m_hNonUniformBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
}

bool CButtonPrefs::Apply() 
{
  double NewValue;

  NewValue = SendMessage(m_hZoomStepsSlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_ZOOMSTEPS, static_cast<int>(NewValue));

  NewValue = SendMessage(m_hRightZoomSlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_RIGHTZOOM, static_cast<int>(NewValue * 1024));

  NewValue = SendMessage(m_hNoBoxesSlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_B, static_cast<int>(NewValue));

  NewValue = SendMessage(m_hSafetySlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_S, static_cast<int>(NewValue));

  NewValue = SendMessage(m_hNonUniformSlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_R, static_cast<int>(NewValue));


	// Return false (and notify the user) if something is wrong.
	return TRUE;
}

LRESULT CButtonPrefs::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_INITDIALOG:
    {
      if(!m_hwnd) {             // If this is the initial dialog for the first time
        m_hwnd = Window;
        PopulateWidgets();
      }
      return TRUE;
      break;
    }
  case WM_HSCROLL: {
    double dNewValue;

    dNewValue = SendMessage(m_hZoomStepsSlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(dNewValue));
    SendMessage(m_hZoomStepsBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    dNewValue = SendMessage(m_hRightZoomSlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%0.2f"), dNewValue);
    SendMessage(m_hRightZoomBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    dNewValue = SendMessage(m_hNoBoxesSlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(dNewValue));
    SendMessage(m_hNoBoxesBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    dNewValue = SendMessage(m_hSafetySlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(dNewValue));
    SendMessage(m_hSafetyBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    dNewValue = SendMessage(m_hNonUniformSlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(dNewValue));
    SendMessage(m_hNonUniformBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    EnableWindow(GetDlgItem(Window, IDC_BP_APPLY), true);

		return TRUE;
    }
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case (IDC_DISPLAY):
      if(HIWORD(wParam) == EN_CHANGE) {
        HWND Control = GetDlgItem(Window, IDC_DISPLAY);
        LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1;        // +1 to allow for terminator
        TCHAR *Buffer = new TCHAR[BufferLength];
        SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM) Buffer);
        string ItemName;
        SendMessage(GetDlgItem(Window, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) Buffer);
        delete[]Buffer;
      }
      break;
    case (IDC_BP_OK):
      Apply();
      EndDialog(Window, LOWORD(wParam));
      return TRUE;
    case (IDC_BP_CANCEL):
      EndDialog(Window, LOWORD(wParam));
      return TRUE;
    case IDC_BP_APPLY:
      Apply();
      EnableWindow(GetDlgItem(Window, IDC_BP_APPLY), false);
      break;
    case ID_CANCEL_KEYCONT:
      EndDialog(Window, LOWORD(wParam));
      return TRUE;
      break;
    }
  default:
    return FALSE;
  }
}
