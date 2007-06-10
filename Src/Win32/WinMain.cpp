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

#ifndef _WIN32_WCE
// Windows Event handler
VOID CALLBACK WEProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
#endif

/*
Entry point to program on Windows systems

An interface to the Dasher library is created.
A GUI and settings manager are created and given to the Dasher interface.
Control is passed to the main GUI loop, and only returns when the main window closes.
*/
static CDasherWindow *g_pDasherWindow = NULL;

#ifndef _WIN32_WCE
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
#endif

#ifndef _WIN32_WCE
  CoInitialize(NULL);
#endif

  WinHelper::hInstApp = hInstance;

  //SHInitExtraControls();

  // We don't want to starve other interactive applications
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

#ifndef _WIN32_WCE
  HWINEVENTHOOK hHook;

  hHook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS, NULL, WEProc,
                          0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
#endif

  int iRet = 0;

  g_pDasherWindow = new CDasherWindow;

 	g_pDasherWindow->Create();
  g_pDasherWindow->Show(nCmdShow);
  g_pDasherWindow->UpdateWindow();

  iRet = g_pDasherWindow->MessageLoop();

  delete g_pDasherWindow;
  g_pDasherWindow = NULL;
  
#ifndef _WIN32_WCE
  if(hHook)
    UnhookWinEvent(hHook);
#endif

#ifndef _WIN32_WCE
  // Close the COM library on the current thread
  CoUninitialize();
#endif

  return iRet;
}

#ifndef _WIN32_WCE
VOID CALLBACK WEProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
  if(g_pDasherWindow)
    g_pDasherWindow->HandleWinEvent(hWinEventHook, event, hwnd, idObject, idChild, dwEventThread, dwmsEventTime);
}
#endif