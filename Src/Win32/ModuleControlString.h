#ifndef __ModuleControlString_h__
#define __ModuleControlString_h__

#include "ModuleControl.h"

class CModuleControlString : public CModuleControl {
public:
  CModuleControlString(SModuleSettings *pSetting) : CModuleControl(pSetting) {};

  virtual int GetHeightRequest();
  virtual void Initialise(Dasher::CDasherInterfaceBase *pInterface);
  virtual void Apply(Dasher::CDasherInterfaceBase *pInterface);
  virtual void CreateChild(HWND hParent);
  virtual void LayoutChild(RECT &sRect);

private:
  HWND m_hEntry;
};

#endif