#include "WinHelper.h"

namespace WinHelper
{
	HINSTANCE hInstApp;
}

void WinHelper::LastWindowsError()
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION);
	LocalFree(lpMsgBuf);
}

namespace {
	bool CommonControlsInited=false;
}

void WinHelper::InitCommonControlLib()
{
	if (CommonControlsInited)
		return;
	
#ifdef OriginalWin95
	InitCommonControls();
#else
	INITCOMMONCONTROLSEX iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&iccex);
#endif
	
	CommonControlsInited = true;
}


#ifndef OriginalWin95
namespace {
// For SHGetSpecialFolderPath. Needs IE 4.0 or above (Win98+ always ok).
#include "shlobj.h"
}
#endif

void WinHelper::GetUserDirectory(Tstring* Output)
{
	Tstring& UserData = *Output;
	
#ifndef OriginalWin95
	TCHAR Buffer[MAX_PATH];
	// My documentation says SHGetSpecialFolderPath returns NOERROR if successful
	// With my headers NOERROR==0 and this function returns TRUE==1 as docs online say.
	// Not sure I trust the return value, so I'm just going to assume if I have a string
	// it is probably correct :)
	Buffer[0] = TEXT('\0');
	SHGetSpecialFolderPath(NULL, Buffer, CSIDL_APPDATA, 0);
	UserData = Buffer;
	if (UserData[UserData.size()-1]!=TEXT('\\'))
		UserData+=TEXT('\\');
#else
	// Not a lot else I can do :(
	GetAppDirectory(Output);
#endif
}


void WinHelper::GetAppDirectory(Tstring* Output)
{
	Tstring& AppLocation = *Output;
	
	TCHAR Buffer[MAX_PATH];
	if (0!=GetModuleFileName(NULL, Buffer, MAX_PATH)) {
		TCHAR* pos = _tcsrchr(Buffer, TEXT('\\')); // 3 line hack to remove filename
		pos++;                                     //
		*pos = TEXT('\0');                         //
		AppLocation = Buffer;
	} else
		AppLocation = TEXT(""); // Cop out. Current directory may be elsewhere.
	                            // Hopefully it won't come to this...
}
