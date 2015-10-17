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

#include "DasherWindow.h"


using namespace Dasher;
class CDasherApp : public CAtlExeModuleT< CDasherApp >
{
  CDasherWindow *m_pDasherWindow;
public:
  /*
  Entry point to program on Windows systems

  An interface to the Dasher library is created.
  A GUI and settings manager are created and given to the Dasher interface.
  Control is passed to the main GUI loop, and only returns when the main window closes.
  */
  HRESULT Run(int nShowCmd){
    m_pDasherWindow = new CDasherWindow;

    m_pDasherWindow->Create();
    m_pDasherWindow->Show(nShowCmd);
    m_pDasherWindow->UpdateWindow();
    HWINEVENTHOOK hHook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS, NULL, HandleWinEvent,
      0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    int iRet = CAtlExeModuleT< CDasherApp >::Run(nShowCmd);

    if (hHook)
      UnhookWinEvent(hHook);

    delete m_pDasherWindow;
    m_pDasherWindow = NULL;

    return iRet;
  };

  static VOID CALLBACK HandleWinEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

} DasherApp;


VOID CALLBACK CDasherApp::HandleWinEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
  DasherApp.m_pDasherWindow->HandleWinEvent(hWinEventHook, event, hwnd, idObject, idChild, dwEventThread, dwmsEventTime);
}


extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int nShowCmd)
{
  WinHelper::hInstApp = hInstance;
  return DasherApp.WinMain(nShowCmd);
}
