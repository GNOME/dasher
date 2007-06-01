// WinMain.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

// Visual leak detector
// #ifdef _DEBUG
// #include "vld/vld.h"
// #endif 

#include "Common/WinHelper.h"
#include "DasherWindow.h"

#include <commctrl.h>

using namespace Dasher;

/*
Entry point to program on Windows systems

An interface to the Dasher library is created.
A GUI and settings manager are created and given to the Dasher interface.
Control is passed to the main GUI loop, and only returns when the main window closes.
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  CoInitialize(NULL);

  WinHelper::hInstApp = hInstance;

  // We don't want to starve other interactive applications
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

  int iRet = 0;

  { // memory leak scoping
    CDasherWindow DasherWindow;

  	DasherWindow.Create();
  
    DasherWindow.Show(nCmdShow);
    DasherWindow.UpdateWindow();

	  iRet = DasherWindow.MessageLoop();
  } // end memory leak scoping

  // Close the COM library on the current thread
  CoUninitialize();

  return iRet;
}
