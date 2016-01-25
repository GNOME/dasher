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
