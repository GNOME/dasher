#ifndef __ModuleControl_h__
#define __ModuleControl_h__

#include "Common/WinCommon.h"
#include "AppSettings.h"
#include "../Common/ModuleSettings.h"
#include <string>

class CModuleControl : public CWindowImpl<CModuleControl> {
public:
  static const int CAPTION_WIDTH = 125;
  static const int CHILD_WIDTH = 125;

  CModuleControl(SModuleSettings *pSetting);
  DECLARE_WND_SUPERCLASS(NULL, L"STATIC")

  BEGIN_MSG_MAP(CModuleControl)
  END_MSG_MAP()
  void Create(HWND hParent, RECT& rect);

  virtual int GetHeight();

  // Abstract members to be implemented by descendents
  virtual void Initialise(CAppSettings*) = 0;
  virtual void Apply(CAppSettings*) = 0;

protected:
  virtual void CreateChild(HWND hParent, RECT& rect) = 0;
  virtual int GetChildHeight() = 0;

  int m_iId;
  int m_iMin;
  int m_iMax;
  int m_iDivisor;
  int m_iStep;

private:
  virtual int GetCaptionHeight();
  std::wstring m_strCaption;
  CWindow m_hCaption;
};

#endif
