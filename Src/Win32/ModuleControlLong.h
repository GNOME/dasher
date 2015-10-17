#ifndef __ModuleControlLong_h__
#define __ModuleControlLong_h__

#include "ModuleControl.h"

class CModuleControlLong : public CModuleControl {
public:
  CModuleControlLong(SModuleSettings *pSetting, bool bShowSlider) : CModuleControl(pSetting), m_bShowSlider(bShowSlider) {};

  BEGIN_MSG_MAP(CModuleControlLong)
    COMMAND_HANDLER(1, EN_CHANGE, OnEditChange)
    COMMAND_HANDLER(1, EN_KILLFOCUS, OnEditLeft)
    MESSAGE_HANDLER(WM_HSCROLL, OnSliderScroll)
  END_MSG_MAP()
  LRESULT OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnEditLeft(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnSliderScroll(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  virtual int GetChildHeight() { return 14; };
  virtual void Initialise(CAppSettings *pAppSets);
  virtual void Apply(CAppSettings *pAppSets);
  virtual void CreateChild(HWND hParent, RECT& rect);

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