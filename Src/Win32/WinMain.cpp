// WinMain.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

//#ifdef _DEBUG
// #include "vld.h"
//#endif 

#include "Common/WinHelper.h"

#include "DasherWindow.h"
#include "Widgets/WinOptions.h"

#include "../DasherCore/DasherInterface.h"
using namespace Dasher;
using namespace std;

// DJW 20031029 - tip - don't use LPCWSTR explicitely
// instead TCHAR (or our Tstring) is your friend - it type-defs to char or wchar depending whether or not you have UNICODE defined

//void AddFiles(LPCWSTR Alphabets, LPCWSTR Colours, CDasherInterface *Interface)
void AddFiles(Tstring Alphabets, Tstring Colours, CDasherInterface *Interface)
{
	using namespace WinHelper;
	using namespace WinUTF8;

	std::string filename;
	WIN32_FIND_DATA find;
	HANDLE handle;

	handle=FindFirstFile(Alphabets.c_str(),&find);
	if (handle!=INVALID_HANDLE_VALUE) {
		wstring_to_UTF8string(wstring(find.cFileName), filename);
		Interface->AddAlphabetFilename(filename);
		while (FindNextFile(handle,&find)!=false) 
		{
			wstring_to_UTF8string(wstring(find.cFileName), filename);
			Interface->AddAlphabetFilename(filename);
		}
		FindClose(handle);
	}

	handle=FindFirstFile(Colours.c_str(),&find);
	if (handle!=INVALID_HANDLE_VALUE) {
		wstring_to_UTF8string(find.cFileName, filename);
		Interface->AddColourFilename(filename);
		while (FindNextFile(handle,&find)!=false) {
			wstring_to_UTF8string(find.cFileName, filename);
			Interface->AddColourFilename(filename);
		}
		FindClose(handle);

	}
}

/*
Entry point to program on Windows systems

An interface to the Dasher library is created.
A GUI and settings manager are created and given to the Dasher interface.
Control is passed to the main GUI loop, and only returns when the main window closes.
*/
int APIENTRY WinMain(HINSTANCE hInstance, 
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow)
{

	// String literals in this function are not in the resource file as they
	// must NOT be translated.

	using namespace WinHelper;
	using namespace WinUTF8;
	hInstApp = hInstance;

	// We don't want to starve other interactive applications
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);

	// Get folder names for system and user data.
	Tstring UserData, AppData;
	std::string UserData2, AppData2;
	Tstring Alphabets, Colours;
	GetUserDirectory(&UserData);
	GetAppDirectory(&AppData);
	UserData+=TEXT("dasher.rc\\");
	AppData+=TEXT("system.rc\\");
	CreateDirectory(UserData.c_str(), NULL); // Try and create folders. Doesn't seem
	CreateDirectory(AppData.c_str(), NULL);  // to do any harm if they already exist.
	wstring_to_UTF8string(UserData, UserData2); // TODO: I don't know if special characters will work.
	wstring_to_UTF8string(AppData, AppData2);   // ASCII-only filenames are safest. Being English doesn't help debug this...

	// Set up COM for the accessibility stuff
	CoInitialize(NULL);

	// Set up the registry
	CWinOptions WinOptions("Inference_Group", "Dasher3"); // Settings storage using Windows Registry.

	int iRet=0;

	// Set up Dasher
	CDasherInterface DasherInterface;                     // Fat interface to all of Dasher's core,
	DasherInterface.SetSystemLocation(AppData2);
	DasherInterface.SetUserLocation(UserData2);

	Alphabets=UserData;
	Alphabets+=TEXT("alphabet*.xml");
	Colours=UserData;
	Colours+=TEXT("colour*.xml");
	AddFiles(Alphabets,Colours,&DasherInterface);
	Alphabets=AppData;
	Alphabets+=TEXT("alphabet*.xml");
	Colours=AppData;
	Colours+=TEXT("colour*.xml");
	AddFiles(Alphabets,Colours,&DasherInterface);

	DasherInterface.SetSettingsStore(&WinOptions);        // which will now use Windows Registry
	DasherInterface.ColourMode(true);

	{
   		DasherInterface.ChangeLanguageModel(0);
		CDasherWindow DasherWindow(&DasherInterface, &DasherInterface, &DasherInterface, WinOptions); 
	
		//The UI will be updated to reflect settings
		DasherInterface.SetSettingsUI(&DasherWindow);         
		DasherWindow.Show(nCmdShow);
		iRet = DasherWindow.MessageLoop();
	}

	// Close the COM library on the current thread
	CoUninitialize();

	return iRet;
}
