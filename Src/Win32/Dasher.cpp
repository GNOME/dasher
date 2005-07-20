
#include "WinCommon.h"          // must include pch first

#include ".\dasher.h"
#include "EditWrapper.h"

using namespace std;
using namespace Dasher;

// Used to signal our message loop to do our periodic work, the value
// shouldn't collide with anything else in our code.
#define WM_DASHER_TIMER WM_USER + 128

//void AddFiles(LPCWSTR Alphabets, LPCWSTR Colours, CDasherInterface *Interface)
void CDasher::AddFiles(Tstring Alphabets, Tstring Colours, CDasherInterface *Interface) {
  using namespace WinHelper;
  using namespace WinUTF8;

  std::string filename;
  WIN32_FIND_DATA find;
  HANDLE handle;

  handle = FindFirstFile(Alphabets.c_str(), &find);
  if(handle != INVALID_HANDLE_VALUE) {
    wstring_to_UTF8string(wstring(find.cFileName), filename);
    Interface->AddAlphabetFilename(filename);
    while(FindNextFile(handle, &find) != false) {
      wstring_to_UTF8string(wstring(find.cFileName), filename);
      Interface->AddAlphabetFilename(filename);
    }
    FindClose(handle);
  }

  handle = FindFirstFile(Colours.c_str(), &find);
  if(handle != INVALID_HANDLE_VALUE) {
    wstring_to_UTF8string(find.cFileName, filename);
    Interface->AddColourFilename(filename);
    while(FindNextFile(handle, &find) != false) {
      wstring_to_UTF8string(find.cFileName, filename);
      Interface->AddColourFilename(filename);
    }
    FindClose(handle);

  }
}

CDasher::CDasher(HWND Parent):m_hParent(Parent) {
  // This class will be a wrapper for the Dasher 'control' - think ActiveX

  m_bWorkerShutdown = false;

  using namespace WinHelper;
  using namespace WinUTF8;

  // Get folder names for system and user data.
  Tstring UserData, AppData;
  std::string UserData2, AppData2;
  Tstring Alphabets, Colours;
  GetUserDirectory(&UserData);
  GetAppDirectory(&AppData);
  UserData += TEXT("dasher.rc\\");
  AppData += TEXT("system.rc\\");
  CreateDirectory(UserData.c_str(), NULL);      // Try and create folders. Doesn't seem
  CreateDirectory(AppData.c_str(), NULL);       // to do any harm if they already exist.
  wstring_to_UTF8string(UserData, UserData2);   // TODO: I don't know if special characters will work.
  wstring_to_UTF8string(AppData, AppData2);     // ASCII-only filenames are safest. Being English doesn't help debug this...

  // Set up COM for the accessibility stuff
  CoInitialize(NULL);

  // Set up the registry
//      CWinOptions WinOptions("Inference_Group", "Dasher3"); // Settings storage using Windows Registry.

  // Set up Dasher
  m_pInterface = new CDasherInterface;
  m_pInterface->SetSystemLocation(AppData2);
  m_pInterface->SetUserLocation(UserData2);

  Alphabets = UserData;
  Alphabets += TEXT("alphabet*.xml");
  Colours = UserData;
  Colours += TEXT("colour*.xml");
  AddFiles(Alphabets, Colours, m_pInterface);
  Alphabets = AppData;
  Alphabets += TEXT("alphabet*.xml");
  Colours = AppData;
  Colours += TEXT("colour*.xml");
  AddFiles(Alphabets, Colours, m_pInterface);

  m_pInterface->ColourMode(true);
  m_pInterface->ChangeLanguageModel(0);

// FIXME - we should create our own edit object (as a wrapper to pass stuff outside), rather than relying on being passed one

  m_pEditWrapper = new CEditWrapper;

  m_pInterface->ChangeEdit(m_pEditWrapper);

  m_pCanvas = new CCanvas(m_hParent, m_pInterface);
  m_pSlidebar = new CSlidebar(m_hParent, m_pInterface, 1.99, false, m_pCanvas);

  // Start up our thread that will periodically handle user movement.  We pass in a pointer to ourselves
  // since the thread function must be static but needs to act on the object that created it.
  DWORD dwThreadId = 0;
  m_workerThread = CreateThread(NULL,   // default security attributes 
                                0,      // use default stack size  
                                CDasher::WorkerThread,  // thread function 
                                this,   // argument to thread function 
                                0,      // use default creation flags 
                                &dwThreadId);   // returns the thread identifier 

}

CDasher::~CDasher(void) {
  ShutdownWorkerThread();

  delete m_pCanvas;
  delete m_pSlidebar;
  delete m_pEditWrapper;
  delete m_pInterface;
}

// Handle periodically poking the canvas to check for user activity.  
// This use to be done with a SetTimer() call, but this horked up 
// the Visual Studio debugger.  Now we send a user specified message
// to the pump and use this to drive the updates.
DWORD CDasher::WorkerThread(LPVOID lpParam) {
  CDasher *parent = (CDasher *) lpParam;

  if(parent == NULL) {
    return -1;
  }

  while(!parent->m_bWorkerShutdown) {
    ::Sleep(20);
    SendMessage(parent->GetCanvas()->getwindow(), WM_DASHER_TIMER, NULL, NULL); // FIXME
  }

  return 0;
}

// Called when we want to get the worker thread to stop.
void CDasher::ShutdownWorkerThread() {
  const int CHECK_EVERY_MS = 100;       // Time between successive attempts to gracefully shutdown
  const int MAX_BEFORE_HARD_KILL = 2000;        // Maximum time to try for a gracefull thread shutdown

  m_bWorkerShutdown = true;

  if(m_workerThread != NULL) {
    // Give the thread some time to shut itself down gracefully
    int elapsed = 0;
    DWORD dwResult = WAIT_TIMEOUT;

    while((dwResult == WAIT_TIMEOUT) && (elapsed < MAX_BEFORE_HARD_KILL)) {
      dwResult = WaitForSingleObject(m_workerThread, 100);

      if(dwResult == WAIT_TIMEOUT) {
        elapsed += CHECK_EVERY_MS;
        ::Sleep(CHECK_EVERY_MS);
      }
    }

    // If all else fails, we'll hard kill the thread
    if(dwResult == WAIT_TIMEOUT)
      TerminateThread(m_workerThread, 0);

    CloseHandle(m_workerThread);
    m_workerThread = NULL;
  }
}

// Handles the work we need to do periodically on a timer event
void CDasher::OnTimer() {
  // HWND testwindow = NULL;

  // Ugh. Can't find a desperately nicer way of doing this, though
  //  testwindow = GetForegroundWindow();
  //  if (testwindow != m_hwnd) 
  //  {
  //      if (m_pEdit != NULL) // FIXME
  //          m_pEdit->SetWindow(testwindow);
  //  }

  // if (m_pCanvas != NULL) // FIXME
  //     m_pCanvas->OnTimer();
}
