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
  m_hParent = hParent;

  CWindowImpl<CModuleControl>::Create(m_hParent);
  
  m_hWndCaption = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), m_strCaption.c_str(), 
    WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, NULL, WinHelper::hInstApp, NULL);

  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);
 
  SendMessage(m_hWndCaption, WM_SETFONT, (WPARAM)hGuiFont, (LPARAM)true);

  CreateChild(m_hWnd);
};

void CModuleControl::Layout(RECT *pRect) {
  MoveWindow(pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
  
  ::MoveWindow(m_hWndCaption, 0, 0, (pRect->right - pRect->left) / 2, pRect->bottom - pRect->top, TRUE);

  RECT sRect;
  sRect.left = (pRect->right - pRect->left) / 2;
  sRect.right = pRect->right;
  sRect.top = 0;
  sRect.bottom = pRect->bottom - pRect->top;

  LayoutChild(sRect);
};