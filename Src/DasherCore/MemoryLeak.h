
#ifdef _WIN32

// Include the following code at the top of a cpp file to enable
// line level memory leak detection on Windows:
//
//      #ifdef _WIN32
//      #include "MemoryLeak.h"
//      #ifdef _DEBUG
//      #define new DEBUG_NEW
//      #undef THIS_FILE
//      static char THIS_FILE[] = __FILE__;
//      #endif
//      #endif

#pragma once

#include <windows.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#endif

void EnableLeakDetection( void );
void SetOutputLeaksToFile();

#endif
