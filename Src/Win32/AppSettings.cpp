#include "WinCommon.h"

#include ".\AppSettings.h"
#include "../Common/AppSettingsData.h"
#include ".\Dasher.h"
#include "../DasherCore/Event.h"

using namespace WinUTF8;
using namespace std;

CAppSettings::CAppSettings(Dasher::CDasher *pDasher, HWND hWnd)
{
  m_hWnd = hWnd;
  m_pDasher = pDasher;

  // Initialise registry stuff

 // Windows requires strings as Tstring
  Tstring TGroup, TProduct;
  UTF8string_to_wstring("Inference Group", TGroup);
  UTF8string_to_wstring("Dasher3", TProduct);

  // Get hold of HKEY_CURRENT_USER\Software
  HKEY SoftwareKey;
  if(GetOrCreate(HKEY_CURRENT_USER, TEXT("Software"), KEY_WRITE, &SoftwareKey) != 0) {
    // Can't open or create key - do something...
    // Probably flag registry as failed and just return default options
    exit(1);                    // give up for now.
  }

  // Then HKEY_CURRENT_USER\Software\<Group>
  HKEY GroupKey;
  if(GetOrCreate(SoftwareKey, TGroup.c_str(), KEY_WRITE, &GroupKey) != 0) {
    exit(1);
  }
  RegCloseKey(SoftwareKey);

  // Then HKEY_CURRENT_USER\Software\<Group>\<Product>
  if(GetOrCreate(GroupKey, TProduct.c_str(), KEY_ALL_ACCESS, &ProductKey) != 0) {
    exit(1);
  }
  RegCloseKey(GroupKey);

 // ---


  m_pBoolTable = new bp_info[NUM_OF_APP_BPS];
  m_pLongTable = new lp_info[NUM_OF_APP_LPS];
  m_pStringTable = new sp_info[NUM_OF_APP_SPS];

  for(int ii = 0; ii < NUM_OF_APP_BPS; ii++) {
    m_pBoolTable[ii].key = app_boolparamtable[ii].key;
    if(!LoadSetting(app_boolparamtable[ii].regName, &m_pBoolTable[ii].value))
      m_pBoolTable[ii].value = app_boolparamtable[ii].bDefaultValue;
    m_pBoolTable[ii].defaultVal = app_boolparamtable[ii].bDefaultValue;
    m_pBoolTable[ii].humanReadable = app_boolparamtable[ii].humanReadable;
    m_pBoolTable[ii].persistent = app_boolparamtable[ii].persistent;
    m_pBoolTable[ii].regName = app_boolparamtable[ii].regName;
  } 
    
  for(int ii = 0; ii < NUM_OF_APP_LPS; ii++) {
    m_pLongTable[ii].key = app_longparamtable[ii].key;
    if(!LoadSetting(app_longparamtable[ii].regName, &m_pLongTable[ii].value))
      m_pLongTable[ii].value = app_longparamtable[ii].iDefaultValue;
    m_pLongTable[ii].defaultVal = app_longparamtable[ii].iDefaultValue;
    m_pLongTable[ii].humanReadable = app_longparamtable[ii].humanReadable;
    m_pLongTable[ii].persistent = app_longparamtable[ii].persistent;
    m_pLongTable[ii].regName = app_longparamtable[ii].regName;
  }

  for(int ii = 0; ii < NUM_OF_APP_SPS; ii++) {
    m_pStringTable[ii].key = app_stringparamtable[ii].key;
    if(!LoadSetting(app_stringparamtable[ii].regName, &m_pStringTable[ii].value)) 
      m_pStringTable[ii].value = app_stringparamtable[ii].szDefaultValue; 
    m_pStringTable[ii].defaultVal = app_stringparamtable[ii].szDefaultValue;
    m_pStringTable[ii].humanReadable = app_stringparamtable[ii].humanReadable;
    m_pStringTable[ii].persistent = app_stringparamtable[ii].persistent;
    m_pStringTable[ii].regName = app_stringparamtable[ii].regName;
  }
}

CAppSettings::~CAppSettings(void)
{
  RegCloseKey(ProductKey);

  delete[] m_pBoolTable;
  delete[] m_pLongTable;
  delete[] m_pStringTable;
}

bool CAppSettings::GetBoolParameter(int iParameter) {
  if( iParameter < END_OF_BPS )
    return m_pDasher->GetBoolParameter(iParameter);
  else
    return m_pBoolTable[iParameter - FIRST_APP_BP].value;
}

