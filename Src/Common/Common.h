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
//		- can be used for precompiled headers
//		- also when order of headers is important

#include "Platform.h"

	#ifdef DASHER_WIN32
		#include "MSVC_Unannoy.h"
	#endif


#include "assert.h"

#include "Trace.h"

#endif // __include__
