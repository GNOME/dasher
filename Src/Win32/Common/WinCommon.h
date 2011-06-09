// WinCommon.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WinCommon_h__
#define __WinCommon_h__

//#pragma message("Compiling WinCommon.h.........................")

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN     // Excludes rarely-used stuff from the Windows headers


#include <atlbase.h>
#include <atlwin.h>


#include <windows.h>
#include <winbase.h>
#include <Commdlg.h>
#include <limits>

// Use common controls
/*
  ACL: Used to have separate header file WinHelper.h in addition to this, which
  wrapped the #include  of commctrl.h with the following "#ifdef OriginalWin95"
  sections. I'm including them here in case they are needed, along with comment:

  Initialise Common Controls library. If we want this to work with Windows 95
  with no extra dll's or versions of IE installed, we define OriginalWin95 and
  play games with macros. Having said that, almost all Windows systems should
  have the required DLL for new common control features (either with the system
  or with IE). If not it is an easy upgrade from:
    http://www.microsoft.com/msdownload/ieplatform/ie/comctrlx86.asp

  IAM 08/2002
*/
#ifdef OriginalWin95
#define _WIN32_IE_TMP _WIN32_IE
#undef _WIN32_IE
#define _WIN32_IE 0x0100
#endif

#include <commctrl.h>
//ACL and also the following...
#ifdef OriginalWin95
#undef _WIN32_IE
#define _WIN32_IE _WIN32_IE_TMP
#undef _WIN32_IE_TMP
#endif

#ifdef OriginalWin95
#undef CP_UTF8                  // as we can't use it
#endif
//ACL end moved from WinHelper.h

#include "../../Common/Common.h"

// windows.h provides the "generic" character type TCHAR.
//ACL apparently TCHAR is "wchar_t" (=16bit) if unicode is enabled, else "char".
//We seem to use it interchangeably with wchar, so presumably will only compile
// with unicode!

//There is no "tchar.h", so commenting out:
// #include "tchar.h"              // this adds some useful macros
//...there _is_ a <tchar.h>, but if we don't need it...?

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

#if DASHER_WIN32 <= 1300
namespace stdext = std;
#endif

#endif /* #ifndef __WinCommon_h__ */
