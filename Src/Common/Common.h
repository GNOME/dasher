// Common.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Common_h__
#define __Common_h__

// Place common includes here - to be include by every .cpp file before its header 
//              - can be used for precompiled headers
//              - also when order of headers is important

/////////////////////////////////////////////////////////////////////////////

#if HAVE_CONFIG_H
#include <config.h>
#endif

/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "MSVC_Unannoy.h"

#if _MSC_VER < 1300
#include "Platform/stdminmax.h"
#endif

#endif

#ifndef HAVE_ROUND
#include "round.h"
#endif

#include "myassert.h"

#include "mydebug.h"

#include "Trace.h"

#include "I18n.h"

#endif // __include__
