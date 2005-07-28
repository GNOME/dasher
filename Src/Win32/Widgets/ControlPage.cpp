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

void CControlPage::PopulateList() {
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
