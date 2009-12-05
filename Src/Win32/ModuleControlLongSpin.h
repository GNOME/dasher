#ifndef __ModuleControlLongSpin_h__
#define __ModuleControlLongSpin_h__

#include "ModuleControl.h"

class CModuleControlLongSpin : public CModuleControl {
public:
  CModuleControlLongSpin(SModuleSettings *pSetting) : CModuleControl(pSetting) {};

  virtual LRESULT OnNotify(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  virtual int GetHeightRequest();
  virtual void Initialise(Dasher::CDasherInterfaceBase *pInterface);
  virtual void Apply(Dasher::CDasherInterfaceBase *pInterface);
  virtual void CreateChild(HWND hParent);
  virtual void LayoutChild(RECT &sRect);

private:
  void UpdateEntry(int iValue, int iDelta);

  HWND m_hEntry;
  HWND m_hSpin;
};

#endif