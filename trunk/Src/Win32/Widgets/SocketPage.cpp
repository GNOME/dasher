// SocketPage.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "SocketPage.h"
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

CSocketPage::CSocketPage(HWND Parent, CDasherInterfaceBase *DI, CAppSettings *pAppSettings)
:CPrefsPageBase(Parent, DI, pAppSettings) {
}

void CSocketPage::PopulateList() {

    if(m_pAppSettings->GetBoolParameter(BP_SOCKET_INPUT_ENABLE)) {
      SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_ENABLE), BM_SETCHECK, BST_CHECKED, 0);
    }
    else  {
      SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_ENABLE), BM_SETCHECK, BST_UNCHECKED, 0);
    }
    
    const int buflen=256; // code below assumes this is > 100
    TCHAR m_tcBuffer[buflen];
    _sntprintf(m_tcBuffer, 100, _T("%ld"), m_pAppSettings->GetLongParameter(LP_SOCKET_PORT));
    SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_PORT), WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    
    mbstowcs(m_tcBuffer, m_pAppSettings->GetStringParameter(SP_SOCKET_INPUT_X_LABEL).c_str(), buflen-1);
    // Usually mbstowcs will terminate the target sting, except if we reached the max
    // no. of chars, in which case we terminate ourselves:
    m_tcBuffer[buflen-1] = 0; // terminate 
    SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_X_LABEL), WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    mbstowcs(m_tcBuffer, m_pAppSettings->GetStringParameter(SP_SOCKET_INPUT_Y_LABEL).c_str(), buflen-1);
    m_tcBuffer[buflen-1] = 0; // terminate 
    SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_Y_LABEL), WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    _sntprintf(m_tcBuffer, 100, _T("%.3f"), m_pAppSettings->GetLongParameter(LP_SOCKET_INPUT_X_MIN)/1000.0);
    SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_X_MIN), WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
    
    _sntprintf(m_tcBuffer, 100, _T("%.3f"), m_pAppSettings->GetLongParameter(LP_SOCKET_INPUT_X_MAX)/1000.0);
    SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_X_MAX), WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
    
    _sntprintf(m_tcBuffer, 100, _T("%.3f"), m_pAppSettings->GetLongParameter(LP_SOCKET_INPUT_Y_MIN)/1000.0);
    SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_Y_MIN), WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
    
    _sntprintf(m_tcBuffer, 100, _T("%.3f"), m_pAppSettings->GetLongParameter(LP_SOCKET_INPUT_Y_MAX)/1000.0);
    SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_Y_MAX), WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
    
    SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_DEBUG), BM_SETCHECK, m_pAppSettings->GetBoolParameter(BP_SOCKET_DEBUG) ? BST_CHECKED : BST_UNCHECKED, 0);
    
}

bool CSocketPage::Validate() {
  OutputDebugString(TEXT("Validate() called\n"));
  // Return false if something is wrong to prevent user from clicking to a different page. Please also pop up a dialogue informing the user at this point.
  return validateTextBoxes(FALSE, FALSE); // don't apply; don't inhibit error messages
}

bool CSocketPage::Apply() {
  OutputDebugString(TEXT("Apply() called\n"));
  m_pAppSettings->SetBoolParameter(BP_SOCKET_INPUT_ENABLE, SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_ENABLE), BM_GETCHECK, 0, 0));
  m_pAppSettings->SetBoolParameter(BP_SOCKET_DEBUG, SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_DEBUG), BM_GETCHECK, 0, 0));  
  return validateTextBoxes(TRUE, FALSE);
  
}


bool CSocketPage::validateTextBoxes(bool apply, bool noerror) {
  
  // && short-circuits here, so the processing will abort after the first validation error
  
  return checkPort(apply, noerror)
    && checkLabel(apply, noerror, SP_SOCKET_INPUT_X_LABEL, IDC_SOCKET_X_LABEL)
    && checkLabel(apply, noerror, SP_SOCKET_INPUT_Y_LABEL, IDC_SOCKET_Y_LABEL)
    && checkMinOrMax(apply, noerror, LP_SOCKET_INPUT_X_MIN, IDC_SOCKET_X_MIN)
    && checkMinOrMax(apply, noerror, LP_SOCKET_INPUT_X_MAX, IDC_SOCKET_X_MAX)
    && checkMinOrMax(apply, noerror, LP_SOCKET_INPUT_Y_MIN, IDC_SOCKET_Y_MIN)
    && checkMinOrMax(apply, noerror, LP_SOCKET_INPUT_Y_MAX, IDC_SOCKET_Y_MAX);
}

bool CSocketPage::checkPort(bool apply, bool noerror) {
  long longval;
  TCHAR m_tcBuffer[256];
  SendMessage(GetDlgItem(m_hwnd, IDC_SOCKET_PORT), WM_GETTEXT, 100, (LPARAM) m_tcBuffer);
  if(_stscanf(m_tcBuffer, TEXT("%ld"), &longval) != 1 || longval < 1 || longval > 65535) {
    if(!noerror) {
      Tstring ErrMsg, ErrTitle;
      WinLocalisation::GetResourceString(IDS_SOCKET_ERROR_TITLE, &ErrTitle);
      WinLocalisation::GetResourceString(IDS_ERR_SOCKET_PORT, &ErrMsg);
      MessageBox(m_hwnd, ErrMsg.c_str(), ErrTitle.c_str(), MB_OK | MB_ICONEXCLAMATION);
    }
    return FALSE;
  }
  if(apply) {
    m_pAppSettings->SetLongParameter(LP_SOCKET_PORT, longval);
  }
  return TRUE;
}

bool CSocketPage::checkMinOrMax(bool apply, bool noerror, int paramID, int idc) {
  double doubleval;

  TCHAR m_tcBuffer[256];
  SendMessage(GetDlgItem(m_hwnd, idc), WM_GETTEXT, 100, (LPARAM) m_tcBuffer);
  if(_stscanf(m_tcBuffer, TEXT("%lf"), &doubleval) != 1) {
    if(!noerror) {
      Tstring ErrMsg, ErrTitle;
      WinLocalisation::GetResourceString(IDS_SOCKET_ERROR_TITLE, &ErrTitle);
      WinLocalisation::GetResourceString(IDS_ERR_SOCKET_MINMAX, &ErrMsg);
      MessageBox(m_hwnd, ErrMsg.c_str(), ErrTitle.c_str(), MB_OK | MB_ICONEXCLAMATION);
    }
    return FALSE;
  }
  if(apply) {
    m_pAppSettings->SetLongParameter(paramID, (long) (doubleval * 1000.0));
  }
  return TRUE;
}

bool CSocketPage::checkLabel(bool apply, bool noerror, int paramID, int idc) {
  TCHAR m_tcBuffer[256];
  SendMessage(GetDlgItem(m_hwnd, idc), WM_GETTEXT, 100, (LPARAM) m_tcBuffer);
  if(apply) {
    char cbuf[1000];
    wcstombs(cbuf, m_tcBuffer, sizeof(cbuf)-1);
    cbuf[sizeof(cbuf)-1]=0; //failsafe termination
    std::string s(cbuf);
    m_pAppSettings->SetStringParameter(paramID, s);
  }
  return TRUE;
}