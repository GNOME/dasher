// assert.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __assert_h__
#define __assert_h__

#include <cassert>

#ifdef _MSC_VER

	// The _ASSERT macro causes execution to break into the debugger in _DEBUG mode
	// In no-debug debug builds - no check is done
	#include 	<crtdbg.h>
	#define ASSERT(expr) _ASSERT(expr)

#else

	// Please feel free to implement this differently on your platform
	#define ASSERT(expr) assert(expr)

#endif


// DJW - useful 'compile time' assertion
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

#define STATIC_CHECK(expr, msg) \
{ CompileTimeError<((expr) != 0)> ERROR_##msg; (void)ERROR_##msg; } 


#endif // __include__