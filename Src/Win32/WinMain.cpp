// WinMain.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////


#include "WinHelper.h"
#include "DasherWindow.h"
#include "Widgets/WinOptions.h"
#include "WinUTF8.h"
#include "WinMenus.h"

#include "../DasherCore/DasherInterface.h"
using namespace Dasher;

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
		Tstring_to_UTF8string(find.cFileName, &filename);
		Interface->AddAlphabetFilename(filename);
		while (FindNextFile(handle,&find)!=false) {
			Tstring_to_UTF8string(find.cFileName, &filename);
			Interface->AddAlphabetFilename(filename);
		}
	}

	handle=FindFirstFile(Colours.c_str(),&find);
	if (handle!=INVALID_HANDLE_VALUE) {
		Tstring_to_UTF8string(find.cFileName, &filename);
		Interface->AddColourFilename(filename);
		while (FindNextFile(handle,&find)!=false) {
			Tstring_to_UTF8string(find.cFileName, &filename);
			Interface->AddColourFilename(filename);
		}
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
	Tstring_to_UTF8string(UserData, &UserData2); // TODO: I don't know if special characters will work.
	Tstring_to_UTF8string(AppData, &AppData2);   // ASCII-only filenames are safest. Being English doesn't help debug this...

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
	Colours+=TEXT("colours*.xml");
	AddFiles(Alphabets,Colours,&DasherInterface);

	DasherInterface.SetSettingsStore(&WinOptions);        // which will now use Windows Registry
	DasherInterface.ColourMode(true);

	{
		CDasherWindow DasherWindow(&DasherInterface, &DasherInterface, &DasherInterface); // Main Window
		DasherInterface.SetSettingsUI(&DasherWindow);         // The UI will be updated to reflect settings
		DasherInterface.ChangeLanguageModel(0);
		DasherInterface.AddControlTree(WinMenus::GetWindowMenus()); // Build control tree
		DasherWindow.Show(nCmdShow);                          // Now set up. Kill splash screen and display main window

		iRet = DasherWindow.MessageLoop();
	}

	return iRet;
}

