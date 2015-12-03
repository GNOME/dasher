// WinHelper.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace WinHelper {
  HINSTANCE hInstApp;
} void WinHelper::LastWindowsError() {
  LPVOID lpMsgBuf;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   // Default language
                (LPTSTR) & lpMsgBuf, 0, NULL);
  MessageBox(NULL, (LPCTSTR) lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION);
  LocalFree(lpMsgBuf);
}

namespace {
  bool CommonControlsInited = false;
}

void WinHelper::InitCommonControlLib() {
  if(CommonControlsInited)
    return;

  INITCOMMONCONTROLSEX iccex;
  iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  iccex.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES;
  InitCommonControlsEx(&iccex);
  CommonControlsInited = true;
}

namespace {
// For SHGetSpecialFolderPath.
#include "shlobj.h"
}

void WinHelper::GetUserDirectory(Tstring *Output) {
  Tstring & UserData = *Output;
  TCHAR Buffer[MAX_PATH];
  // My documentation says SHGetSpecialFolderPath returns NOERROR if successful
  // With my headers NOERROR==0 and this function returns TRUE==1 as docs online say.
  // Not sure I trust the return value, so I'm just going to assume if I have a string
  // it is probably correct :)
  Buffer[0] = TEXT('\0');
  SHGetSpecialFolderPath(NULL, Buffer, CSIDL_APPDATA, 0);
  UserData = Buffer;
  if(UserData[UserData.size() - 1] != TEXT('\\'))
    UserData += TEXT('\\');
}

void WinHelper::GetAppDirectory(Tstring *Output) {
  Tstring & AppLocation = *Output;

  TCHAR Buffer[MAX_PATH];
  if(0 != GetModuleFileName(NULL, Buffer, MAX_PATH)) {
    TCHAR *pos = _tcsrchr(Buffer, TEXT('\\'));  // 3 line hack to remove filename
    pos++;                      //
    *pos = TEXT('\0');          //
    AppLocation = Buffer;
  }
  else
    AppLocation = TEXT("");     // Cop out. Current directory may be elsewhere.
  // Hopefully it won't come to this...
}
