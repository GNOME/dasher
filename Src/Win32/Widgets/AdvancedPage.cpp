// AdvancedPage.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "AdvancedPage.h"
#include "../resource.h"
#include "../Common/StringUtils.h"

#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CAdvancedPage::CAdvancedPage(HWND Parent, CDasherInterfaceBase *DI, CAppSettings *pAppSettings)
:CPrefsPageBase(Parent, DI, pAppSettings) {
}

struct menuentry {
  int paramNum; // enum value in Parameters.h for setting store
  int idcNum;   // #define value in resource.h for dasher.rc
};

// List of menu items that will be displayed in the General Preferences
static menuentry menutable[] = {
  {APP_BP_SHOW_TOOLBAR, IDC_CHECK1},
  {BP_SHOW_SLIDER, IDC_CHECK2},
  {APP_BP_TIME_STAMP, IDC_TIMESTAMP},
  {BP_CONTROL_MODE, IDC_CONTROLMODE},  // Not global setting - specific to editbox/widget
  {APP_BP_SPEECH_WORD, IDC_CHECK4},
  {APP_BP_SPEECH_MODE, IDC_CHECK3},
  {APP_BP_COPY_ALL_ON_STOP, IDC_COPYONSTOP}
};

std::string CAdvancedPage::GetControlText(HWND Dialog, int ControlID) 
{
  HWND Control = GetDlgItem(Dialog, ControlID);
  std::wstring str;
  wincommon::GetWindowText( Control, str);

  string ItemName;
  WinUTF8::wstring_to_UTF8string(str, ItemName);
  return ItemName;
}

void CAdvancedPage::PopulateList() {

   //if(m_pAppSettings->GetBoolParameter(APP_BP_TIME_STAMP)) {
   //   SendMessage(GetDlgItem(m_hwnd, IDC_TIMESTAMP), BM_SETCHECK, BST_CHECKED, 0);
   // }
   // else  {
   //   SendMessage(GetDlgItem(m_hwnd, IDC_TIMESTAMP), BM_SETCHECK, BST_UNCHECKED, 0);
   // }


  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasherInterface
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    if(m_pAppSettings->GetBoolParameter(menutable[ii].paramNum)) {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
    }
    else  {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_UNCHECKED, 0);
    }
  }

  
  switch(m_pAppSettings->GetLongParameter(APP_LP_STYLE)) {
    case APP_STYLE_TRAD:
      SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_STANDALONE), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case APP_STYLE_COMPOSE:
      SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_COMPOSITION), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case APP_STYLE_DIRECT:
      SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_DIRECT), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case APP_STYLE_FULLSCREEN:
      SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_FULL), BM_SETCHECK, BST_CHECKED, 0);
      break;
  }

}

bool CAdvancedPage::Apply() {
  if(SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_STANDALONE), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(APP_LP_STYLE, 0);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_COMPOSITION), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(APP_LP_STYLE, 1);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_DIRECT), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(APP_LP_STYLE, 2);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_FULL), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(APP_LP_STYLE, 3);

  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++) {
    m_pAppSettings->SetBoolParameter(menutable[ii].paramNum, SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED );
  }

  return TRUE;
}

LRESULT CAdvancedPage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {

  // most things we pass on to CPrefsPageBase, but we need to handle slider motion
  switch (message) {

  case WM_COMMAND:
    if(HIWORD(wParam)==BN_CLICKED || HIWORD(wParam)==LBN_SELCHANGE) {
      if(LOWORD(wParam) != 0 && m_hPropertySheet != 0 && m_hwnd != 0) {
        PropSheet_Changed(m_hPropertySheet, m_hwnd); // enables the 'Apply' button
        // Behaviour isn't *perfect* since it activates the Apply button even if you, say,
        // click 'new' alphabet then click Cancel when asked for a name.
      }
    }
    switch (LOWORD(wParam)) {

  case IDC_EFONT_BUTTON:
    // TODO: Put this in a function
     {
  CHOOSEFONT Data;
    LOGFONT lf;
    HFONT Font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
    GetObject(Font, sizeof(LOGFONT), &lf);
    Tstring tstrFaceName;
    WinUTF8::UTF8string_to_wstring(m_pAppSettings->GetStringParameter(APP_SP_EDIT_FONT), tstrFaceName);
    _tcscpy(lf.lfFaceName, tstrFaceName.c_str());
    lf.lfHeight = m_pAppSettings->GetLongParameter(APP_LP_EDIT_FONT_SIZE);
    Data.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
    Data.lStructSize = sizeof(CHOOSEFONT);
    Data.hwndOwner = NULL;
    Data.lpLogFont = &lf;
    if(ChooseFont(&Data)) {
      string FontName;
      WinUTF8::wstring_to_UTF8string(lf.lfFaceName, FontName);
      m_pAppSettings->SetStringParameter(APP_SP_EDIT_FONT, FontName);
      m_pAppSettings->SetLongParameter(APP_LP_EDIT_FONT_SIZE, lf.lfHeight);
    }
  }
    break;

      }
  }

  return CPrefsPageBase::WndProc(Window, message, wParam, lParam);
}