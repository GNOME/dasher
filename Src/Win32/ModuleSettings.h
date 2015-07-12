//#ifndef __ModuleSettings_h__
//#define __ModuleSettings_h__

#include "AppSettings.h"
#include "ModuleControl.h"
#include "resource.h"

#include <atlbase.h>
#include <atlwin.h>
#include <string>

class CModuleSettings : public CDialogImpl<CModuleSettings> {
public:
  CModuleSettings(const std::string &strModuleName, SModuleSettings *pSettings, int iCount, CAppSettings *pAppSets);
  ~CModuleSettings();

  enum { IDD = IDD_MODULESETTINGS };

  BEGIN_MSG_MAP(CModuleSettings)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_RANGE_HANDLER(IDOK, IDCANCEL, OnCloseCmd)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
  END_MSG_MAP()

  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
  void MoveButton(WORD wID, int x, int y);
  int m_iCount;
  CModuleControl **m_pControls;
  std::string m_strModuleName;
  const int m_iDlgWidth = 250;

  CAppSettings *m_pAppSets;
};

//#endif