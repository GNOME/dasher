// Trace.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "Trace.h"
#include "Platform.h"

// Customize behaviour of Trace here

#ifdef DASHER_WIN32
	
	// On Windows, send Trace to the Debug window in DevStudio
	// The ATL/MFC Trace application also picks up Trace when running

	#include "Windows.h"

	void DasherTraceOutputImpl( const char* pszFormat, va_list vargs )
	{
		char buffer[2048];
		vsprintf(buffer,pszFormat,vargs);
		OutputDebugStringA( buffer );
	}

#else

	// Send Trace to stdout

	void DasherTraceOutputImpl( const char* pszFormat, va_list vargs )
	{
		vfprintf(stdout,pszFormat,vargs);
	}

#endif
