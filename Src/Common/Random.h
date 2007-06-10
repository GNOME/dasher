// Random.h
//
// Copyright (c) 2005 David Ward

#ifndef __Random_h__
#define __Random_h__

#include "Platform.h"

// Provide platform-independent interfaces for random number generation

// int RandomInt()
// A wrapper for rand() - WinCE3 is missing this function

//#ifdef DASHER_WINCE
//
//inline int RandomInt() {
//  return Random();
//}

//#else

#include <stdlib.h>

inline int RandomInt() {
  return rand();
}

//#endif

#endif
