#pragma once

#include <string>
#include "../Common/AppSettingsHeader.h"

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

  CAppSettings(Dasher::CDasher *pDasher, HWND hWnd);
  ~CAppSettings(void);

  ///
  /// Get a boolean parameter

  bool GetBoolParameter(int iParameter);

  ///
  /// Set a boolean parameter

  void SetBoolParameter(int iParameter, bool bValue);

  ///
  /// Get a long integer parameter

  long GetLongParameter(int iParameter);

  ///
  /// Set a long integer parameter

  void SetLongParameter(int iParameter, long iValue);

  ///
  /// Get a string parameter

  std::string GetStringParameter(int iParameter);

  ///
  /// Set a string parameter

  void SetStringParameter(int iParameter, const std::string &strValue);

  ///
  /// Reset a parameter to its default value

  void ResetParamater(int iParameter);

  #ifndef DASHER_WINCE
  bool LoadSetting(const std::string & Key, LPWINDOWPLACEMENT pwp);
  void SaveSetting(const std::string & Key, const LPWINDOWPLACEMENT pwp);
#endif

  void SetHwnd(HWND hWnd) {
    m_hWnd = hWnd;
  };

  void SetDasher(Dasher::CDasher *pDasher) {
    m_pDasher = pDasher;
  };

private:
  struct bp_info {
    int key;
    std::string regName;
    bool persistent;
    bool value;
    bool defaultVal;
    std::string humanReadable;
  };
  struct lp_info {
    int key;
    std::string regName;
    bool persistent;
    long value;
    long defaultVal;
    std::string humanReadable;
  };
  struct sp_info {
    int key;
    std::string regName;
    bool persistent;
    std::string value;
    std::string defaultVal;
    std::string humanReadable;
  };

  bp_info *m_pBoolTable;
  lp_info *m_pLongTable;
  sp_info *m_pStringTable;

  Dasher::CDasher *m_pDasher;
  HWND m_hWnd;

   bool LoadSetting(const std::string & Key, bool * Value);
  bool LoadSetting(const std::string & Key, long *Value);
  bool LoadSetting(const std::string & Key, std::string * Value);
  bool LoadSettingT(const std::string & Key, Tstring * Value);

  void SaveSetting(const std::string & Key, bool Value);
  void SaveSetting(const std::string & Key, long Value);
  void SaveSetting(const std::string & Key, const std::string & Value);

  void SaveSettingT(const std::string & Key, const Tstring & TValue);

  // Platform Specific helpers
  HKEY ProductKey;
  int GetOrCreate(HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired, HKEY * lpNewKey);
  // CARE! Users of GetlpByte must call delete[] on *Data after use.
  bool GetlpByte(const Tstring & key, BYTE ** Data) const;


};
