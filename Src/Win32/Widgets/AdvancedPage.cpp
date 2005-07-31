// AlphabetBox.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "AdvancedPage.h"
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

CAdvancedPage::CAdvancedPage(HWND Parent, CDasherInterface *DI)
:m_pDasherInterface(DI), m_CurrentColours(DI->GetStringParameter(SP_COLOUR_ID)) {
  m_hwnd = 0;
}

struct menuentry {
  int paramNum; // enum value in Parameters.h for setting store
  int idcNum;   // #define value in resource.h for dasher.rc
};

// List of menu items that will be displayed in the General Preferences
//static menuentry menutable[] = {
//  {BP_TIME_STAMP, IDC_TIMESTAMP},   // Not global setting - specific to editbox/widget
//};

void CAdvancedPage::PopulateList() {
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasherInterface
 /* for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    if(m_pDasherInterface->GetBoolParameter(menutable[ii].paramNum)) {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
    }
    else  {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_UNCHECKED, 0);
    }
  }*/

  int ypixels = m_pDasherInterface->GetLongParameter(LP_YSCALE);
  int mouseposdist = m_pDasherInterface->GetLongParameter(LP_MOUSEPOSDIST);

/*	for (int i=0; i<18; i++) {
			keycoords[i]=coords[i];
	}
*/
  HWND EditBox = GetDlgItem(m_hwnd, IDC_YPIX);
  SendMessage(EditBox, LB_RESETCONTENT, 0, 0);

  // Perhaps a typedef for std::basic_ostringstream<TCHAR> would be useful
//      std::basic_ostringstream<TCHAR> strYPix;
//      strYPix << ypixels;

  _sntprintf(m_tcBuffer, 100, TEXT("%d"), ypixels);
//
//      SendMessage(EditBox, WM_SETTEXT, 0, (LPARAM)(LPCSTR) Buffer);
  SendMessage(EditBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
//      delete[] Buffer;

  EditBox = GetDlgItem(m_hwnd, IDC_MOUSEPOSDIST);
  SendMessage(EditBox, LB_RESETCONTENT, 0, 0);

  _sntprintf(m_tcBuffer, 100, TEXT("%d"), mouseposdist);
//      std::basic_ostringstream<TCHAR> strMousePosDist;
//      strMousePosDist << mouseposdist;

  SendMessage(EditBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
//      delete[] Buffer;

}

bool CAdvancedPage::Validate() {
  // Return false if something is wrong to prevent user from clicking to a different page. Please also pop up a dialogue informing the user at this point.
  return TRUE;
}

bool CAdvancedPage::Apply() {
  // Return false (and notify the user) if something is wrong.
  return TRUE;
}

LRESULT CAdvancedPage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  NMHDR *pNMHDR;
  
  switch (message) {
  case WM_INITDIALOG:
    if(!m_hwnd) {               // If this is the initial dialog for the first time
      m_hwnd = Window;
      PopulateList();
    }
    return TRUE;
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case (IDC_DISPLAY):
      break;
    }
    break;
  case WM_NOTIFY:
    pNMHDR = (NMHDR*)lParam;
    switch (pNMHDR->code) {
    case PSN_KILLACTIVE: // About to lose focus
      SetWindowLong( pNMHDR->hwndFrom, DWL_MSGRESULT, Validate());
      return TRUE;
      break;
    case PSN_APPLY: // User clicked OK/Apply - apply the changes
      if(Apply())
        SetWindowLong( pNMHDR->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLong( pNMHDR->hwndFrom, DWL_MSGRESULT, PSNRET_INVALID);
      return TRUE;
      break;
    }
    break;
  }
  return FALSE;
}
