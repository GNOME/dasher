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
#include "../ButtonPrefs.h"


#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;


CControlPage::CControlPage(HWND Parent, CDasherInterfaceBase *DI, CAppSettings *pAppSettings)
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
  {BP_PAUSE_OUTSIDE, IDC_WINDOWPAUSE},
  {BP_AUTO_SPEEDCONTROL, IDC_AUTOSPEED},
  {BP_AUTOCALIBRATE, IDC_AUTOCALIBRATE}
};

static menuentry listtable[] = {
  {SP_INPUT_FILTER, IDC_CONTROL_LIST},
  {SP_INPUT_DEVICE, IDC_INPUT_LIST}
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

  m_hMousePosStyle = GetDlgItem(m_hwnd, IDC_MOUSEPOS_STYLE);
  SendMessage(m_hMousePosStyle, CB_ADDSTRING, 0, (LPARAM)L"Centre circle");
  SendMessage(m_hMousePosStyle, CB_ADDSTRING, 0, (LPARAM)L"Two box");

  if(m_pAppSettings->GetBoolParameter(BP_MOUSEPOS_MODE)) {
    SendMessage(m_hMousePosStyle, CB_SETCURSEL, 1, 0);
  }
  else {
    SendMessage(m_hMousePosStyle, CB_SETCURSEL, 0, 0);
  }

  if(m_pAppSettings->GetBoolParameter(BP_MOUSEPOS_MODE) || m_pAppSettings->GetBoolParameter(BP_CIRCLE_START)) {
    SendMessage(GetDlgItem(m_hwnd, IDC_MOUSEPOS), BM_SETCHECK, BST_CHECKED, 0);
  }
  else {
    SendMessage(GetDlgItem(m_hwnd, IDC_MOUSEPOS), BM_SETCHECK, BST_UNCHECKED, 0);
  }

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

  // List entries:

  for(int i(0); i<sizeof(listtable)/sizeof(menuentry); ++i) {
 // {
//    int i(0);
    std::vector<std::string> vValues;
    m_pDasherInterface->GetPermittedValues(listtable[i].paramNum, vValues);

    for(std::vector<std::string>::iterator it(vValues.begin()); it != vValues.end(); ++it) {
      Tstring Item;
      WinUTF8::UTF8string_to_wstring(*it, Item);
      int iIdx(SendMessage(GetDlgItem(m_hwnd, listtable[i].idcNum), LB_ADDSTRING, 0, (LPARAM) Item.c_str()));

      if(*it == m_pAppSettings->GetStringParameter(listtable[i].paramNum))
        SendMessage(GetDlgItem(m_hwnd, listtable[i].idcNum), LB_SETCURSEL, iIdx, 0);
    }
  }

// TODO: Pretty horrible - sort this out
  {
        //CButtonPrefs ButtonPrefs(m_hwnd, 0, m_pAppSettings);
        int iSelection(SendMessage(GetDlgItem(m_hwnd, IDC_CONTROL_LIST), LB_GETCURSEL, 0, 0));
    
        int iLength(SendMessage(GetDlgItem(m_hwnd, IDC_CONTROL_LIST), LB_GETTEXTLEN, iSelection, 0));
        TCHAR *szData(new TCHAR[iLength + 1]);
        SendMessage(GetDlgItem(m_hwnd, IDC_CONTROL_LIST), LB_GETTEXT, iSelection, (LPARAM)szData);

        std::string strNewValue;
        WinUTF8::wstring_to_UTF8string(szData, strNewValue);
        delete[] szData;

        SModuleSettings *pSettings;
        int iSettingsCount;

        EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON_PREFS), m_pDasherInterface->GetModuleSettings(strNewValue, &pSettings, &iSettingsCount));
      }
      
      {
        int iSelection(SendMessage(GetDlgItem(m_hwnd, IDC_INPUT_LIST), LB_GETCURSEL, 0, 0));
    
		if(iSelection != LB_ERR) {
			int iLength(SendMessage(GetDlgItem(m_hwnd, IDC_INPUT_LIST), LB_GETTEXTLEN, iSelection, 0));
			TCHAR *szData(new TCHAR[iLength + 1]);
			SendMessage(GetDlgItem(m_hwnd, IDC_INPUT_LIST), LB_GETTEXT, iSelection, (LPARAM)szData);

			std::string strNewValue;
			WinUTF8::wstring_to_UTF8string(szData, strNewValue);
			delete[] szData;

	        SModuleSettings *pSettings;
			int iSettingsCount;

			EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON_PREFS2), m_pDasherInterface->GetModuleSettings(strNewValue, &pSettings, &iSettingsCount));
		}
		else {
			EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON_PREFS2), false);
		}
      }
      

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

  if(SendMessage(GetDlgItem(m_hwnd, IDC_MOUSEPOS), BM_GETCHECK, 0, 0) == BST_CHECKED ) {
    int iComboIdx;
    iComboIdx = SendMessage(m_hMousePosStyle, CB_GETCURSEL, 0, 0);

    if(iComboIdx == 0) {
      m_pAppSettings->SetBoolParameter(BP_MOUSEPOS_MODE, false);
      m_pAppSettings->SetBoolParameter(BP_CIRCLE_START, true);
    }
    else {
      m_pAppSettings->SetBoolParameter(BP_MOUSEPOS_MODE, true);
      m_pAppSettings->SetBoolParameter(BP_CIRCLE_START, false);
    }
  }
  else {
    m_pAppSettings->SetBoolParameter(BP_MOUSEPOS_MODE, false);
    m_pAppSettings->SetBoolParameter(BP_CIRCLE_START, false);
  }

  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    m_pAppSettings->SetBoolParameter(menutable[ii].paramNum, 
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED );
  }

  for(int i(0); i < sizeof(listtable)/sizeof(menuentry); ++i) {
    int iSelection(SendMessage(GetDlgItem(m_hwnd, listtable[i].idcNum), LB_GETCURSEL, 0, 0));
    
    int iLength(SendMessage(GetDlgItem(m_hwnd, listtable[i].idcNum), LB_GETTEXTLEN, iSelection, 0));
    TCHAR *szData(new TCHAR[iLength + 1]);
    SendMessage(GetDlgItem(m_hwnd, listtable[i].idcNum), LB_GETTEXT, iSelection, (LPARAM)szData);

    std::string strNewValue;
    WinUTF8::wstring_to_UTF8string(szData, strNewValue);
    delete[] szData;

    m_pAppSettings->SetStringParameter(listtable[i].paramNum, strNewValue);
  }

	// Return false (and notify the user) if something is wrong.
	return TRUE;
}

