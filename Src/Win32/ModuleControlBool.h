#ifndef __ModuleControlBool_h__
#define __ModuleControlBool_h__

#include "ModuleControl.h"

class CModuleControlBool : public CModuleControl {
public:
  CModuleControlBool(SModuleSettings *pSetting) : CModuleControl(pSetting) {};

  virtual int GetHeightRequest();
  virtual void Initialise(Dasher::CDasherInterfaceBase *pInterface);
  virtual void Apply(Dasher::CDasherInterfaceBase *pInterface);
  virtual void CreateChild(HWND hParent);
  virtual void LayoutChild(RECT &sRect);

private:
  HWND m_hCheckbox;
};

#endif