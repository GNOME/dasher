// AlphabetBox.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "LMPage.h"
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

CLMPage::CLMPage(HWND Parent, CDasherInterface *DI, CAppSettings *pAppSettings)
:m_pDasherInterface(DI), m_pAppSettings(pAppSettings) {
  m_hwnd = 0;

  
}

void CLMPage::PopulateList() {

    slider = GetDlgItem(m_hwnd, IDC_UNIFORMSLIDER);
    SendMessage(slider, TBM_SETPAGESIZE, 0L, 20); // PgUp and PgDown change bitrate by reasonable amount
    SendMessage(slider, TBM_SETTICFREQ, 10, 0L);
    SendMessage(slider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(0, 1000));
  
    uniformbox = GetDlgItem(m_hwnd, IDC_UNIFORMVAL);  

    if(m_pAppSettings->GetBoolParameter(BP_LM_ADAPTIVE)) {
      SendMessage(GetDlgItem(m_hwnd, IDC_ADAPTIVE), BM_SETCHECK, BST_CHECKED, 0);
    }
    else  {
      SendMessage(GetDlgItem(m_hwnd, IDC_ADAPTIVE), BM_SETCHECK, BST_UNCHECKED, 0);
    }

    SendMessage(slider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_UNIFORM));

    _sntprintf(m_tcBuffer, 100, TEXT("%0.1f"), m_pAppSettings->GetLongParameter(LP_UNIFORM) / 10.0);
    SendMessage(uniformbox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

  switch(m_pAppSettings->GetLongParameter(LP_LANGUAGE_MODEL_ID)) {
    case 0:
      SendMessage(GetDlgItem(m_hwnd, IDC_LM_PPM), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case 2:
      SendMessage(GetDlgItem(m_hwnd, IDC_LM_WORD), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case 3:
      SendMessage(GetDlgItem(m_hwnd, IDC_LM_MIXTURE), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case 4:
      SendMessage(GetDlgItem(m_hwnd, IDC_LM_JAPANESE), BM_SETCHECK, BST_CHECKED, 0);
      break;
   }

}

bool CLMPage::Validate() {
  // Return false if something is wrong to prevent user from clicking to a different page. Please also pop up a dialogue informing the user at this point.
  return TRUE;
}

bool CLMPage::Apply() {
  m_pAppSettings->SetBoolParameter( BP_LM_ADAPTIVE, SendMessage(GetDlgItem(m_hwnd, IDC_ADAPTIVE), BM_GETCHECK, 0, 0));
  
  double NewUniform;
  NewUniform = SendMessage(slider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_UNIFORM, NewUniform);

  if(SendMessage(GetDlgItem(m_hwnd, IDC_LM_PPM), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_LANGUAGE_MODEL_ID, 0);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_LM_WORD), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_LANGUAGE_MODEL_ID, 2);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_LM_MIXTURE), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_LANGUAGE_MODEL_ID, 3);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_LM_JAPANESE), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_LANGUAGE_MODEL_ID, 4);

  // Return false (and notify the user) if something is wrong.
  return TRUE;
}

LRESULT CLMPage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  
  NMHDR *pNMHDR;
  double NewUniform;

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
    break;
  case WM_HSCROLL:
    if((LOWORD(wParam) == SB_THUMBPOSITION) | (LOWORD(wParam) == SB_THUMBTRACK)) {
      // Some messages give the new postion
      NewUniform = HIWORD(wParam);
    }
    else {
      // Otherwise we have to ask for it
      long Pos = SendMessage(slider, TBM_GETPOS, 0, 0);
      NewUniform = Pos;
    }
    {
      _sntprintf(m_tcBuffer, 100, TEXT("%0.1f"), NewUniform / 10);
      SendMessage(uniformbox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
    }
    return TRUE;
    break;
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
