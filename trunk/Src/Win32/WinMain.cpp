// WinMain.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

// Visual leak detector
#ifdef _DEBUG
 //#include "vld/vld.h"
#endif 

#include "Common/WinHelper.h"
#include "DasherWindow.h"

using namespace Dasher;
using namespace std;

/*
Entry point to program on Windows systems

An interface to the Dasher library is created.
A GUI and settings manager are created and given to the Dasher interface.
Control is passed to the main GUI loop, and only returns when the main window closes.
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {


CoInitialize(NULL);

  // String literals in this function are not in the resource file as they
  // must NOT be translated.

  WinHelper::hInstApp = hInstance;      // DJW - put this back in as this global is needed in various places

  // We don't want to starve other interactive applications
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

  int iRet = 0;

  { // memory leak scoping

    CDasherWindow DasherWindow;

	DasherWindow.Create();

  
    DasherWindow.Show(nCmdShow);
    DasherWindow.UpdateWindow();

	iRet = DasherWindow.MessageLoop();

    // Close the COM library on the current thread
    CoUninitialize();

  } // end memory leak scoping

  return iRet;
}
