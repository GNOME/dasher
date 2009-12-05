#ifndef __ModuleControlLong_h__
#define __ModuleControlLong_h__

#include "ModuleControl.h"

class CModuleControlLong : public CModuleControl {
public:
  CModuleControlLong(SModuleSettings *pSetting) : CModuleControl(pSetting) {};

  virtual LRESULT OnScroll(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
 
  virtual int GetHeightRequest();
  virtual void Initialise(Dasher::CDasherInterfaceBase *pInterface);
  virtual void Apply(Dasher::CDasherInterfaceBase *pInterface);
  virtual void CreateChild(HWND hParent);
  virtual void LayoutChild(RECT &sRect);

private:
  HWND m_hSlider;
  HWND m_hEntry;
};

#endif