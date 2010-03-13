#ifndef __ModuleControl_h__
#define __ModuleControl_h__

#include "Common/WinCommon.h"
#include "../DasherCore/DasherInterfaceBase.h"

#include <atlbase.h>
#include <atlwin.h>
#include <string>

class CModuleControl : public CWindowImpl<CModuleControl> {
public:
  CModuleControl(SModuleSettings *pSetting);

  DECLARE_WND_SUPERCLASS(NULL, L"STATIC")

  BEGIN_MSG_MAP(CModuleControl)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
  END_MSG_MAP()

  virtual LRESULT OnNotify(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = false;
    return 0;
  };

  virtual LRESULT OnScroll(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = false;
    return 0;
  }; 
 
  void Create(HWND hParent);
  void Layout(RECT *pRect);

  // Abstract members to be implemented by descendents
  virtual int GetHeightRequest() = 0;
  virtual void Initialise(Dasher::CDasherInterfaceBase *pInterface) = 0;
  virtual void Apply(Dasher::CDasherInterfaceBase *pInterface) = 0;
  virtual void CreateChild(HWND hParent) = 0;
  virtual void LayoutChild(RECT &sRect) = 0;

protected:
  HWND m_hParent;
  std::wstring m_strCaption;

  int m_iId;
  int m_iMin;
  int m_iMax;
  int m_iDivisor;
  int m_iStep;

  HWND m_hWndCaption;
};

#endif
