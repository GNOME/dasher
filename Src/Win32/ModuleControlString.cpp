#include "ModuleControlString.h"

void CModuleControlString::Initialise(CAppSettings *pAppSets) {
  std::wstring wideText;
  WinUTF8::UTF8string_to_wstring(pAppSets->GetStringParameter(m_iId), wideText);
  m_hEntry.SetWindowText(wideText.c_str());
}

void CModuleControlString::Apply(CAppSettings *pAppSets) {
  CString wideText;
  m_hEntry.GetWindowText(wideText);
  pAppSets->SetStringParameter(m_iId, WinUTF8::wstring_to_UTF8string(wideText));
}

void CModuleControlString::CreateChild(HWND hParent, RECT& rect) {
  m_hEntry.Create(TEXT("EDIT"), hParent, rect, 0,
    WS_CHILD | WS_VISIBLE | WS_TABSTOP, WS_EX_CLIENTEDGE);
}
