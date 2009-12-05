#include "ModuleControlBool.h"
#include "ModuleControlLong.h"
#include "ModuleControlLongSpin.h"
#include "ModuleControlString.h"
#include "ModuleSettings.h"

CONST UINT WM_MS_CLOSE = RegisterWindowMessage(_WM_MS_CLOSE);

CModuleSettings::CModuleSettings(const std::string &strModuleName, SModuleSettings *pSettings, int iCount, Dasher::CDasherInterfaceBase *pInterface) {
  m_iCount = iCount;
  m_pControls = new CModuleControl*[m_iCount];
  m_pInterface = pInterface;
  m_strModuleName = strModuleName;

  for(int i(0); i < m_iCount; ++i) {
    switch(pSettings[i].iType) {
      case T_BOOL:
        m_pControls[i] = new CModuleControlBool(pSettings + i);
        break;
      case T_LONG:
        m_pControls[i] = new CModuleControlLong(pSettings + i);
        break;
      case T_LONGSPIN:
        m_pControls[i] = new CModuleControlLongSpin(pSettings + i);
        break;
      case T_STRING:
        m_pControls[i] = new CModuleControlString(pSettings + i);
        break;
    }
   
  }
}

CModuleSettings::~CModuleSettings() {
  for(int i(0); i < m_iCount; ++i)
    delete m_pControls[i];

  delete[] m_pControls;
}

void CModuleSettings::Create(HWND hWndParent, ATL::_U_RECT rect) {
  m_hParent = hWndParent;

  std::wstring strWName;
  WinUTF8::UTF8string_to_wstring(m_strModuleName, strWName);

  RECT sRect = {CW_USEDEFAULT, CW_USEDEFAULT, 0, 0};
  CWindowImpl<CModuleSettings>::Create(hWndParent, sRect, strWName.c_str(), WS_DLGFRAME | WS_SYSMENU | WS_CLIPCHILDREN, WS_EX_WINDOWEDGE | WS_EX_CONTEXTHELP);

  int iHeight(7);

  for(int i(0); i < m_iCount; ++i) {
    m_pControls[i]->Create(m_hWnd);
    m_pControls[i]->Initialise(m_pInterface);
    iHeight += m_pControls[i]->GetHeightRequest() + 2;
  }

  m_hOk = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("BUTTON"), L"Ok", 
      WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 50, 14, m_hWnd, NULL, WinHelper::hInstApp, NULL);

  m_hCancel = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("BUTTON"), L"Cancel", 
      WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 50, 14, m_hWnd, NULL, WinHelper::hInstApp, NULL);

  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);
 
  SendMessage(m_hOk, WM_SETFONT, (WPARAM)hGuiFont, (LPARAM)true);
  SendMessage(m_hCancel, WM_SETFONT, (WPARAM)hGuiFont, (LPARAM)true);
  
  iHeight += 26; // Subtract 2 for spacing

  HDC hDC(::GetDC(m_hWnd));
  SelectObject(hDC, hGuiFont);
  SIZE sSize;
  GetTextExtentPoint32(hDC, L"x", 1, &sSize);

  int iBaseX(sSize.cx);
  int iBaseY(sSize.cy);

  ::ReleaseDC(m_hWnd, hDC);
  
  int iWindowWidth(MulDiv(250, iBaseX, 4));
  int iWindowHeight(MulDiv(iHeight, iBaseY, 8));

  RECT sWindowPos;
  GetWindowRect(&sWindowPos);

  sWindowPos.right = sWindowPos.left + iWindowWidth;
  sWindowPos.bottom = sWindowPos.top + iWindowHeight;

  AdjustWindowRect(&sWindowPos, WS_CAPTION, false);

  MoveWindow(&sWindowPos);
}

LRESULT CModuleSettings::OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = false;

  if(HIWORD(wParam) == BN_CLICKED) {
    if((HWND)lParam == m_hOk) {
      bHandled = true;
      // Apply and close 
      for(int i(0); i < m_iCount; ++i) {
        m_pControls[i]->Apply(m_pInterface);
      }
      ShowWindow(SW_HIDE);
      SendMessage(m_hParent, WM_MS_CLOSE, 0, 0);
    }
    else if((HWND)lParam == m_hCancel) {
      bHandled = true;
      ShowWindow(SW_HIDE);
      SendMessage(m_hParent, WM_MS_CLOSE, 0, 0);
    }
  }

  return 0;
}

LRESULT CModuleSettings::OnPaint(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  bHandled = true;

  RECT sRect;
  GetClientRect(&sRect);

  RECT sUpdateRect;
  if(!GetUpdateRect(&sUpdateRect))
    return 0;

  PAINTSTRUCT sPS;

  BeginPaint(&sPS);

  HBRUSH hBrush(GetSysColorBrush(COLOR_3DFACE));
  FillRect(sPS.hdc, &sRect, hBrush);

  EndPaint(&sPS);

  return 0;
}

LRESULT CModuleSettings::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);

  HDC hDC(::GetDC(m_hWnd));
  SelectObject(hDC, hGuiFont);
  SIZE sSize;
  GetTextExtentPoint32(hDC, L"x", 1, &sSize);

  int iBaseX(sSize.cx);
  int iBaseY(sSize.cy);

  ::ReleaseDC(m_hWnd, hDC);

  RECT sWindowRect;
  GetClientRect(&sWindowRect);

  int iWindowWidth(sWindowRect.right - sWindowRect.left);
  int iWindowHeight(sWindowRect.bottom - sWindowRect.top);

  int iDlgWidth(MulDiv(iWindowWidth, 4, iBaseX));
  int iDlgHeight(MulDiv(iWindowHeight, 8, iBaseY));

  // TODO: Dialogue units rather than pixels

  int iCurrentTop(7);

  for(int i(0); i < m_iCount; ++i) {
    int iHeight(m_pControls[i]->GetHeightRequest());

    RECT sPosition;

    sPosition.left = MulDiv(7, iBaseX, 4);
    sPosition.right = MulDiv(243, iBaseX, 4);
    sPosition.top = MulDiv(iCurrentTop, iBaseY, 8);
    sPosition.bottom = MulDiv(iCurrentTop + iHeight, iBaseY, 8);

    m_pControls[i]->Layout(&sPosition);

    iCurrentTop += iHeight + 2;
  }

  int iDlgX(iDlgWidth - 57);
  int iDlgY(iDlgHeight - 21);
  int iDlgButtonWidth(50);
  int iDlgButtonHeight(14);

  ::MoveWindow(m_hCancel, MulDiv(iDlgX, iBaseX, 4), MulDiv(iDlgY, iBaseY, 8), 
    MulDiv(iDlgButtonWidth, iBaseX, 4), MulDiv(iDlgButtonHeight, iBaseY, 8), true);  

  iDlgX = iDlgWidth - 114;
  
  ::MoveWindow(m_hOk, MulDiv(iDlgX, iBaseX, 4), MulDiv(iDlgY, iBaseY, 8), 
    MulDiv(iDlgButtonWidth, iBaseX, 4), MulDiv(iDlgButtonHeight, iBaseY, 8), true);  

  return 0;
}