LRESULT CControlPage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  // TODO: Why isn't this in case?
  if(message == WM_MS_CLOSE) {
    if(m_pModuleSettingsDialogue) {
      m_pModuleSettingsDialogue->DestroyWindow();
    }

    m_pModuleSettingsDialogue = NULL;
    EnableWindow(m_hwnd, true);
    return 0;
  }

	double NewSpeed;
	switch (message) 
	{	
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
	/*	case (IDC_DISPLAY):
			break;*/
    case IDC_CONTROL_LIST:
      {
        int iSelection(SendMessage(GetDlgItem(m_hwnd, IDC_CONTROL_LIST), LB_GETCURSEL, 0, 0));

        if(iSelection != LB_ERR) {    
          int iLength(SendMessage(GetDlgItem(m_hwnd, IDC_CONTROL_LIST), LB_GETTEXTLEN, iSelection, 0));
          TCHAR *szData(new TCHAR[iLength + 1]);
          SendMessage(GetDlgItem(m_hwnd, IDC_CONTROL_LIST), LB_GETTEXT, iSelection, (LPARAM)szData);
  
          std::string strNewValue;
          WinUTF8::wstring_to_UTF8string(szData, strNewValue);
          delete[] szData;
  
          SModuleSettings *pSettings;
          int iSettingsCount;

          EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON_PREFS), m_pDasherInterface->GetModuleSettings(strNewValue, &pSettings, &iSettingsCount));
        }
        else {
          EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON_PREFS), false);
        }
      }
      break;
    case IDC_INPUT_LIST:
      {
        int iSelection(SendMessage(GetDlgItem(m_hwnd, IDC_INPUT_LIST), LB_GETCURSEL, 0, 0));
    
	    	if(iSelection != LB_ERR) {
          int iLength(SendMessage(GetDlgItem(m_hwnd, IDC_INPUT_LIST), LB_GETTEXTLEN, iSelection, 0));
          TCHAR *szData(new TCHAR[iLength + 1]);
          SendMessage(GetDlgItem(m_hwnd, IDC_INPUT_LIST), LB_GETTEXT, iSelection, (LPARAM)szData);

          std::string strNewValue;
          WinUTF8::wstring_to_UTF8string(szData, strNewValue);
          delete[] szData;

          SModuleSettings *pSettings;
          int iSettingsCount;

          EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON_PREFS2), m_pDasherInterface->GetModuleSettings(strNewValue, &pSettings, &iSettingsCount));
        }
        else {
          EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON_PREFS2), false);
        }
      }
      break;
    case IDC_BUTTON_PREFS: 
      {
        int iSelection(SendMessage(GetDlgItem(m_hwnd, listtable[0].idcNum), LB_GETCURSEL, 0, 0));
      
        if(iSelection != LB_ERR) {
          int iLength(SendMessage(GetDlgItem(m_hwnd, listtable[0].idcNum), LB_GETTEXTLEN, iSelection, 0));
          TCHAR *szData(new TCHAR[iLength + 1]);
          SendMessage(GetDlgItem(m_hwnd, listtable[0].idcNum), LB_GETTEXT, iSelection, (LPARAM)szData);

          std::string strNewValue;
          WinUTF8::wstring_to_UTF8string(szData, strNewValue);
          delete[] szData;

          SModuleSettings *pSettings;
          int iSettingsCount;

          if(!m_pDasherInterface->GetModuleSettings(strNewValue, &pSettings, &iSettingsCount))
            break;

          RECT sRect;

          m_pModuleSettingsDialogue = new CModuleSettings(strNewValue, pSettings, iSettingsCount, m_pDasherInterface);
          m_pModuleSettingsDialogue->Create(m_hwnd, &sRect);
          m_pModuleSettingsDialogue->ShowWindow(SW_RESTORE);
        
          EnableWindow(m_hwnd, false);
        }
      }
		  break;
    case IDC_BUTTON_PREFS2: 
      {
        int iSelection(SendMessage(GetDlgItem(m_hwnd, listtable[1].idcNum), LB_GETCURSEL, 0, 0));
    
        if(iSelection != LB_ERR) {
          int iLength(SendMessage(GetDlgItem(m_hwnd, listtable[1].idcNum), LB_GETTEXTLEN, iSelection, 0));
          TCHAR *szData(new TCHAR[iLength + 1]);
          SendMessage(GetDlgItem(m_hwnd, listtable[1].idcNum), LB_GETTEXT, iSelection, (LPARAM)szData);

          std::string strNewValue;
          WinUTF8::wstring_to_UTF8string(szData, strNewValue);
          delete[] szData;

          SModuleSettings *pSettings;
          int iSettingsCount;

          if(!m_pDasherInterface->GetModuleSettings(strNewValue, &pSettings, &iSettingsCount))
            break;

          RECT sRect;

          m_pModuleSettingsDialogue = new CModuleSettings(strNewValue, pSettings, iSettingsCount, m_pDasherInterface);
          m_pModuleSettingsDialogue->Create(m_hwnd, &sRect);
          m_pModuleSettingsDialogue->ShowWindow(SW_RESTORE);
        
          EnableWindow(m_hwnd, false);
        }
      }
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
