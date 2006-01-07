// Common.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WinCommon_h__
#define __WinCommon_h__

// This file is designed to be used as a precompiled header, and if set up correctly
// you will only see this message once per project
#pragma message("Compiling WinCommon.h.........................")

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN     // Excludes rarely-used stuff from the Windows headers
#include <windows.h>
#include <winbase.h>
#include <Commdlg.h>

// Use common controls
#include <commctrl.h>

#include "../../Common/Common.h"

// windows.h provides the "generic" character type TCHAR.
#include "tchar.h"              // this adds some useful macros

// Sometimes it is much easier to use strings, for which we have Tstring.
#include <string>
typedef std::basic_string < TCHAR > Tstring;

#include "../../Common/NoClones.h"

#include "WinHelper.h"
#include "WinLocalisation.h"

#if DASHER_WIN32 <= 1300
namespace stdext = std;
#endif


#endif /* #ifndef __WinHelper_h__ */
