#pragma once
#include "../SocketInputBase.h"

namespace Dasher {
  class CSocketInput;
}

class Dasher::CSocketInput:public CSocketInputBase {

  friend unsigned __stdcall ThreadLauncherStub(void *_myClass) {
    CSocketInput *myClass = (CSocketInput *) _myClass;
    myClass->ReadForever();
    return 0; // prevents compiler error
  }

public:

  CSocketInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore);
  ~CSocketInput();
  
private:

  HWND m_hwnd;
  HANDLE m_readerThreadHandle;

  bool LaunchReaderThread();

  void CancelReaderThread();

  void ReportErrnoError(std::string prefix);

  //void ReportError(std::string s);

};
