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
  UpdateValue(GetValue());
  return 0;
}

void CModuleControlLong::Initialise(CAppSettings *pAppSets) {
  UpdateValue(pAppSets->GetLongParameter(m_iId));
}

void CModuleControlLong::Apply(CAppSettings *pAppSets) {
  pAppSets->SetLongParameter(m_iId, GetValue());
}

void CModuleControlLong::CreateChild(HWND hParent, RECT& rect) {
  RECT editRect = rect;

  if (m_bShowSlider) {
    RECT sliderRect = editRect;
    sliderRect.left = editRect.left + (editRect.right - editRect.left)/4;
    editRect.right = sliderRect.left;

    m_hSlider.Create(TRACKBAR_CLASS, *this, sliderRect, 0, TBS_HORZ | WS_CHILD | WS_VISIBLE | WS_TABSTOP);
    m_hSlider.SendMessage(TBM_SETPAGESIZE, 0, m_iStep);
    m_hSlider.SendMessage(TBM_SETRANGEMIN, true, m_iMin);
    m_hSlider.SendMessage(TBM_SETRANGEMAX, true, m_iMax);
  }
  m_hEdit.Create(TEXT("EDIT"), *this, editRect, 0,
    WS_CHILD | WS_VISIBLE | WS_TABSTOP, WS_EX_CLIENTEDGE, 1);

}

void CModuleControlLong::UpdateValue(long lValue) {
  if (m_hSlider && GetSliderValue() != lValue) {
    m_hSlider.SendMessage(TBM_SETPOS, true, lValue);
  }
  if (!m_hEdit.GetWindowTextLength() || GetEditValue() != lValue)
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

long CModuleControlLong::GetValue() {
  if (m_hSlider)
    return GetSliderValue();

  long lValue = GetEditValue();
  if (lValue < m_iMin) lValue = m_iMin;
  if (lValue > m_iMax) lValue = m_iMax;
  return lValue;
}
