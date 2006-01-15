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
  {BP_START_MOUSE, IDC_LEFT},
  {BP_START_SPACE, IDC_SPACE},
  {BP_START_STYLUS, IDC_STYLUS},
  {BP_STOP_IDLE, IDC_STOPIDLE},
  {BP_MOUSEPOS_MODE, IDC_MOUSEPOS},
  {BP_NUMBER_DIMENSIONS, IDC_1D},
  {BP_EYETRACKER_MODE, IDC_EYETRACKER},
  {BP_KEY_CONTROL, IDC_BUTTONMODE},
  {APP_BP_COPY_ALL_ON_STOP, IDC_COPYALLONSTOP},
  {APP_BP_SPEECH_MODE, IDC_SPEECH},
  {APP_BP_WINDOW_PAUSE, IDC_WINDOWPAUSE},
  {BP_AUTO_SPEEDCONTROL, IDC_AUTOSPEED}
};

void CButtonTypePage::PopulateList() {
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher


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

	int iIdle;
	bool bSuccess = wincommon::GetWindowInt( GetDlgItem(m_hwnd, IDC_IDLETIME) , iIdle);
	if (bSuccess)
		m_pAppSettings->SetLongParameter( LP_STOP_IDLETIME, iIdle);
	else
	{
		return FALSE; // TODO notify user
	}

  double NewSpeed;
  NewSpeed = SendMessage(SB_slider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_MAX_BITRATE, NewSpeed);
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    m_pAppSettings->SetBoolParameter(menutable[ii].paramNum, 
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED );
  }

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
		case IDC_STOPIDLE:
			// set activity of the idle time edit control
			BOOL bChecked =  SendMessage(GetDlgItem(m_hwnd, IDC_STOPIDLE), BM_GETCHECK, 0, 0) == BST_CHECKED;
			EnableWindow( GetDlgItem(m_hwnd, IDC_IDLETIME), bChecked);
			EnableWindow( GetDlgItem(m_hwnd, IDC_STATICIDLETIME), bChecked);
                        // don't return: also want to pass event on to superclass to trigger Apply activation
			break;
		}
		break;

	case WM_HSCROLL:
		if((LOWORD(wParam) == SB_THUMBPOSITION) | (LOWORD(wParam) == SB_THUMBTRACK)) {
			// Some messages give the new postion
			NewSpeed = HIWORD(wParam);
		}
		else {
			// Otherwise we have to ask for it
			long Pos = SendMessage(SB_slider, TBM_GETPOS, 0, 0);
			NewSpeed = Pos;
		}
		{
			_sntprintf(m_tcBuffer, 100, TEXT("%0.2f"), NewSpeed / 100);
			SendMessage(speedbox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
		}
		return TRUE;
		break;

        }
        // pass on to superclass:
        return CPrefsPageBase::WndProc(Window, message, wParam, lParam);
}
