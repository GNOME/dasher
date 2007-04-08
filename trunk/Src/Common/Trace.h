// Trace.h
//
// Copyright (c) 2005 David Ward

#ifndef __Common_Trace_h__
#define __Common_Trace_h__

// Trace is a mechanism for printf-like debugging that can be switched on/off
// at compile time

// To use Trace, define DASHER_TRACE in your build files, or uncomment the 
// following line
// #define DASHER_TRACE

// Use the DASHER_TRACEOUTPUT macro to format a message to trace
// Syntax is identicaly to printf:
//
// int i=6;
// DASHER_TRACEOUTPUT("Hello World %d", i); 

// The behaviour of DASHER_TRACEOUTPUT can be customized by changing
// DasherTraceOutputImpl in Trace.cpp

// Note that if DASHER_TRACE is not defined, trace code should be completely
// removed by the compiler

#include <stdarg.h>
#include <stdio.h>

void DasherTraceOutput(const char *pszFormat, ...);
void DasherTraceOutputImpl(const char *pszFormat, va_list vargs);

inline void DasherTraceOutput(const char *pszFormat, ...) {
  va_list v;
  va_start(v, pszFormat);
  DasherTraceOutputImpl(pszFormat, v);
  va_end(v);
}

// Define main Trace macro

#ifdef DASHER_TRACE

        // Active
#define DASHER_TRACEOUTPUT \
        DasherTraceOutput
#else

        // Inactive - function should never get called
#define DASHER_TRACEOUTPUT 1 ? (void) 0 : DasherTraceOutput

#endif // DASHER_TRACE

#endif