void CAppSettings::SetBoolParameter(int iParameter, bool bValue) {
  if( iParameter < END_OF_BPS )
    m_pDasher->SetBoolParameter(iParameter , bValue);
  else {
    m_pBoolTable[iParameter - FIRST_APP_BP].value = bValue;
    SaveSetting(m_pBoolTable[iParameter - FIRST_APP_BP].regName, bValue);

    Dasher::CParameterNotificationEvent oEvent(iParameter);
    SendMessage(m_hWnd, WM_DASHER_EVENT, 0, (LPARAM)&oEvent);
  }
}

long CAppSettings::GetLongParameter(int iParameter) {
  if( iParameter < END_OF_LPS)
    return m_pDasher->GetLongParameter(iParameter);
  else
    return m_pLongTable[iParameter - FIRST_APP_LP].value;
}

void CAppSettings::SetLongParameter(int iParameter, long iValue) {
  if( iParameter < END_OF_LPS )
    m_pDasher->SetLongParameter(iParameter, iValue);
  else {
    m_pLongTable[iParameter - FIRST_APP_LP].value = iValue;
    SaveSetting(m_pLongTable[iParameter - FIRST_APP_LP].regName, iValue); 
    Dasher::CParameterNotificationEvent oEvent(iParameter);
    SendMessage(m_hWnd, WM_DASHER_EVENT, 0, (LPARAM)&oEvent);
  }
}

std::string CAppSettings::GetStringParameter(int iParameter) {
  if(iParameter < END_OF_SPS)
    return m_pDasher->GetStringParameter(iParameter);
  else
    return m_pStringTable[iParameter - FIRST_APP_SP].value;
}

void CAppSettings::SetStringParameter(int iParameter, const std::string &strValue) {
  if(iParameter < END_OF_SPS)
    m_pDasher->SetStringParameter(iParameter, strValue);
  else {
    m_pStringTable[iParameter - FIRST_APP_SP].value = strValue;
    SaveSetting(m_pStringTable[iParameter - FIRST_APP_SP].regName, strValue);
    Dasher::CParameterNotificationEvent oEvent(iParameter);
    SendMessage(m_hWnd, WM_DASHER_EVENT, 0, (LPARAM)&oEvent);
  }
}

void CAppSettings::ResetParamater(int iParameter) {
  if(iParameter < END_OF_SPS)
    m_pDasher->ResetParameter(iParameter);
  else if(iParameter < END_OF_APP_BPS)
    SetBoolParameter(iParameter, app_boolparamtable[iParameter - FIRST_APP_BP].bDefaultValue);
  else if(iParameter < END_OF_APP_LPS)
    SetLongParameter(iParameter, app_longparamtable[iParameter - FIRST_APP_LP].iDefaultValue);
  else
    SetStringParameter(iParameter, app_stringparamtable[iParameter - FIRST_APP_SP].szDefaultValue);
}


// Functions for accessing persistent storage (stolen from WinOptions)

