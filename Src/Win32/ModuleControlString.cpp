#include "ModuleControlString.h"

int CModuleControlString::GetHeightRequest() {
  return 14;
}

void CModuleControlString::Initialise(Dasher::CDasherInterfaceBase *pInterface) {
  std::wstring strText;
  WinUTF8::UTF8string_to_wstring(pInterface->GetStringParameter(m_iId), strText);
  SendMessage(m_hEntry, WM_SETTEXT, 0, (LPARAM)strText.c_str());
}

void CModuleControlString::Apply(Dasher::CDasherInterfaceBase *pInterface) {
  TCHAR tcBuffer[256];
  SendMessage(m_hEntry, WM_GETTEXT, 100, (LPARAM)tcBuffer);
  
  std::string strUTF8Text;
  WinUTF8::wstring_to_UTF8string(tcBuffer, strUTF8Text);

  pInterface->SetStringParameter(m_iId, strUTF8Text);
}

void CModuleControlString::CreateChild(HWND hParent) {
  m_hEntry = CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, 
    WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hParent, NULL, WinHelper::hInstApp, NULL);

  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);
 
  SendMessage(m_hEntry, WM_SETFONT, (WPARAM)hGuiFont, (LPARAM)true);
}

void CModuleControlString::LayoutChild(RECT &sRect) {
  ::MoveWindow(m_hEntry, sRect.left, sRect.top, sRect.right - sRect.left, sRect.bottom - sRect.top, TRUE);
}
