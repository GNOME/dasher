#include "ModuleControlLong.h"

LRESULT CModuleControlLong::OnSliderScroll(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  UpdateValue(GetSliderValue());
  return 0;
}

LRESULT CModuleControlLong::OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
  UpdateValue(GetEditValue());
  return 0;
}

LRESULT CModuleControlLong::OnEditLeft(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
  UpdateValue(GetSliderValue());
  return 0;
}

int CModuleControlLong::GetHeightRequest() {
  return 12;
}

void CModuleControlLong::Initialise(CAppSettings *pAppSets) {
  UpdateValue(pAppSets->GetLongParameter(m_iId));
}

void CModuleControlLong::Apply(CAppSettings *pAppSets) {
  pAppSets->SetLongParameter(m_iId, GetSliderValue());
}

void CModuleControlLong::CreateChild(HWND hParent) {
  CWindowImpl<CModuleControlLong>::Create(hParent);

  m_hSlider.Create(TRACKBAR_CLASS, *this, 0, 0, TBS_HORZ | WS_CHILD | WS_VISIBLE | WS_TABSTOP);
  m_hSlider.SendMessage(TBM_SETPAGESIZE, 0, m_iStep);
  m_hSlider.SendMessage(TBM_SETRANGEMIN, true, m_iMin);
  m_hSlider.SendMessage(TBM_SETRANGEMAX, true, m_iMax);

  m_hEdit.Create(TEXT("EDIT"), *this, 0, 0, 
    WS_CHILD | WS_VISIBLE | WS_TABSTOP, WS_EX_CLIENTEDGE, 1);
}

void CModuleControlLong::LayoutChild(RECT &sRect) {
  MoveWindow(&sRect);
  m_hEdit.MoveWindow(0, 0, 32, sRect.bottom - sRect.top);
  m_hSlider.MoveWindow(32, 0, sRect.right - sRect.left - 32, sRect.bottom - sRect.top);
}

void CModuleControlLong::UpdateValue(long lValue) {
  if (GetSliderValue() != lValue)
  {
    m_hSlider.SendMessage(TBM_SETPOS, true, lValue);
  }
  if (GetEditValue() != lValue)
  {
    double dValue = lValue*1.0 / m_iDivisor;
    CString sValue;
    sValue.Format(TEXT("%g"), dValue);
    m_hEdit.SetWindowText(sValue);
  }
}

long CModuleControlLong::GetEditValue() {
  CString sValue;
  m_hEdit.GetWindowText(sValue);
  return wcstod(sValue, 0)*m_iDivisor;
}

long CModuleControlLong::GetSliderValue() {
  return m_hSlider.SendMessage(TBM_GETPOS);
}
