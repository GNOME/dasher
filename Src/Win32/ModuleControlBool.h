#ifndef __ModuleControlBool_h__
#define __ModuleControlBool_h__

#include "ModuleControl.h"

class CModuleControlBool : public CModuleControl {
public:
  CModuleControlBool(SModuleSettings *pSetting) : CModuleControl(pSetting) {};

  virtual int GetChildHeight() { return 10; };
  virtual void Initialise(CAppSettings *pAppSets);
  virtual void Apply(CAppSettings *pAppSets);
  virtual void CreateChild(HWND hParent, RECT& rect);

private:
  CWindow m_hCheckbox;
};

#endif