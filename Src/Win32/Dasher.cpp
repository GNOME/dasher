
#include "WinCommon.h"          // must include pch first

#include ".\dasher.h"
#include "EditWrapper.h"

using namespace std;
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

  // Set up Dasher
  SetStringParameter(SP_SYSTEM_LOC, AppData2);
  SetStringParameter(SP_USER_LOC, UserData2);

  Alphabets = UserData;
  Alphabets += TEXT("alphabet*.xml");
  Colours = UserData;
  Colours += TEXT("colour*.xml");
  AddFiles(Alphabets, Colours, this);
  Alphabets = AppData;
  Alphabets += TEXT("alphabet*.xml");
  Colours = AppData;
  Colours += TEXT("colour*.xml");
  AddFiles(Alphabets, Colours, this);

  SetBoolParameter(BP_COLOUR_MODE, true);
  ChangeLanguageModel(0);

// FIXME - we should create our own edit object (as a wrapper to pass stuff outside), rather than relying on being passed one

  m_pEditWrapper = new CEditWrapper;

  ChangeEdit(m_pEditWrapper);

  m_pCanvas = new CCanvas(m_hParent, this);
  m_pSlidebar = new CSlidebar(m_hParent, this, ((double)GetLongParameter(LP_MAX_BITRATE))/100.0, m_pCanvas);

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

    // Do any periodic work that this object handles
    parent->OnTimer();
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


  CUserLog* pUserLog = GetUserLogPtr();

	// We'll use this timer event to periodically log the user's mouse position
	if ((pUserLog != NULL) && (m_pCanvas != NULL)) {
        // Get the mouse x and y coordinates
        POINT sMousePos;
        GetCursorPos(&sMousePos);
        
        // Since the everything is in screen relative coordinates, we'll
        // make sure we have the right coordinates for our canvas and
        // screen since the user may have move the window around.
        int     iTop     = 0;
        int     iLeft    = 0;
        int     iBottom  = 0;
        int     iRight   = 0;

        if (m_pCanvas->GetCanvasSize(&iTop, &iLeft, &iBottom, &iRight))
          pUserLog->AddCanvasSize(iTop, iLeft, iBottom, iRight);

        // Also update the size of the window in the UserLogTrial object
        if (GetWindowSize(&iTop, &iLeft, &iBottom, &iRight))
          pUserLog->AddWindowSize(iTop, iLeft, iBottom, iRight);

        // We'll store a normalized version so if the user changes the window
        // size during a trial, it won't effect our coordinates.  The 
        // normalization is with respect to the canvas and not the main 
        // window.        
        pUserLog->AddMouseLocationNormalized(sMousePos.x, 
                                             sMousePos.y, 
                                             true,
                                             (float) GetNats());
    }

}

void Dasher::CDasher::ExternalEventHandler(CEvent* pEvent) {
  // Here we send SendMessage calls to the DasherWindow class
  if( pEvent->m_iEventType == 1 ) {
    Dasher::CParameterNotificationEvent * pEvt(static_cast < Dasher::CParameterNotificationEvent * >(pEvent));
    if( pEvt->m_iParameter == BP_DASHER_PAUSED)
      if( GetBoolParameter(BP_DASHER_PAUSED) )
        SendMessage(m_hParent, WM_COMMAND, ID_EDIT_COPY_ALL, 0);
  }
  else if((pEvent->m_iEventType >= 2) && (pEvent->m_iEventType <= 5)) {
    if(m_DashEditbox != NULL)
      m_DashEditbox->HandleEvent(pEvent);
  }
}

// Get the pointer to our user logging object
CUserLog* Dasher::CDasher::GetUserLogPtr()
{
  return m_pUserLog;
}

// Gets the size of the window in screen coordinates.  
bool Dasher::CDasher::GetWindowSize(int* pTop, int* pLeft, int* pBottom, int* pRight)
{
	if ((pTop == NULL) || (pLeft == NULL) || (pBottom == NULL) || (pRight == NULL))
		return false;

	RECT sWindowRect;
	if (GetWindowRect(m_hParent, &sWindowRect))
  {
    *pTop    = sWindowRect.top;
    *pLeft   = sWindowRect.left;
    *pBottom = sWindowRect.bottom;
    *pRight  = sWindowRect.right;
    return true;
  }
  else
    return false;
}