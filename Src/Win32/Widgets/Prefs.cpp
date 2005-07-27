// Prefs.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"
#include "Prsht.h"

#include "Prefs.h"
#include "../../DasherCore/Parameters.h"

#include "../resource.h"
#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;

CPrefs::CPrefs(HWND hParent, CDasher *pDasher)
:m_pDasher(pDasher) {
  m_hwnd = 0;

    PROPSHEETPAGE psp[6];
    PROPSHEETHEADER psh;

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[0].hInstance = WinHelper::hInstApp;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_ALPHABET);
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = NULL;
    psp[0].pszTitle =  MAKEINTRESOURCE(IDS_PREFS_ALPHABET);
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;

    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[1].hInstance = WinHelper::hInstApp;
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_COLOUR);
    psp[1].pszIcon = NULL;
    psp[1].pfnDlgProc = NULL;
    psp[1].pszTitle =  MAKEINTRESOURCE(IDS_PREFS_COLOUR);
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;

    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[2].hInstance = WinHelper::hInstApp;
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_PREFS);
    psp[2].pszIcon = NULL;
    psp[2].pfnDlgProc = (DLGPROC) WinWrapMap::PSWndProc;
    psp[2].pszTitle = MAKEINTRESOURCE(IDS_PREFS_CONTROL);
    psp[2].lParam = (LPARAM) this;
    psp[2].pfnCallback = NULL;

    psp[3].dwSize = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[3].hInstance = WinHelper::hInstApp;
    psp[3].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG3);
    psp[3].pszIcon = NULL;
    psp[3].pfnDlgProc = NULL;
    psp[3].pszTitle =  MAKEINTRESOURCE(IDS_PREFS_VIEW);
    psp[3].lParam = 0;
    psp[3].pfnCallback = NULL;

    psp[4].dwSize = sizeof(PROPSHEETPAGE);
    psp[4].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[4].hInstance = WinHelper::hInstApp;
    psp[4].pszTemplate = MAKEINTRESOURCE(IDD_KEYCONTROL1);
    psp[4].pszIcon = NULL;
    psp[4].pfnDlgProc = NULL;
    psp[4].pszTitle =  MAKEINTRESOURCE(IDS_PREFS_ADVANCED);
    psp[4].lParam = 0;
    psp[4].pfnCallback = NULL;

    psp[5].dwSize = sizeof(PROPSHEETPAGE);
    psp[5].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[5].hInstance = WinHelper::hInstApp;
    psp[5].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG4);
    psp[5].pszIcon = NULL;
    psp[5].pfnDlgProc = NULL;
    psp[5].pszTitle =  MAKEINTRESOURCE(IDS_PREFS_LM);
    psp[5].lParam = 0;
    psp[5].pfnCallback = NULL;
    
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE;
    psh.hwndParent = hParent;
    psh.hInstance = WinHelper::hInstApp;
    psh.pszIcon = NULL;
    psh.pszCaption =  MAKEINTRESOURCE(IDS_PREFS_TITLE);
    psh.nPages = sizeof(psp) /
       sizeof(PROPSHEETPAGE);
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = NULL;
    PropertySheet(&psh);
//    return;


 // DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_PREFS, hP
    //arent, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
  PopulateWidgets();
}

struct menuentry {
  int paramNum; // enum value in Parameters.h for setting store
  int idcNum;   // #define value in resource.h for dasher.rc
};

// List of menu items that will be displayed in the General Preferences
static menuentry menutable[] = {
  {BP_START_MOUSE, IDC_LEFT},
  {BP_START_SPACE, IDC_SPACE},
  {BP_WINDOW_PAUSE, IDC_WINDOWPAUSE},
  {BP_MOUSEPOS_MODE, IDC_MOUSEPOS},
  {BP_NUMBER_DIMENSIONS, IDC_1D},
  {BP_EYETRACKER_MODE, IDC_EYETRACKER},
  {BP_KEY_CONTROL, IDC_BUTTON},
  {BP_OUTLINE_MODE, IDC_OUTLINE},
  {BP_DRAW_MOUSE, IDC_DRAWMOUSE},
  {BP_DRAW_MOUSE_LINE, IDC_DRAWMOUSELINE},
  {BP_PALETTE_CHANGE, IDC_COLOURSCHEME},
  {BP_TIME_STAMP, IDC_TIMESTAMP},   // Not global setting - specific to editbox/widget
  {BP_COPY_ALL_ON_STOP, IDC_COPYALLONSTOP}, // Same
  {BP_SPEECH_MODE, IDC_SPEECH}     // Same
};

void CPrefs::PopulateWidgets() {
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    if(m_pDasher->GetBoolParameter(menutable[ii].paramNum)) {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
    }
    else  {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_UNCHECKED, 0);
    }
  }
}

LRESULT CPrefs::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
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
    case (IDOK):
      for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
      {
        if(SendMessage(GetDlgItem(Window, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED) {
          m_pDasher->SetBoolParameter(menutable[ii].paramNum, true);
        }
        else {
          m_pDasher->SetBoolParameter(menutable[ii].paramNum, false);
        }
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
