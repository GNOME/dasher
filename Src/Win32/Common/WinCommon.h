// WinCommon.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WinCommon_h__
#define __WinCommon_h__
// Windows Vista 
#define _WIN32_WINNT 0x6000 
//#pragma message("Compiling WinCommon.h.........................")

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN     // Excludes rarely-used stuff from the Windows headers
#define STRICT
#define _ATL_NO_COM_SUPPORT
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_ENABLE_PTM_WARNING

#include <atlbase.h>
#include <atlstr.h>
#include <atlwin.h>


#include <windows.h>
#include <winbase.h>
#include <Commdlg.h>
#include <limits>

#include <commctrl.h>

#include "../../Common/Common.h"

// Sometimes it is much easier to use strings, for which we have Tstring.
#include <string>
//ACL given unicode, this defines Tstring as equivalent to std::wstring...
typedef std::basic_string < TCHAR > Tstring;

#include "../../Common/NoClones.h"

#include "WinUTF8.h"

// Some global helpers. In all my applications there is just one instance,
// or one important one, and WinHelper::hInstApp saves passing it around endlessly
namespace WinHelper {
  extern HINSTANCE hInstApp;
  void LastWindowsError();
  void InitCommonControlLib();
  void GetUserDirectory(Tstring * Output);      // Both thof these will always
  void GetAppDirectory(Tstring * Output);       // return a trailing "\"
}

#include "WinLocalisation.h"

#if _MSC_VER <= 1300
namespace stdext = std;
#endif

#endif /* #ifndef __WinCommon_h__ */
