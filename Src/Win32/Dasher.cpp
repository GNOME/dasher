
#include "Common\WinCommon.h"          // must include pch first

#include "../DasherCore/ModuleManager.h"

#include "Dasher.h"
#include "../DasherCore/Event.h"
#include "Common\WinUTF8.h"
#include "Widgets/Canvas.h"
#include "DasherMouseInput.h"
#include "DasherWindow.h"
#include "Widgets/Edit.h"

#ifndef _WIN32_WCE
#include "Sockets/SocketInput.h"
#include "BTSocketInput.h"
#endif

#include "Common/WinOptions.h"

//ACL not sure what headers we need to include to get clipboard operations, but may need:
//#include <afxpriv.h>

#ifndef _WIN32_WCE
#include <sys/stat.h>
#endif

using namespace std;
using namespace Dasher;
using namespace WinUTF8;

// Used to signal our message loop to do our periodic work, the value
// shouldn't collide with anything else in our code.
#define WM_DASHER_TIMER WM_USER + 128

CONST UINT WM_DASHER_FOCUS = RegisterWindowMessage(_WM_DASHER_FOCUS);

CDasher::CDasher(HWND Parent, CDasherWindow *pWindow, CEdit *pEdit)
 : CDashIntfScreenMsgs(new CWinOptions( "Inference Group", "Dasher3")), m_hParent(Parent), m_pWindow(pWindow), m_pEdit(pEdit) {
  // This class will be a wrapper for the Dasher 'control' - think ActiveX

#ifndef _WIN32_WCE
  // Set up COM for the accessibility stuff
  CoInitialize(NULL);
#endif

  DWORD dwTicks = GetTickCount();
  Realize(dwTicks);
}

CDasher::~CDasher(void) {
//  WriteTrainFileFull();
  delete m_pCanvas;
}

void CDasher::CreateModules() {
  //create default set first.
  CDasherInterfaceBase::CreateModules();
#ifndef _WIN32_WCE
  RegisterModule(new CSocketInput(this,this));
  RegisterModule(new CBTSocketInput());
#endif
  RegisterModule(new CDasherMouseInput(m_pCanvas->getwindow()));
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

void Dasher::CDasher::HandleEvent(int iParameter) {
  CDashIntfScreenMsgs::HandleEvent(iParameter);
  m_pWindow->HandleParameterChange(iParameter);
  m_pEdit->HandleParameterChange(iParameter);
  if (iParameter == SP_DASHER_FONT)
    m_pCanvas->SetFont(GetStringParameter(SP_DASHER_FONT));
}

void Dasher::CDasher::editOutput(const string &strText, CDasherNode *pSource) {
  if(m_pWindow->m_pGameModeHelper)
    m_pWindow->m_pGameModeHelper->Output(strText);
  m_pEdit->output(strText);
  CDasherInterfaceBase::editOutput(strText, pSource);
}

void Dasher::CDasher::editDelete(const string &strText, CDasherNode *pSource) {
  if (m_pWindow->m_pGameModeHelper)
    m_pWindow->m_pGameModeHelper->Delete(strText.size());
  m_pEdit->deletetext(strText);
  CDasherInterfaceBase::editDelete(strText, pSource);
}

unsigned int Dasher::CDasher::ctrlMove(bool bForwards, CControlManager::EditDistance iDist) {
  return m_pEdit->Move(bForwards, iDist);
}

unsigned int Dasher::CDasher::ctrlDelete(bool bForwards, CControlManager::EditDistance iDist) {
  return m_pEdit->Delete(bForwards, iDist);
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

void Dasher::CDasher::WriteTrainFile(const std::string &filename, const std::string &strNewText) {
  const std::string TrainFile = GetStringParameter(SP_USER_LOC) + filename;

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
  m_pCanvas = new CCanvas(this);
  m_pCanvas->Create(m_hParent); // TODO - check return 

  // TODO: See MessageLoop, Main in CDasherWindow - should be brought into this class
  // Framerate settings: currently 40fps.
  SetTimer(m_pCanvas->getwindow(), 1, 25, NULL);
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

// TODO: Check that syntax here is sensible
void CDasher::Move(int iX, int iY, int iWidth, int iHeight) {
  if(m_pCanvas)
    m_pCanvas->Move(iX, iY, iWidth, iHeight);
}

void CDasher::TakeFocus() {
  // TODO: Implement me
}
#ifdef WIN32_SPEECH
bool CDasher::SupportsSpeech() {
  if (!m_bAttemptedSpeech) {
    //try to create speech synthesizer lazily, saving resources if no speech req'd.
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
    
    if(hr!=S_OK)
      pVoice=0;
    else if (pVoice) {
      //ACL Do we need to check pVoice? copying old code again, previous comment said:
      // TODO: Why is this needed?
      pVoice->Speak(L"",SPF_ASYNC,NULL);
    }
    m_bAttemptedSpeech = true;
  }
  return pVoice;
}

void CDasher::Speak(const string &strText, bool bInterrupt) {
  //ACL TODO - take account of bInterrupt
  if (pVoice)
    pVoice->Speak(strText.c_str(), SPF_ASYNC, NULL);
}
#endif

void CDasher::CopyToClipboard(const string &strText) {
  if (OpenClipboard(m_hParent))
  {
    EmptyClipboard(); //also frees memory containing any previous data
    
    //Allocate global memory for string - enough for characters + NULL.
    HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, strText.length()+1);
    
    //GlobalLock returns a pointer to the data associated with the handle returned from GlobalAlloc    
    char * pchData = (char*)GlobalLock(hClipboardData);

    //now fill it...
	strcpy(pchData, strText.c_str());
    
    //Unlock memory, i.e. release our access to it - 
    // but don't free it (with GlobalFree), as it will "belong"
    // to the clipboard.
    GlobalUnlock(hClipboardData);
    
    //Now, point the clipboard at that global memory...
    //ACL may have to use CF_TEXT or CF_OEMTEXT prior to WinNT/2K???
    SetClipboardData(CF_UNICODETEXT,hClipboardData);
    
    //Finally, unlock the clipboard (i.e. a pointer to the data on it!)
    // so that other applications can see / modify it
    CloseClipboard();
  }
}

std::string CDasher::GetAllContext() {
	int speechlength = m_pEdit->GetWindowTextLength();
	LPTSTR allspeech = new TCHAR[speechlength + 1];
	m_pEdit->GetWindowText(allspeech, speechlength + 1);
	string res;
	wstring_to_UTF8string(wstring(allspeech),res);
	return res;
}

std::string CDasher::GetContext(unsigned int iStart, unsigned int iLength) {
  return m_pEdit->get_context(iStart, iLength);
}
