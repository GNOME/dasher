// WinOptions.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "WinOptions.h"

using namespace WinUTF8;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/* TODO: Consider using Template functions to make this neater. */

CWinOptions::CWinOptions(const std::string &Group, const std::string &Product, Dasher::CEventHandler *pEventHandler):CSettingsStore(pEventHandler) {
  // Windows requires strings as Tstring
  Tstring TGroup, TProduct;
  UTF8string_to_wstring(Group, TGroup);
  UTF8string_to_wstring(Product, TProduct);

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

  LoadPersistent();
}

CWinOptions::~CWinOptions() {
  RegCloseKey(ProductKey);
}

/* Private functions -- Used to load and save to the Windows Registry
--------------------------------------------------------------------------*/

bool CWinOptions::LoadSetting(const std::string &Key, bool *Value) {
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

bool CWinOptions::LoadSetting(const std::string &Key, long *Value) {
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

bool CWinOptions::LoadSettingT(const std::string &Key, Tstring *TValue) {
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

bool CWinOptions::LoadSetting(const std::string &Key, std::string *Value) {

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

void CWinOptions::SaveSetting(const std::string &Key, const LPWINDOWPLACEMENT pwp) {
  DASHER_ASSERT(pwp != NULL);

  TCHAR t[200];
  _stprintf(t, FormatWindowPlacement, pwp->flags, pwp->showCmd, pwp->ptMinPosition.x, pwp->ptMinPosition.y, pwp->ptMaxPosition.x, pwp->ptMaxPosition.y, pwp->rcNormalPosition.left, pwp->rcNormalPosition.top, pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);

  Tstring ts(t);
  SaveSettingT(Key, ts);

}

/////////////////////////////////////////////////////////////////////////////

bool CWinOptions::LoadSetting(const std::string &Key, LPWINDOWPLACEMENT pwp) {
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

void CWinOptions::SaveSetting(const std::string &Key, bool Value) {
  if(Value)
    SaveSetting(Key, 1l);
  else
    SaveSetting(Key, 0l);
}

/////////////////////////////////////////////////////////////////////////////

void CWinOptions::SaveSetting(const std::string &Key, long Value) {
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

void CWinOptions::SaveSettingT(const std::string &Key, const Tstring &TValue) {
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

void CWinOptions::SaveSetting(const std::string &Key, const std::string &Value) {

  // DJW20031107 - i think Values should also be converted to Tstring
  Tstring TValue;
  UTF8string_to_wstring(Value, TValue);

  SaveSettingT(Key, TValue);
}

// Used for getting a handle on the Dasher key in the constructor.
int CWinOptions::GetOrCreate(HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired, HKEY *lpNewKey) {
  if(!(RegOpenKeyEx(hKey, lpSubKey, 0, samDesired, lpNewKey) == ERROR_SUCCESS)) {

    if(!(RegCreateKeyEx(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, samDesired, NULL, lpNewKey, NULL) == ERROR_SUCCESS)) {

      return (1);
    }
  }
  return 0;
}

bool CWinOptions::GetlpByte(const Tstring &Key, BYTE **Data) const {
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
