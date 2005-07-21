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
#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;

CPrefs::CPrefs(HWND hParent, CDasher *pDasher)
:m_pDasher(pDasher) {
  m_hwnd = 0;
  DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_PREFS, hParent, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
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
  {BP_PALETTE_CHANGE, IDC_COLOURSCHEME}//,
  //{BP_TIME_STAMP, IDC_TIMESTAMP},   // Not global setting - specific to editbox/widget
  //{BP_COPY_ALL_ON_STOP, IDC_COPYALLONSTOP}, // Same
  //{BP_SPEECH_MODE, IDC_SPEECH},     // Same
  
};

void CPrefs::PopulateWidgets() {
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    if(m_pDasher->GetBoolParameter(menutable[ii].paramNum)) {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
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
