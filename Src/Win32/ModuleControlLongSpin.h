#ifndef __ModuleControlLongSpin_h__
#define __ModuleControlLongSpin_h__

#include "ModuleControl.h"

class CModuleControlLongSpin : public CModuleControl {
public:
  CModuleControlLongSpin(SModuleSettings *pSetting) : CModuleControl(pSetting) {};

  virtual int GetHeightRequest();
  virtual void Initialise(CAppSettings *pAppSets);
  virtual void Apply(CAppSettings *pAppSets);
  virtual void CreateChild(HWND hParent);
  virtual void LayoutChild(RECT &sRect);

private:
  CWindow m_hEntry;
};

#endif