bool CAppSettings::LoadSetting(const std::string &Key, bool *Value) {
  Tstring TKey;
  UTF8string_to_wstring(Key, TKey);
  BYTE *Data = 0;

  if(!GetlpByte(TKey, &Data)) {
    delete[]Data;
    return false;
  }

  if((DWORD) * Data == 0)
    *Value = false;
  else
    *Value = true;

  delete[]Data;
  return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CAppSettings::LoadSetting(const std::string &Key, long *Value) {
  Tstring TKey;
  UTF8string_to_wstring(Key, TKey);
  BYTE *Data = 0;

  if(!GetlpByte(TKey, &Data)) {
    delete[]Data;
    return false;
  }

  // Evil casting to make sure I can retrieve signed longs, even
  // though windows registry only stores +ve values.
  *Value = *((long *)Data);
  delete[]Data;
  return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CAppSettings::LoadSettingT(const std::string &Key, Tstring *TValue) {
  Tstring TKey;
  UTF8string_to_wstring(Key, TKey);
  BYTE *Data = 0;

  if(!GetlpByte(TKey, &Data)) {
    delete[]Data;
    return false;
  }

  *TValue = (TCHAR *) Data;
  delete[]Data;

  return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CAppSettings::LoadSetting(const std::string &Key, std::string *Value) {

  Tstring str;
  if(LoadSettingT(Key, &str)) {
    wstring_to_UTF8string(str, *Value);
    return true;

  }
  return false;

}

/////////////////////////////////////////////////////////////////////////////

static TCHAR FormatWindowPlacement[] = TEXT("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");

/////////////////////////////////////////////////////////////////////////////
#ifndef DASHER_WINCE

void CAppSettings::SaveSetting(const std::string &Key, const LPWINDOWPLACEMENT pwp) {
  DASHER_ASSERT(pwp != NULL);

  TCHAR t[200];
  _stprintf(t, FormatWindowPlacement, pwp->flags, pwp->showCmd, pwp->ptMinPosition.x, pwp->ptMinPosition.y, pwp->ptMaxPosition.x, pwp->ptMaxPosition.y, pwp->rcNormalPosition.left, pwp->rcNormalPosition.top, pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);

  Tstring ts(t);
  SaveSettingT(Key, ts);

}

/////////////////////////////////////////////////////////////////////////////

bool CAppSettings::LoadSetting(const std::string &Key, LPWINDOWPLACEMENT pwp) {
  DASHER_ASSERT(pwp != NULL);

  Tstring str;

  if(!LoadSettingT(Key, &str))
    return false;

  WINDOWPLACEMENT wp;
  int nRead = _stscanf(str.c_str(), FormatWindowPlacement,
                       &wp.flags, &wp.showCmd,
                       &wp.ptMinPosition.x, &wp.ptMinPosition.y,
                       &wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
                       &wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
                       &wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

  if(nRead != 10)
    return false;
  wp.length = sizeof(wp);

  *pwp = wp;
  return true;

}

#endif
/////////////////////////////////////////////////////////////////////////////

void CAppSettings::SaveSetting(const std::string &Key, bool Value) {
  if(Value)
    SaveSetting(Key, 1l);
  else
    SaveSetting(Key, 0l);
}

/////////////////////////////////////////////////////////////////////////////

void CAppSettings::SaveSetting(const std::string &Key, long Value) {
  Tstring TKey;
  UTF8string_to_wstring(Key, TKey);
  // Evil casting. Registry stores DWORD's (unsigned longs)
  // I'm forcing in signed longs and if I force them out again in the same
  // way I should get a sensible result.
  DWORD *RegValue = (DWORD *) & Value;
  DWORD MemAllow = sizeof(DWORD);
  LONG ErrVal = RegSetValueEx(ProductKey, TKey.c_str(), 0,
                              REG_DWORD, (const unsigned char *)RegValue, MemAllow);
}

/////////////////////////////////////////////////////////////////////////////

void CAppSettings::SaveSettingT(const std::string &Key, const Tstring &TValue) {
  Tstring TKey;
  UTF8string_to_wstring(Key, TKey);

  DWORD MemAllow = (TValue.size() + 1) * sizeof(TCHAR);

  //const unsigned char* StrInput = (const unsigned char*) Value.c_str();
  //LONG ErrVal = RegSetValueEx(ProductKey, TKey.c_str(), 0,
  //      REG_SZ, StrInput, MemAllow);

  LONG ErrVal = RegSetValueEx(ProductKey, TKey.c_str(), 0,
                              REG_SZ, (CONST BYTE *) TValue.c_str(), MemAllow);

}

/////////////////////////////////////////////////////////////////////////////

void CAppSettings::SaveSetting(const std::string &Key, const std::string &Value) {

  // DJW20031107 - i think Values should also be converted to Tstring
  Tstring TValue;
  UTF8string_to_wstring(Value, TValue);

  SaveSettingT(Key, TValue);
}

// Used for getting a handle on the Dasher key in the constructor.
int CAppSettings::GetOrCreate(HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired, HKEY *lpNewKey) {
  if(!(RegOpenKeyEx(hKey, lpSubKey, 0, samDesired, lpNewKey) == ERROR_SUCCESS)) {

    if(!(RegCreateKeyEx(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, samDesired, NULL, lpNewKey, NULL) == ERROR_SUCCESS)) {

      return (1);
    }
  }
  return 0;
}

bool CAppSettings::GetlpByte(const Tstring &Key, BYTE **Data) const {
  *Data = new BYTE[1];
  unsigned long datasize = sizeof(BYTE[1]);
  DWORD Type;

  LONG ErrVal = RegQueryValueEx(ProductKey, Key.c_str(), 0, &Type, *Data, &datasize);
  while(ErrVal == ERROR_MORE_DATA) {
    delete[] * Data;
    *Data = new BYTE[datasize];
    ErrVal = RegQueryValueEx(ProductKey, Key.c_str(), 0, &Type, *Data, &datasize);
  }

  // Perhaps I should spend lots of time working out why the registry doesn't work.
  // when this fails. Would probably just confuse users though. Users with a broken
  // registry can live with defaults :)
  if((ErrVal == ERROR_SUCCESS) && (*Data != 0))
    return true;
  else
    return false;
}

