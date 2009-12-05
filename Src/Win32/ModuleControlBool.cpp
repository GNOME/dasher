#include "ModuleControlBool.h"

int CModuleControlBool::GetHeightRequest() {
  return 10;
}

void CModuleControlBool::Initialise(Dasher::CDasherInterfaceBase *pInterface) {
  if(pInterface->GetBoolParameter(m_iId))
    SendMessage(m_hCheckbox, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendMessage(m_hCheckbox, BM_SETCHECK, BST_UNCHECKED, 0);
}

void CModuleControlBool::Apply(Dasher::CDasherInterfaceBase *pInterface) {
  pInterface->SetBoolParameter(m_iId, SendMessage(m_hCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void CModuleControlBool::CreateChild(HWND hParent) {
  m_hCheckbox = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("BUTTON"), NULL, 
    BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hParent, NULL, WinHelper::hInstApp, NULL);

  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);
 
  SendMessage(m_hCheckbox, WM_SETFONT, (WPARAM)hGuiFont, (LPARAM)true);
}

void CModuleControlBool::LayoutChild(RECT &sRect) {
  ::MoveWindow(m_hCheckbox, sRect.left, sRect.top, sRect.right - sRect.left, sRect.bottom - sRect.top, TRUE);
}
