// int64.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __int64_h__
#define __int64_h__

#include "int.h"

//#ifdef _MSC_VER

// Get rid of annoying namespace pollution
#undef max
#undef min

#include <limits>
//#endif

#include "../myassert.h"

typedef int64 Cint64;

#endif // __include__
