// AlphabetBox.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "ControlPage.h"
#include "../resource.h"

#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CControlPage::CControlPage(HWND Parent, CDasherInterface *DI)
:m_pDasherInterface(DI), m_CurrentColours(DI->GetStringParameter(SP_COLOUR_ID)) {
  m_hwnd = 0;
}

struct menuentry {
  int paramNum; // enum value in Parameters.h for setting store
  int idcNum;   // #define value in resource.h for dasher.rc
};

// List of menu items that will be displayed in the General Preferences
static menuentry menutable[] = {
  {BP_START_MOUSE, IDC_LEFT},
  {BP_START_SPACE, IDC_SPACE},
//  {BP_WINDOW_PAUSE, IDC_WINDOWPAUSE},
  {BP_MOUSEPOS_MODE, IDC_MOUSEPOS},
  {BP_NUMBER_DIMENSIONS, IDC_1D},
  {BP_EYETRACKER_MODE, IDC_EYETRACKER},
 // {BP_KEY_CONTROL, IDC_BUTTON},
 // {BP_OUTLINE_MODE, IDC_OUTLINE},
 // {BP_DRAW_MOUSE, IDC_DRAWMOUSE},
 // {BP_DRAW_MOUSE_LINE, IDC_DRAWMOUSELINE},
 // {BP_PALETTE_CHANGE, IDC_COLOURSCHEME},
 // {BP_TIME_STAMP, IDC_TIMESTAMP},   // Not global setting - specific to editbox/widget
//  {BP_COPY_ALL_ON_STOP, IDC_COPYALLONSTOP}, // Same
 // {BP_SPEECH_MODE, IDC_SPEECH}     // Same
};

void CControlPage::PopulateList() {
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    if(m_pDasherInterface->GetBoolParameter(menutable[ii].paramNum)) {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
    }
    else  {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_UNCHECKED, 0);
    }
  }
}

bool CControlPage::Validate() {
  // Return false if something is wrong to prevent user from clicking to a different page. Please also pop up a dialogue informing the user at this point.
  return TRUE;
}

bool CControlPage::Apply() {
  // Return false (and notify the user) if something is wrong.
  return TRUE;
}

LRESULT CControlPage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  NMHDR *pNMHDR;
  
  switch (message) {
  case WM_INITDIALOG:
    if(!m_hwnd) {               // If this is the initial dialog for the first time
      m_hwnd = Window;
      PopulateList();
    }
    return TRUE;
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case (IDC_DISPLAY):
      break;
    }
  case WM_NOTIFY:
    pNMHDR = (NMHDR*)lParam;
    switch (pNMHDR->code) {
    case PSN_KILLACTIVE: // About to lose focus
      SetWindowLong( pNMHDR->hwndFrom, DWL_MSGRESULT, Validate());
      return TRUE;
      break;
    case PSN_APPLY: // User clicked OK/Apply - apply the changes
      if(Apply())
        SetWindowLong( pNMHDR->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLong( pNMHDR->hwndFrom, DWL_MSGRESULT, PSNRET_INVALID);
      return TRUE;
      break;
    }
    break;
  }
  return FALSE;
}
