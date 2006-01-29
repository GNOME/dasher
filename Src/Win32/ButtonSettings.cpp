// ButtonSettings.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "ButtonSettings.h"
#include "../resource.h"
#include "../Common/StringUtils.h"
#include "../AppSettings.h"

#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;


CButtonSettingsPage::CButtonSettingsPage(HWND Parent, CDasherInterface *DI, CAppSettings *pAppSettings)
:CPrefsPageBase(Parent, DI, pAppSettings) {
}

void CButtonSettingsPage::PopulateList() {
  // Populate the controls in the dialogue box based on the relevent parameters
  // in m_pDasher

  m_hRightZoomSlider = GetDlgItem(m_hwnd, IDC_RIGHTZOOM_SLIDER);
  SendMessage(m_hRightZoomSlider, TBM_SETPAGESIZE, 0L, 1); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hRightZoomSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(1, 9));
  SendMessage(m_hRightZoomSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_RIGHTZOOM)/1024.0);

  m_hNoBoxesSlider = GetDlgItem(m_hwnd, IDC_NOBOXES_SLIDER);
  SendMessage(m_hNoBoxesSlider, TBM_SETPAGESIZE, 0L, 1); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hNoBoxesSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(2, 10));
  SendMessage(m_hNoBoxesSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_B));

  m_hSafetySlider = GetDlgItem(m_hwnd, IDC_SAFETY_SLIDER);
  SendMessage(m_hSafetySlider, TBM_SETPAGESIZE, 0L, 10); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hSafetySlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(0, 256));
  SendMessage(m_hSafetySlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_S));

  m_hNonUniformSlider = GetDlgItem(m_hwnd, IDC_NONUNIFORM_SLIDER);
  SendMessage(m_hNonUniformSlider, TBM_SETPAGESIZE, 0L, 50); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hNonUniformSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(-400, 400));
  SendMessage(m_hNonUniformSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_R));
}

bool CButtonSettingsPage::Validate() {
  // Return false if something is wrong to prevent user from clicking to a different page. Please also pop up a dialogue informing the user at this point.
  return TRUE;
}

bool CButtonSettingsPage::Apply() 
{

  double NewValue;

  NewValue = SendMessage(m_hRightZoomSlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_RIGHTZOOM, static_cast<int>(NewValue * 1024));

  NewValue = SendMessage(m_hNoBoxesSlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_B, static_cast<int>(NewValue));

  NewValue = SendMessage(m_hSafetySlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_S, static_cast<int>(NewValue));

  NewValue = SendMessage(m_hNonUniformSlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_R, static_cast<int>(NewValue));


	// Return false (and notify the user) if something is wrong.
	return TRUE;
}

LRESULT CButtonSettingsPage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {

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
