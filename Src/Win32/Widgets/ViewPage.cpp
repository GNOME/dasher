// AlphabetBox.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "ViewPage.h"
#include "../resource.h"

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

CViewPage::CViewPage(HWND Parent, CDasherInterfaceBase *DI, CAppSettings *pAppSettings)
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
  {BP_OUTLINE_MODE, IDC_OUTLINE},
  {BP_DRAW_MOUSE, IDC_DRAWMOUSE},
  {BP_DRAW_MOUSE_LINE, IDC_DRAWMOUSELINE},
};

void CViewPage::PopulateList() {
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    if(m_pAppSettings->GetBoolParameter(menutable[ii].paramNum)) {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
    }
    else  {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_UNCHECKED, 0);
    }
  }

  if(m_pAppSettings->GetLongParameter(LP_LINE_WIDTH) > 1)
    SendMessage(GetDlgItem(m_hwnd, IDC_THICKLINE), BM_SETCHECK, BST_CHECKED, 0);
  else
    SendMessage(GetDlgItem(m_hwnd, IDC_THICKLINE), BM_SETCHECK, BST_UNCHECKED, 0);

  // Populate the orientation selection:

  switch(m_pAppSettings->GetLongParameter(LP_ORIENTATION)) {
    case Dasher::Opts::AlphabetDefault:
      SendMessage(GetDlgItem(m_hwnd, IDC_RADIO1), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case Dasher::Opts::LeftToRight:
      SendMessage(GetDlgItem(m_hwnd, IDC_RADIO2), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case Dasher::Opts::RightToLeft:
      SendMessage(GetDlgItem(m_hwnd, IDC_RADIO3), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case Dasher::Opts::TopToBottom:
      SendMessage(GetDlgItem(m_hwnd, IDC_RADIO4), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case Dasher::Opts::BottomToTop:
      SendMessage(GetDlgItem(m_hwnd, IDC_RADIO5), BM_SETCHECK, BST_CHECKED, 0);
      break;
  }

  switch(m_pAppSettings->GetLongParameter(APP_LP_STYLE)) {
    case 0:
      SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_STANDALONE), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case 1:
      SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_COMPOSITION), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case 2:
      SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_DIRECT), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case 3:
      SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_FULL), BM_SETCHECK, BST_CHECKED, 0);
      break;
  }
}


bool CViewPage::Apply() {
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    m_pAppSettings->SetBoolParameter(menutable[ii].paramNum, 
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED );
  }

  if(SendMessage(GetDlgItem(m_hwnd, IDC_THICKLINE), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_LINE_WIDTH, 3);
  else
    m_pAppSettings->SetLongParameter(LP_LINE_WIDTH, 1);

  if(SendMessage(GetDlgItem(m_hwnd, IDC_RADIO1), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::AlphabetDefault);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_RADIO2), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::LeftToRight);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_RADIO3), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::RightToLeft);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_RADIO4), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::TopToBottom);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_RADIO5), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::BottomToTop);

  if(SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_STANDALONE), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(APP_LP_STYLE, 0);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_COMPOSITION), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(APP_LP_STYLE, 1);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_DIRECT), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(APP_LP_STYLE, 2);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_STYLE_FULL), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(APP_LP_STYLE, 3);

  // Return false (and notify the user) if something is wrong.
  return TRUE;
}

