#include "ModuleControl.h"

CModuleControl::CModuleControl(SModuleSettings *pSetting) {
  WinUTF8::UTF8string_to_wstring(pSetting->szDescription, m_strCaption);

  m_iId = pSetting->iParameter;
  m_iMin = pSetting->iMin;
  m_iMax = pSetting->iMax;
  m_iDivisor = pSetting->iDivisor;
  m_iStep = pSetting->iStep;
};

void CModuleControl::Create(HWND hParent) {
  m_hCaption.Create(TEXT("STATIC"), hParent, 0, m_strCaption.c_str(), WS_CHILD | WS_VISIBLE);
  CreateChild(hParent);
};

void CModuleControl::Layout(RECT *pRect) {
  m_hCaption.MoveWindow(pRect->left, pRect->top, (pRect->right - pRect->left) / 2, pRect->bottom - pRect->top);

  RECT sRect;
  sRect.left = (pRect->right + pRect->left) / 2;
  sRect.right = pRect->right;
  sRect.top = pRect->top;
  sRect.bottom = pRect->bottom;

  LayoutChild(sRect);
};