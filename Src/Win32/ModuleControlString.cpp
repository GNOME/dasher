#include "ModuleControlString.h"

int CModuleControlString::GetHeightRequest() {
  return 14;
}

void CModuleControlString::Initialise(CAppSettings *pAppSets) {
  std::wstring wideText;
  WinUTF8::UTF8string_to_wstring(pAppSets->GetStringParameter(m_iId), wideText);
  m_hEntry.SetWindowText(wideText.c_str());
}

void CModuleControlString::Apply(CAppSettings *pAppSets) {
  CString wideText;
  m_hEntry.GetWindowText(wideText);
  std::string strUTF8Text;
  WinUTF8::wstring_to_UTF8string(std::wstring(wideText), strUTF8Text);

  pAppSets->SetStringParameter(m_iId, strUTF8Text);
}

void CModuleControlString::CreateChild(HWND hParent) {
  m_hEntry.Create(TEXT("EDIT"), hParent, 0, 0,
    WS_CHILD | WS_VISIBLE | WS_TABSTOP, WS_EX_CLIENTEDGE);
}

void CModuleControlString::LayoutChild(RECT &sRect) {
  m_hEntry.MoveWindow(&sRect);
}
