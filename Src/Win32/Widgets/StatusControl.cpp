#include "StatusControl.h"
#include "WinCommon.h"

#include <string>

// TODO: Make this a notify?
CONST UINT DASHER_SHOW_PREFS = RegisterWindowMessage(_DASHER_SHOW_PREFS);

CStatusControl::CStatusControl(Dasher::CDasherInterfaceBase *pDasherInterface) {
  m_pDasherInterface = pDasherInterface;
}

// TODO: ATL has more sophisticated handlers for conrol and notify messages - consider using them instead
LRESULT CStatusControl::OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  switch(HIWORD(wParam)) {
    case CBN_SELCHANGE:
      SelectAlphabet();
      break;
    default:
      bHandled = false;
      break;
  }

  return 0;
}

LRESULT CStatusControl::OnNotify(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  NMHDR *pNotify((LPNMHDR)lParam);

  switch(pNotify->code) {
    case UDN_DELTAPOS:
      UpdateSpeed(((LPNMUPDOWN) lParam)->iPos, ((LPNMUPDOWN) lParam)->iDelta);
      break;
    case EN_CHANGE:
      {
        TCHAR wszBuffer[32];
        SendMessage(m_hEdit, WM_GETTEXT, 32, (long)wszBuffer);
        double dNewSpeed = _tstof(wszBuffer);

        m_pDasherInterface->SetLongParameter(LP_MAX_BITRATE, dNewSpeed * 100);
      }
      break;
    default:
      bHandled = false;
      break;
  }

  return 0;
}

LRESULT CStatusControl::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  LayoutChildrenUpdate();
  return 0;
}

HWND CStatusControl::Create(HWND hParent) {
  CWindowImpl<CStatusControl>::Create(hParent, 0, 0, WS_VISIBLE | WS_CHILD );

  CreateChildren();

  PopulateCombo();
  PopulateSpeed();

  LayoutChildrenInitial();

  return m_hWnd;
}

void CStatusControl::CreateChildren() {
  // First create the two static labels.
  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);

  std::wstring strSpeedLabel(L"Speed:");
  std::wstring strAlphabetLabel(L"Alphabet:");

  // TODO: Wrap windows here in CWindow classes.
#ifndef _WIN32_WCE
  // TODO: Is this really needed?
  m_hSpeedLabel = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), strSpeedLabel.c_str(), 
      SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, NULL, WinHelper::hInstApp, NULL);
#else
  m_hSpeedLabel = CreateWindowEx(0, TEXT("STATIC"), strSpeedLabel.c_str(), 
      SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, NULL, WinHelper::hInstApp, NULL);
#endif

#ifndef _WIN32_WCE
  m_hAlphabetLabel = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), strAlphabetLabel.c_str(), 
      SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, NULL, WinHelper::hInstApp, NULL);
#else
  m_hAlphabetLabel = CreateWindowEx(0, TEXT("STATIC"), strAlphabetLabel.c_str(), 
      SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, NULL, WinHelper::hInstApp, NULL);
#endif

  SendMessage(m_hSpeedLabel, WM_SETFONT, (WPARAM) hGuiFont, true);
  SendMessage(m_hAlphabetLabel, WM_SETFONT, (WPARAM) hGuiFont, true);

  SIZE sSize;

  HDC hSpeedDC(::GetDC(m_hSpeedLabel));
  SelectObject(hSpeedDC, hGuiFont);
  GetTextExtentPoint32(hSpeedDC, strSpeedLabel.c_str(), strSpeedLabel.size(), &sSize);
  ::MoveWindow(m_hSpeedLabel, 0, 0, sSize.cx + 4, sSize.cy, false);
  ::ReleaseDC(m_hSpeedLabel, hSpeedDC);

  HDC hAlphabetDC(::GetDC(m_hAlphabetLabel));
  SelectObject(hAlphabetDC, hGuiFont);
  GetTextExtentPoint32(hAlphabetDC, strAlphabetLabel.c_str(), strAlphabetLabel.size(), &sSize);
  ::MoveWindow(m_hAlphabetLabel, 0, 0, sSize.cx + 4, sSize.cy, false);
  ::ReleaseDC(m_hAlphabetLabel, hAlphabetDC);

  // Next the edit box and up/down control
  m_hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_GROUP, 0, 0, 0, CW_USEDEFAULT, m_hWnd, NULL, WinHelper::hInstApp, NULL);
  SendMessage(m_hEdit, EM_LIMITTEXT, (WPARAM) 4, (LPARAM) 0);

  HDC hdc = ::GetDC(m_hEdit);
  TEXTMETRIC tmGui;
  GetTextMetrics(hdc, &tmGui);
  SendMessage(m_hEdit, WM_SETFONT, (WPARAM) hGuiFont, true);
  ::ReleaseDC(m_hEdit, hdc);

  m_iEditHeight = tmGui.tmHeight + (GetSystemMetrics(SM_CYEDGE) * 2);
  m_iEditWidth = tmGui.tmAveCharWidth * 7;
  ::MoveWindow(m_hEdit, 0, 0, m_iEditWidth, m_iEditHeight, false);
 
  m_hUpDown = CreateWindowEx(WS_EX_CLIENTEDGE, UPDOWN_CLASS, TEXT(""), UDS_ALIGNRIGHT | WS_CHILD  | WS_TABSTOP |WS_VISIBLE |  WS_GROUP, 0, 0, 16, 16, m_hWnd, NULL, WinHelper::hInstApp, NULL);
  SendMessage(m_hUpDown, UDM_SETRANGE, 0, (LPARAM) MAKELONG(800, 1));

  // And finally the combo box
  m_hCombo = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("COMBOBOX"), TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VISIBLE |WS_GROUP | ES_READONLY, 0, 0, 128, 128, m_hWnd, NULL, WinHelper::hInstApp, NULL);
  SendMessage(m_hCombo, WM_SETFONT, (WPARAM) hGuiFont, true);

  DeleteObject(hGuiFont);

  // Set the height to what is finally required. The extra pixel is needed to get everything to line up nicely
  MoveWindow(0, 0, 0, m_iEditHeight+1, false);
}

