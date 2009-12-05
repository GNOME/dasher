#include "ModuleControlLongSpin.h"

int CModuleControlLongSpin::GetHeightRequest() {
  return 12;
}

void CModuleControlLongSpin::Initialise(Dasher::CDasherInterfaceBase *pInterface) {
  int iValue(pInterface->GetLongParameter(m_iId));
  SendMessage(m_hSpin, UDM_SETPOS, 0, (LPARAM) MAKELONG ((short)iValue, 0));
  UpdateEntry(iValue, 0);
}

void CModuleControlLongSpin::Apply(Dasher::CDasherInterfaceBase *pInterface) {
  int iValue(SendMessage(m_hSpin, UDM_GETPOS, 0, 0));
  pInterface->SetLongParameter(m_iId, iValue);
}

void CModuleControlLongSpin::CreateChild(HWND hParent) {
  m_hEntry = CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, 
    WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hParent, NULL, WinHelper::hInstApp, NULL);

  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);
 
  SendMessage(m_hEntry, WM_SETFONT, (WPARAM)hGuiFont, (LPARAM)true);

  m_hSpin = CreateWindowEx(WS_EX_CLIENTEDGE, UPDOWN_CLASS, TEXT(""), 
    UDS_ALIGNRIGHT | WS_CHILD  | WS_TABSTOP |WS_VISIBLE |  WS_GROUP, 0, 0, 16, 16, 
    hParent, NULL, WinHelper::hInstApp, NULL);

  SendMessage(m_hSpin, UDM_SETRANGE, 0, (LPARAM) MAKELONG(m_iMax, m_iMin));
}

void CModuleControlLongSpin::LayoutChild(RECT &sRect) {
   ::MoveWindow(m_hEntry, sRect.left, sRect.top, sRect.right - sRect.left, sRect.bottom - sRect.top, TRUE);
   SendMessage(m_hSpin, UDM_SETBUDDY, (WPARAM)m_hEntry, 0);
}

LRESULT CModuleControlLongSpin::OnNotify(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  NMHDR *pNotify((LPNMHDR)lParam);

  switch(pNotify->code) {
    case UDN_DELTAPOS:
      UpdateEntry(((LPNMUPDOWN) lParam)->iPos, ((LPNMUPDOWN) lParam)->iDelta);
      break;
    default:
      bHandled = false;
      break;
  }

  return 0;
}

void CModuleControlLongSpin::UpdateEntry(int iValue, int iDelta) {
  WCHAR tcBuffer[256];
  _sntprintf(tcBuffer, 100, TEXT("%0.4f"), (iValue + iDelta) / static_cast<double>(m_iDivisor));
  SendMessage(m_hEntry, WM_SETTEXT, 0, (LPARAM) tcBuffer);
}
