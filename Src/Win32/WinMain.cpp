// WinMain.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

//#ifdef _DEBUG
// #include "vld.h"
//#endif 

#include "Common/WinHelper.h"

#include "DasherWindow.h"

#include "../DasherCore/Win32/DasherInterface.h"
#include "Dasher.h"

#ifdef _WIN32
// In order to track leaks to line number, we need this at the top of every file
#include "../DasherCore/MemoryLeak.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

using namespace Dasher;
using namespace std;

// DJW 20031029 - tip - don't use LPCWSTR explicitely
// instead TCHAR (or our Tstring) is your friend - it type-defs to char or wchar depending whether or not you have UNICODE defined

/*
Entry point to program on Windows systems

An interface to the Dasher library is created.
A GUI and settings manager are created and given to the Dasher interface.
Control is passed to the main GUI loop, and only returns when the main window closes.
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

  // String literals in this function are not in the resource file as they
  // must NOT be translated.

  WinHelper::hInstApp = hInstance;      // DJW - put this back in as this global is needed in various placed

  // We don't want to starve other interactive applications
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

  int iRet = 0;

  { // memory leak scoping

    CDasherWindow DasherWindow;

    //The UI will be updated to reflect settings

    DasherWindow.Show(nCmdShow);
    iRet = DasherWindow.MessageLoop();

    // Close the COM library on the current thread
    CoUninitialize();

  } // end memory leak scoping

  return iRet;
}
