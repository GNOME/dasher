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
  {BP_CONTROL_MODE, IDC_CONTROLMODE}  // Not global setting - specific to editbox/widget
  //{APP_BP_COPY_ALL_ON_STOP, IDC_COPYALLONSTOP},
  //{APP_BP_SPEECH_MODE, IDC_SPEECH}
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
}

bool CAdvancedPage::Apply() {

  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++) {
    m_pAppSettings->SetBoolParameter(menutable[ii].paramNum, SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED );
  }

  return TRUE;
}
