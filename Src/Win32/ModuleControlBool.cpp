#include "ModuleControlBool.h"

int CModuleControlBool::GetHeightRequest() {
  return 10;
}

void CModuleControlBool::Initialise(CAppSettings *pAppSets) {
  m_hCheckbox.SendMessage(BM_SETCHECK, pAppSets->GetBoolParameter(m_iId) ? BST_CHECKED : BST_UNCHECKED);
}

void CModuleControlBool::Apply(CAppSettings *pAppSets) {
  pAppSets->SetBoolParameter(m_iId, m_hCheckbox.SendMessage(BM_GETCHECK) == BST_CHECKED);
}

void CModuleControlBool::CreateChild(HWND hParent) {
  m_hCheckbox.Create(TEXT("BUTTON"), hParent, 0,0,
    BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP);
}

void CModuleControlBool::LayoutChild(RECT &sRect) {
  m_hCheckbox.MoveWindow(&sRect);
}
