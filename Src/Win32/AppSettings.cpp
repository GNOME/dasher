#include "WinCommon.h"

#include <stdio.h>

#include ".\AppSettings.h"
#include "../Common/AppSettingsData.h"
#include ".\Dasher.h"
#include "../DasherCore/Event.h"

using namespace WinUTF8;
using namespace std;

CAppSettings::CAppSettings(Dasher::CDasher *pDasher, HWND hWnd, Dasher::CSettingsStore* settings_store) : settings_store_(settings_store)
{
  m_hWnd = hWnd;
  m_pDasher = pDasher;

  settings_store_->AddParameters(app_boolparamtable, NUM_OF_APP_BPS);
  settings_store_->AddParameters(app_longparamtable, NUM_OF_APP_LPS);
  settings_store_->AddParameters(app_stringparamtable, NUM_OF_APP_SPS);
}

CAppSettings::~CAppSettings(void)
{
}

bool CAppSettings::GetBoolParameter(int iParameter) {
  return settings_store_->GetBoolParameter(iParameter);
}

void CAppSettings::SetBoolParameter(int iParameter, bool bValue) {
  settings_store_->SetBoolParameter(iParameter, bValue);
  m_pDasher->HandleEvent(iParameter);
}

long CAppSettings::GetLongParameter(int iParameter) {
  return settings_store_->GetLongParameter(iParameter);
}

void CAppSettings::SetLongParameter(int iParameter, long iValue) {
  settings_store_->SetLongParameter(iParameter, iValue);
  m_pDasher->HandleEvent(iParameter);
}

std::string CAppSettings::GetStringParameter(int iParameter) {
  return settings_store_->GetStringParameter(iParameter);
}

void CAppSettings::SetStringParameter(int iParameter, const std::string &strValue) {
  settings_store_->SetStringParameter(iParameter, strValue);
  m_pDasher->HandleEvent(iParameter);
}

void CAppSettings::ResetParamater(int iParameter) {
  settings_store_->ResetParameter(iParameter);
}

void CAppSettings::GetPermittedValues(int iParameter, vector<string> &vList) {
  //Don't think there are any app-specific string parameters with permitted values?
  DASHER_ASSERT(iParameter>=FIRST_SP && iParameter < END_OF_SPS);
  DASHER_ASSERT(m_pDasher != NULL);
  m_pDasher->GetPermittedValues(iParameter,vList);
}

static const char FormatWindowPlacement[] = "%u,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d";

/////////////////////////////////////////////////////////////////////////////
#ifndef _WIN32_WCE

void CAppSettings::SaveWindowPlacement(int iParameter, const LPWINDOWPLACEMENT pwp, int sp) {
  DASHER_ASSERT(pwp != NULL);

  char t[200];
  sprintf_s(t, sizeof(t), FormatWindowPlacement, pwp->flags, pwp->showCmd, pwp->ptMinPosition.x, pwp->ptMinPosition.y, pwp->ptMaxPosition.x, pwp->ptMaxPosition.y, pwp->rcNormalPosition.left, pwp->rcNormalPosition.top, pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom,sp);
  SetStringParameter(iParameter, t);
}

/////////////////////////////////////////////////////////////////////////////

bool CAppSettings::LoadWindowPlacement(int iParameter, LPWINDOWPLACEMENT pwp, int* psp) {
  DASHER_ASSERT(pwp != NULL);

  auto str = GetStringParameter(iParameter);
  if (str.empty())
    return false;

  WINDOWPLACEMENT wp;
  int nRead = sscanf_s(str.c_str(), FormatWindowPlacement,
                       &wp.flags, &wp.showCmd,
                       &wp.ptMinPosition.x, &wp.ptMinPosition.y,
                       &wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
                       &wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
                       &wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom, psp);

  if(nRead != 11)
    return false;
  wp.length = sizeof(wp);
  *pwp = wp;
  return true;
}

#endif
