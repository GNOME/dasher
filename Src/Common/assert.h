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

/////////////////////////////////////////////////////////////////////////////
// DASHER_ASSERT macro defined below
/////////////////////////////////////////////////////////////////////////////

#ifdef DASHER_WIN32

	// The DASHER_ASSERT macro causes execution to break into the debugger in DEBUG mode
	// In non-debug debug builds - no check is done

	#ifdef DASHER_WINCE
		#ifdef DEBUG
			void __cdecl __debugbreak(void);
			#define DASHER_ASSERT(x)   if (!(x)) __debugbreak()
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

/////////////////////////////////////////////////////////////////////////////

// DJW - useful 'compile time' assertion
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

#define STATIC_CHECK(expr, msg) \
{ CompileTimeError<((expr) != 0)> ERROR_##msg; (void)ERROR_##msg; };

/////////////////////////////////////////////////////////////////////////////

// Pointer checking - some CRTs provide functionality to check the integrity of memory

// DASHER_ASSERT_VALIDPTR_RW(p) asserts that a pointer is valid for read and write
// DASHER_ASSERT_VALIDPTR_R(p) asserts that a pointer is valid for read

#if DASHER_WIN32 >= 1300
	#define DASHER_ASSERT_VALIDPTR_RW(p)	DASHER_ASSERT(_CrtIsValidPointer(p, sizeof(p), 1))
	#define DASHER_ASSERT_VALIDPTR_R(p)		DASHER_ASSERT(_CrtIsValidPointer(p, sizeof(p), 0))
#else

	// Please implement any platform-specific pointer checking

	// Simple check that the pointer is non-null
	#define DASHER_ASSERT_VALIDPTR_RW(p)	DASHER_ASSERT(p!=NULL)
	#define DASHER_ASSERT_VALIDPTR_R(p)		DASHER_ASSERT(p!=NULL)

#endif


#endif // __include__
