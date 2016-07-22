#include "StatusControl.h"
#include "WinCommon.h"
#include "../resource.h"

#include <string>

CStatusControl::CStatusControl(Dasher::CSettingsUser *pCreateFrom, CAppSettings *pAppSettings)
  : CSettingsObserver(pCreateFrom),
  m_pAppSettings(pAppSettings),
  m_dialogHeight(0) {
}

void CStatusControl::HandleEvent(int iParameter) {
	switch (iParameter) {
  case  SP_ALPHABET_ID:
  case  SP_ALPHABET_1:
  case  SP_ALPHABET_2:
  case  SP_ALPHABET_3:
  case  SP_ALPHABET_4:
		PopulateCombo();
		break;
	case LP_MAX_BITRATE:
		PopulateSpeed();
		break;
	}
}
LRESULT CStatusControl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    m_hEdit = GetDlgItem(IDC_SPEED_EDIT);
    m_hUpDown = GetDlgItem(IDC_SPEED_SPIN);
    m_hCombo = GetDlgItem(IDC_ALPHABET_COMBO);

    RECT r = { 0,0,0,14 };
    MapDialogRect(&r);
    m_dialogHeight = r.bottom;

    m_hEdit.SendMessage(EM_LIMITTEXT, (WPARAM)4, (LPARAM)0);
    m_hUpDown.SendMessage(UDM_SETRANGE, 0, (LPARAM)MAKELONG(800, 1));

    PopulateCombo();
    PopulateSpeed();
    return 1;
}

LRESULT CStatusControl::OnSpeedSpinChange(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled) {
  LPNMUPDOWN pUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

  if (pUpDown->hdr.code == UDN_DELTAPOS)
    UpdateSpeed(pUpDown->iPos, pUpDown->iDelta);
  else
    bHandled = false;
  return 0;
}

LRESULT CStatusControl::OnSpeedEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
  CString wideText;
  m_hEdit.GetWindowText(wideText);
  double dNewSpeed = _tstof(wideText);
  m_pAppSettings->SetLongParameter(LP_MAX_BITRATE, dNewSpeed * 100);
  return 0;
}

LRESULT CStatusControl::OnAlphabetChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
  SelectAlphabet();
  return 0;
}

LRESULT CStatusControl::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  RECT rc;
  GetClientRect(&rc);
  RECT comboRc;
  m_hCombo.GetWindowRect(&comboRc);
  ScreenToClient(&comboRc);
  comboRc.right = rc.right - rc.left;
  m_hCombo.MoveWindow(&comboRc, true);
  return 0;
}

void CStatusControl::PopulateCombo() {
  int iCount(SendMessage(m_hCombo, CB_GETCOUNT, 0, 0));

  for(int i(0); i < iCount; ++i)
    SendMessage(m_hCombo, CB_DELETESTRING, 0, 0);

  std::wstring strEntry;
  
  WinUTF8::UTF8string_to_wstring(m_pAppSettings->GetStringParameter(SP_ALPHABET_ID), strEntry);
  SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pAppSettings->GetStringParameter(SP_ALPHABET_1), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pAppSettings->GetStringParameter(SP_ALPHABET_2), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pAppSettings->GetStringParameter(SP_ALPHABET_3), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pAppSettings->GetStringParameter(SP_ALPHABET_4), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)L"More Alphabets...");
  
  SendMessage(m_hCombo, CB_SETCURSEL, 0, 0);
}

void CStatusControl::SelectAlphabet() {
  int iIndex(SendMessage(m_hCombo, CB_GETCURSEL, 0, 0));
  int iLength(SendMessage(m_hCombo, CB_GETLBTEXTLEN, iIndex, 0));

  TCHAR *szSelection = new TCHAR[iLength + 1];
  SendMessage(m_hCombo, CB_GETLBTEXT, iIndex, (LPARAM)szSelection);

  if(!_tcscmp(szSelection, L"More Alphabets...")) {
    GetParent().SendMessage(WM_COMMAND, ID_OPTIONS_PREFS, 0);
  }
  else {
    std::string strNewValue;
    WinUTF8::wstring_to_UTF8string(szSelection, strNewValue);

    m_pAppSettings->SetStringParameter(SP_ALPHABET_ID, strNewValue);
  }

  delete[] szSelection;
}

void CStatusControl::PopulateSpeed() {
  int iValue(m_pAppSettings->GetLongParameter(LP_MAX_BITRATE));

  TCHAR Buffer[10];
  _stprintf(Buffer, TEXT("%0.2f"), iValue / 100.0);
  SendMessage(m_hEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);
  SendMessage(m_hUpDown, UDM_SETPOS, 0, (LPARAM) MAKELONG ((short)iValue, 0));
}

void CStatusControl::UpdateSpeed(int iPos, int iDelta) {
  int iValue(iPos + iDelta);

  if(iValue > 800)
    iValue = 800;

  if(iValue < 1)
    iValue = 1;

  TCHAR Buffer[10];
  _stprintf(Buffer, TEXT("%0.2f"), iValue / 100.0);
  SendMessage(m_hEdit, WM_SETTEXT, 0, (LPARAM)(LPCSTR)Buffer);

  m_pAppSettings->SetLongParameter(LP_MAX_BITRATE, iValue);

}