void CStatusControl::LayoutChildrenInitial() {
  RECT rc;
  GetWindowRect(&rc);

  // And how much is needed for the various bits and pieces
  RECT sRect;
  
  ::GetWindowRect(m_hSpeedLabel, &sRect);
  int iSpeedLabelWidth(sRect.right - sRect.left);  
  int iSpeedLabelHeight(sRect.bottom - sRect.top);
  
  ::GetWindowRect(m_hAlphabetLabel, &sRect);
  int iAlphabetLabelWidth(sRect.right - sRect.left);  
  int iAlphabetLabelHeight(sRect.bottom - sRect.top);
  
  ::GetWindowRect(m_hEdit, &sRect);
  int iEditHeight(sRect.bottom - sRect.top);
  
  ::GetWindowRect(m_hCombo, &sRect);
  int iComboWidth(sRect.right - sRect.left);  
  int iComboHeight(sRect.bottom - sRect.top);

  // Finally, do the layout
  // TODO: This isn't necessary - the only thing which is going to change is the width of the combo
  int iPosition(0);

  ::MoveWindow(m_hSpeedLabel, iPosition, (iEditHeight - iSpeedLabelHeight) / 2, iSpeedLabelWidth, iSpeedLabelHeight, TRUE);
  iPosition += iSpeedLabelWidth + 2;

  ::MoveWindow(m_hEdit, iPosition, 0, m_iEditWidth, iEditHeight, TRUE);
  ::SendMessage(m_hUpDown, UDM_SETBUDDY, (WPARAM)m_hEdit, 0);
  iPosition += m_iEditWidth + 2;

  ::MoveWindow(m_hAlphabetLabel, iPosition, (iEditHeight - iAlphabetLabelHeight) / 2, iAlphabetLabelWidth, iAlphabetLabelHeight, TRUE);
  iPosition += iAlphabetLabelWidth + 2;

  ::MoveWindow(m_hCombo, iPosition, 0, rc.right - rc.left - iPosition, iComboHeight, TRUE);
}

void CStatusControl::LayoutChildrenUpdate() {
  // For now don't do anything special here
  LayoutChildrenInitial();
}

void CStatusControl::PopulateCombo() {
  int iCount(SendMessage(m_hCombo, CB_GETCOUNT, 0, 0));

  for(int i(0); i < iCount; ++i)
    SendMessage(m_hCombo, CB_DELETESTRING, 0, 0);

  std::wstring strEntry;
  
  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_ID), strEntry);
  SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_1), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_2), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_3), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_4), strEntry);
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
    SendMessage(GetParent().m_hWnd, DASHER_SHOW_PREFS, 0, 0);
  }
  else {
    std::string strNewValue;
    WinUTF8::wstring_to_UTF8string(szSelection, strNewValue);

    m_pDasherInterface->SetStringParameter(SP_ALPHABET_ID, strNewValue);
  }

  delete[] szSelection;
}

void CStatusControl::PopulateSpeed() {
  int iValue(m_pDasherInterface->GetLongParameter(LP_MAX_BITRATE));

  TCHAR *Buffer = new TCHAR[10];
  _stprintf(Buffer, TEXT("%0.2f"), iValue / 100.0);
  SendMessage(m_hEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);
  delete[]Buffer;

  SendMessage(m_hUpDown, UDM_SETPOS, 0, (LPARAM) MAKELONG ((short)iValue, 0));
}

void CStatusControl::UpdateSpeed(int iPos, int iDelta) {
  int iValue(iPos + iDelta);

  if(iValue > 800)
    iValue = 800;

  if(iValue < 1)
    iValue = 1;

  TCHAR *Buffer = new TCHAR[10];
  _stprintf(Buffer, TEXT("%0.2f"), iValue / 100.0);
  SendMessage(m_hEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);
  delete[]Buffer;

  m_pDasherInterface->SetLongParameter(LP_MAX_BITRATE, iValue);
}
