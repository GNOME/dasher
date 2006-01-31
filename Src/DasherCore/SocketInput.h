// SocketInput.h
//
// (C) Copyright Seb Wills 2005
//
// Linux-specific subclass of SocketInputBase: provides network socket control of Dasher cursor

#ifndef __SocketInput_h__
#define __SocketInput_h__

#include "./SocketInputBase.h"
#include "./DasherComponent.h"
#include "./EventHandler.h"

#include <iostream>
#include <pthread.h>

namespace Dasher {
  class CSocketInput;
  void *ThreadLauncherStub(void *_myClass);
}

using namespace Dasher;
using namespace std;

class Dasher::CSocketInput:public CSocketInputBase {

  // This non-member launcher stub function is required because pthreads can't launch a non-static member method.
  friend void *Dasher::ThreadLauncherStub(void *_myClass) {
    CSocketInput *myClass = (CSocketInput *) _myClass;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);     // kill this thread immediately if another thread cancels it
    // don't know how this interacts with recv blocking

    myClass->ReadForever();

    return NULL;
  }

public:

  CSocketInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore);
  ~CSocketInput();

private:

  pthread_t readerThread;

  bool LaunchReaderThread();

  void CancelReaderThread();

  // TODO: should probably override ReportErrnoError() to popup a Gtk error message

};

#endif
