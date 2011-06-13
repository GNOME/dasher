// PrefsPageBase.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "PrefsPageBase.h"
//#include "../resource.h"

//#include <utility>              // for std::pair

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

CPrefsPageBase::CPrefsPageBase(HWND Parent, CAppSettings *pAppSettings)
: m_pAppSettings(pAppSettings) {
  m_hwnd = 0;
  m_hPropertySheet = 0;
}

LRESULT CPrefsPageBase::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  
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
    switch (HIWORD(wParam)) {
    case BN_CLICKED:
    case EN_CHANGE:
    //case CBN_SELCHANGE: // value is same as LBN_SELCHANGE
    case LBN_SELCHANGE:
      if(m_hPropertySheet != 0 && m_hwnd != 0) {
        PropSheet_Changed(m_hPropertySheet, m_hwnd); // enables the 'Apply' button
        return TRUE;
      }
      break;
    }
    break;
  case WM_NOTIFY:
    pNMHDR = (NMHDR*)lParam;
    if(m_hPropertySheet==0) {
      m_hPropertySheet = pNMHDR->hwndFrom;
    }
    switch (pNMHDR->code) {
    //case PSN_SETACTIVE:
    //  if(!m_hwnd) {               // If this is the initial dialog for the first time
    //    m_hwnd = Window;
    //    PopulateList();
    //  }
    //  return TRUE;
    //  break;
    case PSN_KILLACTIVE: // About to lose focus
      SetWindowLong( Window, DWL_MSGRESULT, !Validate());
      return TRUE;
      break;
    case PSN_APPLY: // User clicked OK/Apply - apply the changes
      if(Apply())
        SetWindowLong( Window, DWL_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLong( Window, DWL_MSGRESULT, PSNRET_INVALID);
      return TRUE;
      break;
    }
    break;
  }
  return FALSE;
}

bool CPrefsPageBase::Validate() {
  return TRUE;
}
