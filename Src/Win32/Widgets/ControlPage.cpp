// ControlPage.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "ControlPage.h"
#include "../resource.h"
#include "../Common/StringUtils.h"
#include "../AppSettings.h"

#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;


CControlPage::CControlPage(HWND Parent, CDasherInterface *DI, CAppSettings *pAppSettings)
:m_pDasherInterface(DI), m_pAppSettings(pAppSettings) {
  m_hwnd = 0;
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
  {APP_BP_COPY_ALL_ON_STOP, IDC_COPYALLONSTOP},
  {APP_BP_SPEECH_MODE, IDC_SPEECH},
  {APP_BP_WINDOW_PAUSE, IDC_WINDOWPAUSE},
  {BP_AUTO_SPEEDCONTROL, IDC_AUTOSPEED}
};

void CControlPage::PopulateList() {
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher

  SB_slider = GetDlgItem(m_hwnd, IDC_SPEEDSLIDER);

  SendMessage(SB_slider, TBM_SETPAGESIZE, 0L, 20);      // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(SB_slider, TBM_SETTICFREQ, 100, 0L);
  SendMessage(SB_slider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(10, 800));
  
  speedbox = GetDlgItem(m_hwnd, IDC_SPEEDVAL);  

  SendMessage(SB_slider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_MAX_BITRATE));
  _sntprintf(m_tcBuffer, 100, TEXT("%0.2f"), m_pAppSettings->GetLongParameter(LP_MAX_BITRATE) / 100.0);
  SendMessage(speedbox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);


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

  // enable idletime control if button checked
  BOOL bIdle =  m_pAppSettings->GetBoolParameter(BP_STOP_IDLE) ? TRUE : FALSE;
  EnableWindow( GetDlgItem(m_hwnd, IDC_IDLETIME), bIdle);
  EnableWindow( GetDlgItem(m_hwnd, IDC_STATICIDLETIME), bIdle);
			
  // Set the idle time data
  SetDlgItemInt ( m_hwnd, IDC_IDLETIME, m_pAppSettings->GetLongParameter( LP_STOP_IDLETIME) , TRUE);

}

bool CControlPage::Validate() {
  // Return false if something is wrong to prevent user from clicking to a different page. Please also pop up a dialogue informing the user at this point.
  return TRUE;
}

bool CControlPage::Apply() 
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
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_GETCHECK, 0, 0));
  }

	// Return false (and notify the user) if something is wrong.
	return TRUE;
}

LRESULT CControlPage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {

	NMHDR *pNMHDR;
	double NewSpeed;
	switch (message) 
	{
	case WM_INITDIALOG:
		if(!m_hwnd) {               // If this is the initial dialog for the first time
			m_hwnd = Window;
			PopulateList();
		}
		return TRUE;
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case (IDC_DISPLAY):
			break;
		case IDC_STOPIDLE:
			// set activity of the idle time edit control
			BOOL bChecked =  SendMessage(GetDlgItem(m_hwnd, IDC_STOPIDLE), BM_GETCHECK, 0, 0) !=0;
			EnableWindow( GetDlgItem(m_hwnd, IDC_IDLETIME), bChecked);
			EnableWindow( GetDlgItem(m_hwnd, IDC_STATICIDLETIME), bChecked);
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
