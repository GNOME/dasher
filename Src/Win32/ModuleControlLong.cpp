#include "ModuleControlLong.h"

#include <cstring>

LRESULT CModuleControlLong::OnScroll(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  int iValue = SendMessage(m_hSlider, TBM_GETPOS, 0, 0);

  WCHAR tcBuffer[256];  
  _sntprintf(tcBuffer, 100, TEXT("%0.2f"), iValue / static_cast<double>(m_iDivisor));
  SendMessage(m_hEntry, WM_SETTEXT, 0, (LPARAM) tcBuffer);

  bHandled = true;
  return 0;
}

int CModuleControlLong::GetHeightRequest() {
  return 12;
}

void CModuleControlLong::Initialise(Dasher::CDasherInterfaceBase *pInterface) {
  int iValue(pInterface->GetLongParameter(m_iId));
  SendMessage(m_hSlider, TBM_SETPOS, (WPARAM)false, (LPARAM)iValue);

  WCHAR tcBuffer[256];
  _sntprintf(tcBuffer, 100, TEXT("%0.2f"), iValue / static_cast<double>(m_iDivisor));
  SendMessage(m_hEntry, WM_SETTEXT, 0, (LPARAM) tcBuffer);
}

void CModuleControlLong::Apply(Dasher::CDasherInterfaceBase *pInterface) {
  int iValue = SendMessage(m_hSlider, TBM_GETPOS, 0, 0);
  pInterface->SetLongParameter(m_iId, iValue);
}

void CModuleControlLong::CreateChild(HWND hParent) {
  m_hSlider = CreateWindowEx(WS_EX_CONTROLPARENT, TRACKBAR_CLASS, NULL, 
    TBS_HORZ | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hParent, NULL, WinHelper::hInstApp, NULL);

  SendMessage(m_hSlider, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(m_iMin, m_iMax));
  SendMessage(m_hSlider, TBM_SETPAGESIZE, 0, (LPARAM)m_iStep);

  m_hEntry = CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, 
    WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hParent, NULL, WinHelper::hInstApp, NULL);

  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);
 
  SendMessage(m_hEntry, WM_SETFONT, (WPARAM)hGuiFont, (LPARAM)true);
}

void CModuleControlLong::LayoutChild(RECT &sRect) {
  ::MoveWindow(m_hSlider, sRect.left + 32, sRect.top, sRect.right - sRect.left - 32, sRect.bottom - sRect.top, TRUE);
  ::MoveWindow(m_hEntry, sRect.left, sRect.top, 32, sRect.bottom - sRect.top, TRUE);
}
