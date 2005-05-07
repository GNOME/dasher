// Common.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __WinCommon_h__
#define __WinCommon_h__



#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Excludes rarely-used stuff from the Windows headers
#include <windows.h>
#include <winbase.h>
#include <Commdlg.h>

// Use common controls
#include <commctrl.h>


// windows.h provides the "generic" character type TCHAR.
#include "tchar.h" // this adds some useful macros

// Sometimes it is much easier to use strings, for which we have Tstring.
#include <string>
typedef std::basic_string<TCHAR> Tstring;

#include "../../Common/NoClones.h"

#include "WinUTF8.h"
#include "WinHelper.h"
#include "WinWrap.h"
#include "WinLocalisation.h"

#include "../../Common/Common.h"

#if DASHER_WIN32 <= 1300
	namespace stdext = std;
#endif


#endif /* #ifndef __WinHelper_h__ */
