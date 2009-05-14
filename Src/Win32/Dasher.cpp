
#include "Common\WinCommon.h"          // must include pch first

#include "../DasherCore/ModuleManager.h"

#include "Dasher.h"
#include "../DasherCore/Event.h"
#include "Common\WinUTF8.h"
#include "Widgets/Canvas.h"
#include "DasherMouseInput.h"

#ifndef _WIN32_WCE
#include "Sockets/SocketInput.h"
#include "BTSocketInput.h"
#endif

#include "Common/WinOptions.h"

#ifndef _WIN32_WCE
#include <sys/stat.h>
#endif

using namespace std;
using namespace Dasher;
using namespace WinUTF8;

// Used to signal our message loop to do our periodic work, the value
// shouldn't collide with anything else in our code.
#define WM_DASHER_TIMER WM_USER + 128

CONST UINT WM_DASHER_EVENT = RegisterWindowMessage(_WM_DASHER_EVENT);
CONST UINT WM_DASHER_FOCUS = RegisterWindowMessage(_WM_DASHER_FOCUS);
CONST UINT WM_DASHER_GAME_MESSAGE = RegisterWindowMessage(_WM_DASHER_GAME_MESSAGE);

CDasher::CDasher(HWND Parent):m_hParent(Parent) {
  // This class will be a wrapper for the Dasher 'control' - think ActiveX
  m_pEdit = 0;

#ifndef _WIN32_WCE
  // Set up COM for the accessibility stuff
  CoInitialize(NULL);
#endif

  Realize();
}

CDasher::~CDasher(void) {
//  WriteTrainFileFull();
  delete m_pCanvas;
}

void CDasher::CreateModules() {
  //create default set first.
  CDasherInterfaceBase::CreateModules();
#ifndef _WIN32_WCE
  RegisterModule(new CSocketInput(m_pEventHandler, m_pSettingsStore));
  RegisterModule(new CBTSocketInput(m_pEventHandler, m_pSettingsStore));
#endif
  RegisterModule(new CDasherMouseInput(m_pEventHandler, m_pSettingsStore, m_pCanvas->getwindow()));
}

void CDasher::Main() {
  if(m_pCanvas) {
		m_pCanvas->DoFrame();
  }

  DWORD dwTicks = GetTickCount();
	NewFrame(dwTicks, false);
}

// Handles the work we need to do periodically on a timer event
void CDasher::Log() {

  CUserLogBase* pUserLog = GetUserLogPtr();

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

        if (m_pCanvas->GetCanvasSize(iTop, iLeft, iBottom, iRight))
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
  SendMessage(m_hParent, WM_DASHER_EVENT, 0, (LPARAM)pEvent);
}

void Dasher::CDasher::GameMessageOut(int message, const void *messagedata)
{
  SendMessage(m_hParent, WM_DASHER_GAME_MESSAGE, (WPARAM)message, (LPARAM)messagedata);
}

