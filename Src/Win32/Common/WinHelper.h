// WinHelper.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WinHelper_h__
#define __WinHelper_h__

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN     // Excludes rarely-used stuff from the Windows headers
#include <windows.h>
#include <winbase.h>
#include <Commdlg.h>

/*
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

#ifdef OriginalWin95
#undef _WIN32_IE
#define _WIN32_IE _WIN32_IE_TMP
#undef _WIN32_IE_TMP
#endif

#ifdef OriginalWin95
#undef CP_UTF8                  // as we can't use it
#endif

// windows.h provides the "generic" character type TCHAR.
#include "tchar.h"              // this adds some useful macros
// Sometimes it is much easier to use strings, for which we now have Tstring.
#include <string>
typedef std::basic_string < TCHAR > Tstring;

// Some global helpers. In all my applications there is just one instance,
// or one important one, and WinHelper::hInstApp saves passing it around endlessly
namespace WinHelper {
  extern HINSTANCE hInstApp;
  void LastWindowsError();
  void InitCommonControlLib();
  void GetUserDirectory(Tstring * Output);      // Both thof these will always
  void GetAppDirectory(Tstring * Output);       // return a trailing "\"
}
#endif                          /* #ifndef __WinHelper_h__ */
