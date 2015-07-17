#ifndef __ModuleControlBool_h__
#define __ModuleControlBool_h__

#include "ModuleControl.h"

class CModuleControlBool : public CModuleControl {
public:
  CModuleControlBool(SModuleSettings *pSetting) : CModuleControl(pSetting) {};

  virtual int GetHeightRequest();
  virtual void Initialise(CAppSettings *pAppSets);
  virtual void Apply(CAppSettings *pAppSets);
  virtual void CreateChild(HWND hParent);
  virtual void LayoutChild(RECT &sRect);

private:
  CWindow m_hCheckbox;
};

#endif