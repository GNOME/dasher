// int.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __int_h__
#define __int_h__

#ifdef _WIN32

typedef __int64 int64;
typedef unsigned __int64 uint64;
typedef int int32;
typedef unsigned int uint32;

#else

typedef long long int int64;
typedef unsigned long long int uint64;
typedef int int32;
typedef unsigned int uint32;

#endif

#include <limits>

const int64 int64_max = std::numeric_limits<int64>::max();
const int64 int64_min = std::numeric_limits<int64>::min();

#endif
