// assert.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __assert_h__
#define __assert_h__

#include "Platform.h"

#ifdef DASHER_WIN32

	// The _ASSERT macro causes execution to break into the debugger in DEBUG mode
	// In no-debug debug builds - no check is done

	#ifdef DASHER_WINCE
		#ifdef DEBUG
			#define DASHER_ASSERT(x)   if (!(x)) DebugBreak()
		#else
			#define DASHER_ASSERT(x)
		#endif 

	#else

		#include <crtdbg.h>
		#define DASHER_ASSERT(expr) _ASSERT(expr)
	
	#endif

#else

	#include <cassert>
	
	// Please feel free to implement this differently on your platform
	#define DASHER_ASSERT(expr) assert(expr)

#endif


// DJW - useful 'compile time' assertion
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

#define STATIC_CHECK(expr, msg) \
{ CompileTimeError<((expr) != 0)> ERROR_##msg; (void)ERROR_##msg; } 


#endif // __include__
