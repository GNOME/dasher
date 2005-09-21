// SocketInput.cpp
// (C) Copyright Seb Wills 2005
//
// Win32-specific socket input class

#include "WinCommon.h"

#include "./SocketInput.h"
#include "../../Win32/resource.h" // Probably shouldn't be including a file outside DasherCore. This is needed purely for the resource strings (IDS_...) used in the error message titles

#include <process.h>
#include <atlstr.h>
#include <stdlib.h>
#include <stddef.h>

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CSocketInput::CSocketInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore)
:CSocketInputBase(pEventHandler, pSettingsStore) {

  // Windows-specific initialisation of Winsock:
  WORD wVersionRequested;
  WSADATA wsaData;  
  int err;
  wVersionRequested = MAKEWORD( 2, 2 );
  err = WSAStartup( wVersionRequested, &wsaData );
  if ( err != 0 ) {
    Tstring ErrTitle;
    WinLocalisation::GetResourceString(IDS_ERR_SOCKET_TITLE, &ErrTitle);
    MessageBox(NULL, _TEXT("Failed to initialize Winsock DLL"),  ErrTitle.c_str(), MB_OK | MB_ICONEXCLAMATION);
  }
}

CSocketInput::~CSocketInput() {
  StopListening();
}


bool CSocketInput::LaunchReaderThread() {

  unsigned threadId;
  m_readerThreadHandle = (HANDLE) _beginthreadex(NULL, // security attributes
                 0,    // stack size (0 = use default)
                 ThreadLauncherStub, // function to invoke
                 this, // argument
                 0,    // initflag
                 &threadId); // receives thread ID
  if(m_readerThreadHandle == 0) {
    ReportErrnoError("Error creating reader thread");
    return false;
  }
  return true;
}

void CSocketInput::CancelReaderThread() {
  // FIXME: this is ugly; should have a way of cleanly terminating the thread
  // The thread will need to use select to check the socket input buffer
  // (rather than just calling recv which blocks indefinitely) and monitor a pleaseStop
  // variable or something. But this seems to work OK...
  TerminateThread(m_readerThreadHandle, 1);
}

void CSocketInput::ReportErrnoError(std::string prefix) {
  CString msg(prefix.c_str()); //FIXME: check what goes on here with encodings. Is prefix in UTF-8? Do we nee dto call mbstowcs or something on it?
  //wchar_t *tmp = new wchar_t[mbstowcs(NULL, (const char *) prefix.c_str(), 1024)];

  msg += _TEXT(": Errno reports '");
  msg += _tcserror(errno); // retrieve localised error message for the most recent socket error
  CString msg2;
  msg2.Format(_TEXT("'. WSAGetLastError returns %d."), WSAGetLastError());
  msg += msg2;

  Tstring ErrTitle;
  WinLocalisation::GetResourceString(IDS_ERR_SOCKET_TITLE, &ErrTitle);
  MessageBox(NULL, msg, ErrTitle.c_str(), MB_OK | MB_ICONEXCLAMATION);
}
