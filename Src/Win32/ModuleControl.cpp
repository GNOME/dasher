#include "ModuleControl.h"
#include <algorithm>

CModuleControl::CModuleControl(SModuleSettings *pSetting) {
  WinUTF8::UTF8string_to_wstring(pSetting->szDescription, m_strCaption);

  m_iId = pSetting->iParameter;
  m_iMin = pSetting->iMin;
  m_iMax = pSetting->iMax;
  m_iDivisor = pSetting->iDivisor;
  m_iStep = pSetting->iStep;
};

void CModuleControl::Create(HWND hParent, RECT& rect)
{
  CWindowImpl<CModuleControl>::Create(hParent, rect);

  RECT captionRect = { 0, 0, CAPTION_WIDTH, GetCaptionHeight() };
  MapDialogRect(hParent, &captionRect);
  m_hCaption.Create(TEXT("STATIC"), *this, captionRect, m_strCaption.c_str(), WS_CHILD | WS_VISIBLE);

  RECT childRect = { CAPTION_WIDTH, 0, CAPTION_WIDTH + CHILD_WIDTH, GetChildHeight() };
  MapDialogRect(hParent, &childRect);
  CreateChild(*this, childRect);
};

int CModuleControl::GetHeight() {
  return std::max(GetCaptionHeight(), GetChildHeight());
}

int CModuleControl::GetCaptionHeight() {
  int charsPerLine = CAPTION_WIDTH / 4;
  return 8 * ((m_strCaption.length() + charsPerLine - 1) / charsPerLine);
}
