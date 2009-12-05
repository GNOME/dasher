//#ifndef __ModuleSettings_h__
//#define __ModuleSettings_h__

#include "../DasherCore/DasherInterfaceBase.h"
#include "ModuleControl.h"

#include <atlbase.h>
#include <atlwin.h>
#include <string>

extern CONST UINT WM_MS_CLOSE;
#define _WM_MS_CLOSE (LPCWSTR)"wm_ms_close"

class CModuleSettings : public CWindowImpl<CModuleSettings> {
public:
  CModuleSettings(const std::string &strModuleName, SModuleSettings *pSettings, int iCount, Dasher::CDasherInterfaceBase *pInterface);
  ~CModuleSettings();

  void Create(HWND hWndParent, ATL::_U_RECT rect);

  DECLARE_WND_CLASS(NULL);

  BEGIN_MSG_MAP(CModuleSettings)
    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
  END_MSG_MAP()

  LRESULT OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnPaint(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  virtual void OnFinalMessage(HWND hWnd) {
    delete this;
  }

private:
  int m_iCount;
  CModuleControl **m_pControls;
  std::string m_strModuleName;

  HWND m_hParent;

  HWND m_hOk;
  HWND m_hCancel;

  Dasher::CDasherInterfaceBase *m_pInterface;
};

//#endif