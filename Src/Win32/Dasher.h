#pragma once

#include "DasherInterface.h"
#include "../DasherCore/UserLog.h"

extern CONST UINT WM_DASHER_EVENT;
#define _WM_DASHER_EVENT (LPCWSTR)"wm_dasher_event"

extern CONST UINT WM_DASHER_FOCUS;
#define _WM_DASHER_FOCUS (LPCWSTR)"wm_dasher_focus"

namespace Dasher {
  class CDasher;
} 
class CCanvas;
class CSlidebar;
class CDashEditbox;

class Dasher::CDasher : public CDasherInterface 
{
public:
  CDasher(HWND Parent);
  ~CDasher(void);

  // The following functions will not be part of the final interface

  CCanvas *GetCanvas() {
	  return m_pCanvas;
  }

  void SetEdit(CDashEditbox * pEdit);

  void ExternalEventHandler(Dasher::CEvent *pEvent);

  CUserLog*     GetUserLogPtr();

  virtual void WriteTrainFile(const std::string &strNewText);

    void					  Main(); 


private:

  virtual void ScanAlphabetFiles(std::vector<std::string> &vFileList);
  virtual void ScanColourFiles(std::vector<std::string> &vFileList);
  virtual void SetupPaths();
  virtual void SetupUI();
  virtual int GetFileSize(const std::string &strFileName);
  void ScanDirectory(const Tstring &strMask, std::vector<std::string> &vFileList);
  bool                    GetWindowSize(int* pTop, int* pLeft, int* pBottom, int* pRight);
  void                    Log();                        // Does the logging

  CCanvas *m_pCanvas;
  CDashEditbox *m_pEdit;
 
// Retired  
  // static DWORD WINAPI     WorkerThread(LPVOID lpParam);     // Spins around and sends WM_DASHER_TIMER message
  // void                    ShutdownWorkerThread();           // Called when we want the worker thread to stop
  // HANDLE m_workerThread;        // Handle to our worker thread that periodically checks on user's activities
  // HANDLE m_EventWorkerThreadShutdown; // Event for thread-safe thread shutdown 

  HWND m_hParent;

};
