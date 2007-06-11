// int.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __int_h__
#define __int_h__

#include "../Platform.h"

#ifdef DASHER_WIN32

typedef __int64 int64;
typedef unsigned __int64 uint64;
typedef int int32;
typedef unsigned int uint32;

#include <limits>

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807
#define LLONG_MIN (-LLONG_MAX - 1)
#endif

#else

typedef long long int int64;
typedef unsigned long long int uint64;
typedef int int32;
typedef unsigned int uint32;

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#define LLONG_MIN (-LLONG_MAX - 1LL)
#endif

#endif

const int64 int64_max = LLONG_MAX;
const int64 int64_min = LLONG_MIN;

template < class T > inline bool AreDifferentSigns(T lhs, T rhs) {
  return ((lhs ^ rhs) < 0);
};

#endif
