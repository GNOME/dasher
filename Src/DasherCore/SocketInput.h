// SocketInput.h
//
// (C) Copyright Seb Wills 2005
//
// Linux-specific subclass of SocketInputBase: provides network socket control of Dasher cursor

#ifndef __SocketInput_h__
#define __SocketInput_h__

#include "./SocketInputBase.h"
#include "./Observable.h"

#include <iostream>
#include <pthread.h>

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)



namespace Dasher {
  class CSocketInput;
#if GCC_VERSION >= 40100
  void *ThreadLauncherStub(void *_myClass);
#endif

using namespace std;
/// \ingroup Input
/// \{
class CSocketInput:public CSocketInputBase {

  // This non-member launcher stub function is required because pthreads can't launch a non-static member method.
  friend void *ThreadLauncherStub(void *_myClass) {
    CSocketInput *myClass = (CSocketInput *) _myClass;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);     // kill this thread immediately if another thread cancels it
    // don't know how this interacts with recv blocking

    myClass->ReadForever();

    return NULL;
  }

public:

  CSocketInput(CSettingsUser *pCreator, CMessageDisplay *pMsgs);
  ~CSocketInput();

private:

  pthread_t readerThread;

  bool LaunchReaderThread();

  void CancelReaderThread();

  // TODO: should probably override ReportErrnoError() to popup a Gtk error message

};
}
/// \}
#endif
