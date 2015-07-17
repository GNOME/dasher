#ifndef __ModuleControlString_h__
#define __ModuleControlString_h__

#include "ModuleControl.h"

class CModuleControlString : public CModuleControl {
public:
  CModuleControlString(SModuleSettings *pSetting) : CModuleControl(pSetting) {};

  virtual int GetHeightRequest();
  virtual void Initialise(CAppSettings *pAppSets);
  virtual void Apply(CAppSettings *pAppSets);
  virtual void CreateChild(HWND hParent);
  virtual void LayoutChild(RECT &sRect);

private:
  CWindow m_hEntry;
};

#endif