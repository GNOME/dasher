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

   m_pButtonTypePage = new CButtonTypePage(hParent, pDasher, pAppSettings);

    PROPSHEETPAGE psp[1];
    PROPSHEETHEADER psh;

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[0].hInstance = WinHelper::hInstApp;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_ALPHABET);
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = (DLGPROC) WinWrapMap::PSWndProc;
    psp[0].pszTitle =  MAKEINTRESOURCE(IDS_PREFS_ALPHABET);
    psp[0].lParam = (LPARAM) m_pButtonTypePage;
    psp[0].pfnCallback = NULL;

    
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

CButtonPrefs::~CButtonPrefs()
{
  //if (m_pAlphabetBox != NULL) {
  //  delete m_pAlphabetBox;
  //  m_pAlphabetBox  = NULL;
  //}
}



void CButtonPrefs::PopulateWidgets() {
 
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
    //  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
   //   {
    //    if(SendMessage(GetDlgItem(Window, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED) {
     //     m_pDasher->SetBoolParameter(menutable[ii].paramNum, true);
    //    }
    //    else {
    //      m_pDasher->SetBoolParameter(menutable[ii].paramNum, false);
    //    }
    //  }
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
