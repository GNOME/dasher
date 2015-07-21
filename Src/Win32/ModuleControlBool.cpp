#include "ModuleControlBool.h"

void CModuleControlBool::Initialise(CAppSettings *pAppSets) {
  m_hCheckbox.SendMessage(BM_SETCHECK, pAppSets->GetBoolParameter(m_iId) ? BST_CHECKED : BST_UNCHECKED);
}

void CModuleControlBool::Apply(CAppSettings *pAppSets) {
  pAppSets->SetBoolParameter(m_iId, m_hCheckbox.SendMessage(BM_GETCHECK) == BST_CHECKED);
}

void CModuleControlBool::CreateChild(HWND hParent, RECT& rect) {
  m_hCheckbox.Create(TEXT("BUTTON"), hParent, rect, 0,
    BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP);
}
