#ifndef __ModuleControl_h__
#define __ModuleControl_h__

#include "Common/WinCommon.h"
#include "AppSettings.h"
#include "../Common/ModuleSettings.h"
#include <atlbase.h>
#include <atlwin.h>
#include <string>

class CModuleControl {
public:
  CModuleControl(SModuleSettings *pSetting);

 
  void Create(HWND hParent);
  void Layout(RECT *pRect);

  // Abstract members to be implemented by descendents
  virtual int GetHeightRequest() = 0;
  virtual void Initialise(CAppSettings*) = 0;
  virtual void Apply(CAppSettings*) = 0;
  virtual void CreateChild(HWND hParent) = 0;
  virtual void LayoutChild(RECT &sRect) = 0;

protected:
  std::wstring m_strCaption;

  int m_iId;
  int m_iMin;
  int m_iMax;
  int m_iDivisor;
  int m_iStep;

  CWindow m_hCaption;
};

#endif
