// Platform.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Platform_h__
#define __Platform_h__

// Place to set platform-defines of the form DASHER_........

#ifdef _MSC_VER

	#ifdef WIN32
		#define DASHER_WIN32
	#endif

	#ifdef _WIN32_WCE
		#define DASHER_WINCE
	#endif

#else

// Please set platform-defines here

#endif

#endif // __include__
