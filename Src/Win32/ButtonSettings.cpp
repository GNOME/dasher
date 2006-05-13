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

  m_hZoomStepsSlider = GetDlgItem(m_hwnd, IDC_ZOOMSTEPS_SLIDER);
  SendMessage(m_hZoomStepsSlider, TBM_SETPAGESIZE, 0L, 1); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hZoomStepsSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(1, 9));
  SendMessage(m_hZoomStepsSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_ZOOMSTEPS));

  m_hZoomStepsBox = GetDlgItem(m_hwnd, IDC_ZOOMSTEPS_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(m_pAppSettings->GetLongParameter(LP_ZOOMSTEPS)));
  SendMessage(m_hZoomStepsBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);


  m_hRightZoomSlider = GetDlgItem(m_hwnd, IDC_RIGHTZOOM_SLIDER);
  SendMessage(m_hRightZoomSlider, TBM_SETPAGESIZE, 0L, 1); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hRightZoomSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(1, 9));
  SendMessage(m_hRightZoomSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_RIGHTZOOM)/1024.0);

  m_hRightZoomBox = GetDlgItem(m_hwnd, IDC_RIGHTZOOM_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%0.2f"), m_pAppSettings->GetLongParameter(LP_RIGHTZOOM)/1024.0);
  SendMessage(m_hRightZoomBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

  m_hNoBoxesSlider = GetDlgItem(m_hwnd, IDC_NOBOXES_SLIDER);
  SendMessage(m_hNoBoxesSlider, TBM_SETPAGESIZE, 0L, 1); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hNoBoxesSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(2, 10));
  SendMessage(m_hNoBoxesSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_B));

  m_hNoBoxesBox = GetDlgItem(m_hwnd, IDC_NBOXES_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(m_pAppSettings->GetLongParameter(LP_B)));
  SendMessage(m_hNoBoxesBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

  m_hSafetySlider = GetDlgItem(m_hwnd, IDC_SAFETY_SLIDER);
  SendMessage(m_hSafetySlider, TBM_SETPAGESIZE, 0L, 10); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hSafetySlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(0, 256));
  SendMessage(m_hSafetySlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_S));

  m_hSafetyBox = GetDlgItem(m_hwnd, IDC_SAFETY_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(m_pAppSettings->GetLongParameter(LP_S)));
  SendMessage(m_hSafetyBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

  m_hNonUniformSlider = GetDlgItem(m_hwnd, IDC_NONUNIFORM_SLIDER);
  SendMessage(m_hNonUniformSlider, TBM_SETPAGESIZE, 0L, 50); // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(m_hNonUniformSlider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(-400, 400));
  SendMessage(m_hNonUniformSlider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_R));

  m_hNonUniformBox = GetDlgItem(m_hwnd, IDC_NU_VALUE);  
  _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(m_pAppSettings->GetLongParameter(LP_R)));
  SendMessage(m_hNonUniformBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
}

bool CButtonSettingsPage::Validate() {
  // Return false if something is wrong to prevent user from clicking to a different page. Please also pop up a dialogue informing the user at this point.
  return TRUE;
}

bool CButtonSettingsPage::Apply() 
{

  double NewValue;

  NewValue = SendMessage(m_hZoomStepsSlider, TBM_GETPOS, 0, 0);
  m_pAppSettings->SetLongParameter( LP_ZOOMSTEPS, static_cast<int>(NewValue));

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
		switch (LOWORD(wParam)) {
		case (IDC_DISPLAY):
			break;
		}
		break;
  case WM_HSCROLL: {
    double dNewValue;

    dNewValue = SendMessage(m_hZoomStepsSlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(dNewValue));
    SendMessage(m_hZoomStepsBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    dNewValue = SendMessage(m_hRightZoomSlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%0.2f"), dNewValue);
    SendMessage(m_hRightZoomBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    dNewValue = SendMessage(m_hNoBoxesSlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(dNewValue));
    SendMessage(m_hNoBoxesBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    dNewValue = SendMessage(m_hSafetySlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(dNewValue));
    SendMessage(m_hSafetyBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

    dNewValue = SendMessage(m_hNonUniformSlider, TBM_GETPOS, 0, 0);
    _sntprintf(m_tcBuffer, 100, TEXT("%d"), static_cast<int>(dNewValue));
    SendMessage(m_hNonUniformBox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

		return TRUE;
    }
  }
  // pass on to superclass:
  return CPrefsPageBase::WndProc(Window, message, wParam, lParam);
}
