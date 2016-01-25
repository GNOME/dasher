#pragma once

#include "../Common/AppSettingsHeader.h"
#include "../DasherCore/SettingsStore.h"

#include <memory>
#include <vector>
#include <string>

#include <windows.h>

//#include "WinCommon.h"
#include "WinUTF8.h"

// Define first int value of the first element of each type.
// Useful for offsetting into specific arrays,
// since each setting is a unique int, but all 3 arrays start at 0
#define FIRST_APP_BP END_OF_SPS
#define FIRST_APP_LP END_OF_APP_BPS
#define FIRST_APP_SP END_OF_APP_LPS

#define NUM_OF_APP_BPS (END_OF_APP_BPS - END_OF_SPS)
#define NUM_OF_APP_LPS (END_OF_APP_LPS - END_OF_APP_BPS)
#define NUM_OF_APP_SPS (END_OF_APP_SPS - END_OF_APP_LPS)

namespace Dasher {
  class CDasher;
}

class CAppSettings
{
public:
  // The constructor takes ownership of the settings_store.   
  CAppSettings(Dasher::CDasher *pDasher, HWND hWnd, Dasher::CSettingsStore* settings_store);
  ~CAppSettings(void);

  ///
  /// The following all just wrap corresponding methods in CDasher,
  /// augmented for dealing with platform-specific parameters (APP_*),
  /// allowing the AppSettings to be used as a central point-of-access
  /// to all the settings data necessary for the GUI.
  ///
  bool GetBoolParameter(int iParameter);
  void SetBoolParameter(int iParameter, bool bValue);
  long GetLongParameter(int iParameter);
  void SetLongParameter(int iParameter, long iValue);
  std::string GetStringParameter(int iParameter);
  void SetStringParameter(int iParameter, const std::string &strValue);
  void ResetParamater(int iParameter);
  void GetPermittedValues(int iParameter, std::vector<std::string> &vList);

  void SetHwnd(HWND hWnd) {
    m_hWnd = hWnd;

  };

  void SetDasher(Dasher::CDasher *pDasher) {
    m_pDasher = pDasher;
  };

private:

  Dasher::CDasher *m_pDasher;
  HWND m_hWnd;

  std::unique_ptr<Dasher::CSettingsStore> settings_store_;
};
