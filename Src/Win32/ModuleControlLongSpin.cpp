#include "ModuleControlLongSpin.h"

int CModuleControlLongSpin::GetHeightRequest() {
  return 12;
}

void CModuleControlLongSpin::Initialise(CAppSettings *pAppSets) {
  double dValue = pAppSets->GetLongParameter(m_iId)*1.0 / m_iDivisor;
  CString sValue;
  sValue.Format(TEXT("%g"), dValue);
  m_hEntry.SetWindowText(sValue);
}

void CModuleControlLongSpin::Apply(CAppSettings *pAppSets) {
  CString sValue;
  m_hEntry.GetWindowText(sValue);
  long lValue = wcstod(sValue, 0)*m_iDivisor;
  if (lValue < m_iMin)
    lValue = m_iMin;
  else if (lValue > m_iMax)
    lValue = m_iMax;

  pAppSets->SetLongParameter(m_iId, lValue);
}

void CModuleControlLongSpin::CreateChild(HWND hParent) {
  m_hEntry.Create(TEXT("EDIT"), hParent, 0, 0,
    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, WS_EX_CONTROLPARENT | WS_EX_CLIENTEDGE);
  m_hEntry.SetFont(m_hEntry.GetParent().GetFont());
}

void CModuleControlLongSpin::LayoutChild(RECT &sRect) {
  m_hEntry.MoveWindow(&sRect);
}
