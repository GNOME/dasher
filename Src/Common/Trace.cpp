// Trace.cpp
//
// Copyright (c) 2005 David Ward

#include "Common.h"

#include "Trace.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// Customize behaviour of Trace here

#ifdef _WIN32

        // On Windows, send Trace to the Debug window in DevStudio
        // The ATL/MFC Trace application also picks up Trace when running

#include "Windows.h"

void DasherTraceOutputImpl(const char *pszFormat, va_list vargs) {
	// TODO: Reimplement
//  char buffer[2048];
//  _vsnprintf(buffer, 2048,pszFormat, vargs);
//  OutputDebugStringA(buffer);
}

#else

        // Send Trace to stdout

void DasherTraceOutputImpl(const char *pszFormat, va_list vargs) {
  vfprintf(stdout, pszFormat, vargs);
}

#endif
