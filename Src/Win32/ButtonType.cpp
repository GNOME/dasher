// ButtonType.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "ButtonType.h"
#include "../resource.h"
#include "../Common/StringUtils.h"
#include "../AppSettings.h"

#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;


CButtonTypePage::CButtonTypePage(HWND Parent, CDasherInterface *DI, CAppSettings *pAppSettings)
:CPrefsPageBase(Parent, DI, pAppSettings) {
}

struct menuentry {
  int paramNum; // enum value in Parameters.h for setting store
  int idcNum;   // #define value in resource.h for dasher.rc
};

// List of menu items that will be displayed in the General Preferences
static menuentry menutable[] = {
  {BP_BUTTONONEDYNAMIC, IDC_ONE_DYNAMIC},
  {BP_BUTTONMENU, IDC_MENU},
  {BP_BUTTONDIRECT, IDC_THREE_DIRECT},
  {BP_BUTTONALTERNATINGDIRECT, IDC_ALTERNATING_DIRECT},
  {BP_COMPASSMODE, IDC_COMPASS}
};

void CButtonTypePage::PopulateList() {
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher

  m_hSlider = GetDlgItem(m_hwnd, IDC_UNIFORMSLIDER);
  SendMessage(m_hSlider, TBM_SETPAGESIZE, 0L, 10); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(0, 63));

  SendMessage(m_hSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_ZOOMSTEPS));


  // all the button checkboxes
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    if(m_pAppSettings->GetBoolParameter(menutable[ii].paramNum)) 
	{
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
    }
    else  
	{
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_UNCHECKED, 0);
    }
  }


}

bool CButtonTypePage::Validate() {
  // Return false if something is wrong to prevent user from clicking to a different page. Please also pop up a dialogue informing the user at this point.
  return TRUE;
}

bool CButtonTypePage::Apply() 
{
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    m_pAppSettings->SetBoolParameter(menutable[ii].paramNum, 
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED );
  }

  double NewZoomSteps;
  NewZoomSteps = SendMessage(m_hSlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_ZOOMSTEPS, static_cast<int>(NewZoomSteps));

	// Return false (and notify the user) if something is wrong.
	return TRUE;
}

LRESULT CButtonTypePage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {

	double NewSpeed;
	switch (message) 
	{	
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case (IDC_DISPLAY):
			break;
		}
		break;

	

  }
  // pass on to superclass:
  return CPrefsPageBase::WndProc(Window, message, wParam, lParam);
}
