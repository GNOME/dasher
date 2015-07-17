#ifndef __ModuleControlLong_h__
#define __ModuleControlLong_h__

#include "ModuleControl.h"

class CModuleControlLong : public CModuleControl, public CWindowImpl<CModuleControlLong>{
public:
  CModuleControlLong(SModuleSettings *pSetting, bool bShowSlider) : CModuleControl(pSetting), m_bShowSlider(bShowSlider) {};
  DECLARE_WND_SUPERCLASS(NULL, L"STATIC")

  BEGIN_MSG_MAP(CModuleControlLong)
    COMMAND_HANDLER(1, EN_CHANGE, OnEditChange)
    COMMAND_HANDLER(1, EN_KILLFOCUS, OnEditLeft)
    MESSAGE_HANDLER(WM_HSCROLL, OnSliderScroll)
  END_MSG_MAP()
  LRESULT OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnEditLeft(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnSliderScroll(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  virtual int GetHeightRequest();
  virtual void Initialise(CAppSettings *pAppSets);
  virtual void Apply(CAppSettings *pAppSets);
  virtual void CreateChild(HWND hParent);
  virtual void LayoutChild(RECT &sRect);

private:
  void UpdateValue(long lValue);
  long GetEditValue();
  long GetSliderValue();
  long GetValue();
  CWindow m_hEdit;
  CWindow m_hSlider;
  bool m_bShowSlider;
};

#endif