// SocketInput.cpp
//
// (C) Copyright Seb Wills 2005
//
// Linux-specific subclass of SocketInputBase: provides network socket control of Dasher cursor

#include "../Common/Common.h"

#include "SocketInput.h"
#include "SocketInputBase.h"

#include <pthread.h>

using namespace Dasher;

CSocketInput::CSocketInput(CSettingsUser *pCreator, CMessageDisplay *pMsgs)
:CSocketInputBase(pCreator, pMsgs) {
}

CSocketInput::~CSocketInput() {
  StopListening();
}

// private methods:

bool CSocketInput::LaunchReaderThread() {
  if (pthread_create(&readerThread, NULL, ThreadLauncherStub, this) == 0) {
    return true;
  } else {
    //TODO should probably pop up a Gtk error message and think about how to do i18n:
    cerr << _("Dasher socket input: failed to launch reader thread.") << endl;
    return false;
  }
}

void CSocketInput::CancelReaderThread() {
  pthread_cancel(readerThread);
}
