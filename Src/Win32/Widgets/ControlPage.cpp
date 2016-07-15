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
#include "../ModuleSettings.h"


#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;


CControlPage::CControlPage(HWND Parent, CDasherInterfaceBase *DI, CAppSettings *pAppSettings)
:CPrefsPageBase(Parent, pAppSettings), m_pDasherInterface(DI) {
}

struct menuentry {
  int paramNum; // enum value in Parameters.h for setting store
  int idcNum;   // #define value in resource.h for dasher.rc
};

// List of menu items that will be displayed in the General Preferences
static menuentry menutable[] = {
  {BP_START_MOUSE, IDC_LEFT},
  {BP_START_SPACE, IDC_SPACE},
  {BP_STOP_OUTSIDE, IDC_WINDOWPAUSE},
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
  SendMessage(SB_slider, TBM_SETRANGE, FALSE, (LPARAM)MAKELONG(10, 800));

  speedbox = GetDlgItem(m_hwnd, IDC_SPEEDVAL);

  SendMessage(SB_slider, TBM_SETPOS, TRUE, (LPARAM)m_pAppSettings->GetLongParameter(LP_MAX_BITRATE));
  _sntprintf(m_tcBuffer, 100, TEXT("%0.2f"), m_pAppSettings->GetLongParameter(LP_MAX_BITRATE) / 100.0);
  SendMessage(speedbox, WM_SETTEXT, 0, (LPARAM)m_tcBuffer);

  m_hMousePosStyle = GetDlgItem(m_hwnd, IDC_MOUSEPOS_STYLE);
  SendMessage(m_hMousePosStyle, CB_ADDSTRING, 0, (LPARAM)L"Centre circle");
  SendMessage(m_hMousePosStyle, CB_ADDSTRING, 0, (LPARAM)L"Two box");

  if (m_pAppSettings->GetBoolParameter(BP_MOUSEPOS_MODE)) {
    SendMessage(m_hMousePosStyle, CB_SETCURSEL, 1, 0);
  }
  else {
    SendMessage(m_hMousePosStyle, CB_SETCURSEL, 0, 0);
  }

  if (m_pAppSettings->GetBoolParameter(BP_MOUSEPOS_MODE) || m_pAppSettings->GetBoolParameter(BP_CIRCLE_START)) {
    SendMessage(GetDlgItem(m_hwnd, IDC_MOUSEPOS), BM_SETCHECK, BST_CHECKED, 0);
  }
  else {
    SendMessage(GetDlgItem(m_hwnd, IDC_MOUSEPOS), BM_SETCHECK, BST_UNCHECKED, 0);
  }

  // all the button checkboxes
  for (int ii = 0; ii < sizeof(menutable) / sizeof(menuentry); ii++)
  {
    if (m_pAppSettings->GetBoolParameter(menutable[ii].paramNum))
    {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
    }
    else
    {
      SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_UNCHECKED, 0);
    }
  }

  // List entries:

  for (int i(0); i < sizeof(listtable) / sizeof(menuentry); ++i) {
    std::vector<std::string> vValues;
    m_pAppSettings->GetPermittedValues(listtable[i].paramNum, vValues);
    CWindow lbox = GetDlgItem(m_hwnd, listtable[i].idcNum);
    auto currentParameterValue = m_pAppSettings->GetStringParameter(listtable[i].paramNum);
    for (auto value : vValues) {
      wstring Item;
      WinUTF8::UTF8string_to_wstring(value, Item);
      int iIdx = lbox.SendMessage(LB_ADDSTRING, 0, (LPARAM)Item.c_str());

      if (value == currentParameterValue)
        lbox.SendMessage(LB_SETCURSEL, iIdx, 0);
    }
  }
  EnablePrefferencesForSelectedModule(IDC_CONTROL_LIST, IDC_BUTTON_PREFS);
  EnablePrefferencesForSelectedModule(IDC_INPUT_LIST, IDC_BUTTON_PREFS2);
}
bool CControlPage::Apply()
{
  int NewSpeed = SendMessage(SB_slider, TBM_GETPOS, 0, 0);
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
    std::string strNewValue;
	if (GetSelectedListboxText(listtable[i].idcNum, strNewValue))
	{
		m_pAppSettings->SetStringParameter(listtable[i].paramNum, strNewValue);
	}
  }

	// Return false (and notify the user) if something is wrong.
	return TRUE;
}

LRESULT CControlPage::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  double NewSpeed;
  switch (message)
  {
  case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	case IDC_CONTROL_LIST:
	  EnablePrefferencesForSelectedModule(IDC_CONTROL_LIST, IDC_BUTTON_PREFS);
	  break;
	case IDC_INPUT_LIST:
	  EnablePrefferencesForSelectedModule(IDC_INPUT_LIST, IDC_BUTTON_PREFS2);
	  break;
	case IDC_BUTTON_PREFS:
	{
	  std::string strNewValue;
	  if (!GetSelectedListboxText(IDC_CONTROL_LIST, strNewValue))
		break;

	  SModuleSettings *pSettings = 0;
	  int iSettingsCount = 0;
	  if (!m_pDasherInterface->GetModuleSettings(strNewValue, &pSettings, &iSettingsCount))
		break;

	  CModuleSettings dlg(strNewValue, pSettings, iSettingsCount, m_pAppSettings);
	  dlg.DoModal(m_hwnd);
	}
	break;
	case IDC_BUTTON_PREFS2:
	{
	  std::string strNewValue;
	  if (!GetSelectedListboxText(IDC_INPUT_LIST, strNewValue))
		break;

	  SModuleSettings *pSettings = 0;
	  int iSettingsCount = 0;
	  if (!m_pDasherInterface->GetModuleSettings(strNewValue, &pSettings, &iSettingsCount))
		break;

	  CModuleSettings dlg(strNewValue, pSettings, iSettingsCount, m_pAppSettings);
	  dlg.DoModal(m_hwnd);
	}
	break;
	}
	break;
  case WM_HSCROLL:
	if ((LOWORD(wParam) == SB_THUMBPOSITION) | (LOWORD(wParam) == SB_THUMBTRACK)) {
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
	  SendMessage(speedbox, WM_SETTEXT, 0, (LPARAM)m_tcBuffer);
	}
	return TRUE;
	break;

  }
  // pass on to superclass:
  return CPrefsPageBase::WndProc(Window, message, wParam, lParam);
}

bool CControlPage::GetSelectedListboxText(int lboxId, string& selectedText)
{
  CWindow lbox = GetDlgItem(m_hwnd, lboxId);
  int iSelection = lbox.SendMessage(LB_GETCURSEL, 0, 0);
  if (iSelection == LB_ERR)
	return false;

  int iLength = lbox.SendMessage(LB_GETTEXTLEN, iSelection, 0);
  if (iLength == LB_ERR)
	return false;

  TCHAR* szData = new TCHAR[iLength + 1];
  int iLen2 = lbox.SendMessage(LB_GETTEXT, iSelection, (LPARAM)szData);
  if (iLen2 == LB_ERR) {
	delete[] szData;
	return false;
  }
  WinUTF8::wstring_to_UTF8string(szData, selectedText);
  delete[] szData;
  return true;

}

void CControlPage::EnablePrefferencesForSelectedModule(int lboxId, int btnId)
{
  std::string strValue;
  GetSelectedListboxText(lboxId, strValue);
  SModuleSettings *pSettings = 0;
  int iSettingsCount = 0;
  EnableWindow(GetDlgItem(m_hwnd, btnId),
	m_pDasherInterface->GetModuleSettings(strValue, &pSettings, &iSettingsCount));
}
