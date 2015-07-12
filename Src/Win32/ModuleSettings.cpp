#include "ModuleControlBool.h"
#include "ModuleControlLong.h"
#include "ModuleControlLongSpin.h"
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
      m_pControls[i] = new CModuleControlLong(pSettings + i);
      break;
    case T_LONGSPIN:
      m_pControls[i] = new CModuleControlLongSpin(pSettings + i);
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

  int iHeight(7);

  for (int i(0); i < m_iCount; ++i) {
    m_pControls[i]->Create(m_hWnd);
    m_pControls[i]->Initialise(m_pAppSets);
    iHeight += m_pControls[i]->GetHeightRequest() + 2;
  }
  iHeight += (-2 + 7 + 14 + 7); // - last item spacing + buttons 

  RECT size = { 0, 0, m_iDlgWidth, iHeight };
  MapDialogRect(&size);
  ResizeClient(size.right, size.bottom);
  return 1;
}

LRESULT  CModuleSettings::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled){
  if (wID == IDOK) {
    // Apply and close 
    for (int i(0); i < m_iCount; ++i) {
      m_pControls[i]->Apply(m_pAppSets);
    }
  }
  EndDialog(wID);
  return 0;
}

LRESULT CModuleSettings::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  int iCurrentTop(7);

  for (int i(0); i < m_iCount; ++i) {
    int iHeight(m_pControls[i]->GetHeightRequest());

    RECT sPosition = { 7, iCurrentTop, m_iDlgWidth - 7, iCurrentTop + iHeight };
    MapDialogRect(&sPosition);
    m_pControls[i]->Layout(&sPosition);

    iCurrentTop += iHeight + 2;
  }
  int btnX = m_iDlgWidth - (50 + 7);
  int btnY = iCurrentTop -2 + 7;
  MoveButton(IDCANCEL, btnX, btnY);
  btnX -= 50 + 7;
  MoveButton(IDOK, btnX, btnY);
  return 0;
}

void CModuleSettings::MoveButton(WORD wID, int x, int y){

  RECT r = { x, y, x + 50, y + 14 };
  MapDialogRect(&r);
  GetDlgItem(wID).MoveWindow(&r);
}