// Gets the size of the window in screen coordinates.  
bool Dasher::CDasher::GetWindowSize(int* pTop, int* pLeft, int* pBottom, int* pRight) {
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

void Dasher::CDasher::SetEdit(CDashEditbox * pEdit) {
  // FIXME - we really need to make sure we have a
  // more sensible way of passing messages out here.

  m_pEdit = pEdit;
}

void Dasher::CDasher::WriteTrainFile(const std::string &strNewText) {
  const std::string TrainFile = GetStringParameter(SP_USER_LOC) + GetStringParameter(SP_TRAIN_FILE);

  if(strNewText.size() == 0)
    return;

  Tstring TTrainFile;
  UTF8string_to_wstring(TrainFile, TTrainFile);

  HANDLE hFile = CreateFile(TTrainFile.c_str(),
                            GENERIC_WRITE, 0, NULL, 
                            OPEN_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL, 0);

  if(hFile == INVALID_HANDLE_VALUE) {
    OutputDebugString(TEXT("Can not open file\n"));
  }
  else {
    DWORD NumberOfBytesWritten;
    SetFilePointer(hFile, 0, NULL, FILE_END);

  //// Surely there are better ways to write to files than this??

    for(unsigned int i = 0; i < strNewText.size(); i++) {
      WriteFile(hFile, &strNewText[i], 1, &NumberOfBytesWritten, NULL);
    }

    CloseHandle(hFile);
  }
}

void CDasher::ScanDirectory(const Tstring &strMask, std::vector<std::string> &vFileList) {
  using namespace WinUTF8;

  std::string filename;
  WIN32_FIND_DATA find;
  HANDLE handle;

  handle = FindFirstFile(strMask.c_str(), &find);
  if(handle != INVALID_HANDLE_VALUE) {
    wstring_to_UTF8string(wstring(find.cFileName), filename);
    vFileList.push_back(filename);
    while(FindNextFile(handle, &find) != false) {
      wstring_to_UTF8string(wstring(find.cFileName), filename);
      vFileList.push_back(filename);
    }
    FindClose(handle);
  }
}

void CDasher::ScanColourFiles(std::vector<std::string> &vFileList) {
  Tstring Colours;

  // TODO: Is it okay to have duplicate names in the array?
  std::string strAppData2(GetStringParameter(SP_SYSTEM_LOC));
  Tstring strAppData;

  WinUTF8::UTF8string_to_wstring(strAppData2, strAppData);
  
  Colours = strAppData;
  Colours += TEXT("colour*.xml");
  ScanDirectory(Colours, vFileList); 

  std::string strUserData2(GetStringParameter(SP_USER_LOC));
  Tstring strUserData;

  WinUTF8::UTF8string_to_wstring(strUserData2, strUserData);

  Colours = strUserData;
  Colours += TEXT("colour*.xml");
  ScanDirectory(Colours, vFileList); 
}

void CDasher::ScanAlphabetFiles(std::vector<std::string> &vFileList) {
  Tstring Alphabets;

  // TODO: Is it okay to have duplicate names in the array?
  std::string strAppData2(GetStringParameter(SP_SYSTEM_LOC));
  Tstring strAppData;

  WinUTF8::UTF8string_to_wstring(strAppData2, strAppData);
  
  Alphabets = strAppData;
  Alphabets += TEXT("alphabet*.xml");
  ScanDirectory(Alphabets, vFileList); 

  std::string strUserData2(GetStringParameter(SP_USER_LOC));
  Tstring strUserData;

  WinUTF8::UTF8string_to_wstring(strUserData2, strUserData);

  Alphabets = strUserData;
  Alphabets += TEXT("alphabet*.xml");
  ScanDirectory(Alphabets, vFileList); 
}

void CDasher::SetupPaths() {
  using namespace WinHelper;
  using namespace WinUTF8;

  Tstring UserData, AppData;
  std::string UserData2, AppData2;
  GetUserDirectory(&UserData);
  GetAppDirectory(&AppData);
  UserData += TEXT("dasher.rc\\");
  AppData += TEXT("system.rc\\");
  CreateDirectory(UserData.c_str(), NULL);      // Try and create folders. Doesn't seem
  CreateDirectory(AppData.c_str(), NULL);       // to do any harm if they already exist.
  wstring_to_UTF8string(UserData, UserData2);   // TODO: I don't know if special characters will work.
  wstring_to_UTF8string(AppData, AppData2);     // ASCII-only filenames are safest. Being English doesn't help debug this...
  SetStringParameter(SP_SYSTEM_LOC, AppData2);
  SetStringParameter(SP_USER_LOC, UserData2);
}

void CDasher::SetupUI() {
  m_pCanvas = new CCanvas(this, m_pEventHandler, m_pSettingsStore);
  m_pCanvas->Create(m_hParent); // TODO - check return 

  OnUIRealised();
}

int CDasher::GetFileSize(const std::string &strFileName) {
#ifndef _WIN32_WCE
  struct _stat sStatInfo;
  _stat(strFileName.c_str(), &sStatInfo);
  return sStatInfo.st_size;
#else
  // TODO: Fix this on Win CE
  return 0;
#endif
}

void CDasher::CreateSettingsStore(void) {
  m_pSettingsStore = new CWinOptions( "Inference Group", "Dasher3", m_pEventHandler );
}

void CDasher::StartTimer() {
  // TODO: See MessageLoop, Main in CDasherWindow - should be brought into this class
  // Framerate settings: currently 40fps.
  SetTimer(m_pCanvas->getwindow(), 1, 25, NULL);
}

void CDasher::ShutdownTimer() {
}

// TODO: Check that syntax here is sensible
void CDasher::Move(int iX, int iY, int iWidth, int iHeight) {
  if(m_pCanvas)
    m_pCanvas->Move(iX, iY, iWidth, iHeight);
}

void CDasher::TakeFocus() {
  // TODO: Implement me
}
