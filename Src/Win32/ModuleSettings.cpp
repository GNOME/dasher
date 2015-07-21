#include "ModuleControlBool.h"
#include "ModuleControlLong.h"
#include "ModuleControlString.h"
#include "ModuleSettings.h"

CModuleSettings::CModuleSettings(const std::string &strModuleName, SModuleSettings *pSettings, int iCount, CAppSettings *pAppSets) {
  m_iCount = iCount;
  m_pControls = new CModuleControl*[m_iCount];
  m_pAppSets = pAppSets;
  m_strModuleName = strModuleName;

  for (int i(0); i < m_iCount; ++i) {
    switch (pSettings[i].iType) {
    case T_BOOL:
      m_pControls[i] = new CModuleControlBool(pSettings + i);
      break;
    case T_LONG:
      m_pControls[i] = new CModuleControlLong(pSettings + i, true);
      break;
    case T_LONGSPIN:
      m_pControls[i] = new CModuleControlLong(pSettings + i, false);
      break;
    case T_STRING:
      m_pControls[i] = new CModuleControlString(pSettings + i);
      break;
    }

  }
}

CModuleSettings::~CModuleSettings() {
  for (int i(0); i < m_iCount; ++i)
    delete m_pControls[i];

  delete[] m_pControls;
}

LRESULT CModuleSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  std::wstring strWName;
  WinUTF8::UTF8string_to_wstring(m_strModuleName, strWName);
  SetWindowText(strWName.c_str());

  int iDlgWidth = 7 + CModuleControl::CAPTION_WIDTH + CModuleControl::CHILD_WIDTH + 7;
  int iTop = 7;

  for (int i(0); i < m_iCount; ++i) {
    RECT controlRect = { 7, iTop, iDlgWidth - 7, iTop + m_pControls[i]->GetHeight() };
    MapDialogRect(&controlRect);

    m_pControls[i]->Create(m_hWnd, controlRect);
    m_pControls[i]->Initialise(m_pAppSets);
    iTop += m_pControls[i]->GetHeight() + 2;
  }
  iTop += -2 + 7;

  MoveButton(IDCANCEL, iDlgWidth - 57, iTop);
  MoveButton(IDOK, iDlgWidth - 2 * (57), iTop);

  RECT size = { 0, 0, iDlgWidth, iTop + 14 + 7 };
  MapDialogRect(&size);
  ResizeClient(size.right, size.bottom);

  SendMessageToDescendants(WM_SETFONT, (WPARAM)GetFont(), true);
  return 1;
}

LRESULT  CModuleSettings::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
  if (wID == IDOK) {
    // Apply and close 
    for (int i(0); i < m_iCount; ++i) {
      m_pControls[i]->Apply(m_pAppSets);
    }
  }
  EndDialog(wID);
  return 0;
}

void CModuleSettings::MoveButton(WORD wID, int x, int y){

  RECT r = { x, y, x + 50, y + 14 };
  MapDialogRect(&r);
  GetDlgItem(wID).MoveWindow(&r);
}
