#ifndef __ModuleControlString_h__
#define __ModuleControlString_h__

#include "ModuleControl.h"

class CModuleControlString : public CModuleControl {
public:
  CModuleControlString(SModuleSettings *pSetting) : CModuleControl(pSetting) {};

  virtual void Initialise(CAppSettings *pAppSets);
  virtual void Apply(CAppSettings *pAppSets);

protected:
  virtual void CreateChild(HWND hParent, RECT& rect);
  virtual int GetChildHeight() { return 14; };

private:
  CWindow m_hEntry;
};

#endif