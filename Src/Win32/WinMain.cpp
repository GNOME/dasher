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
	
	// Get folder names for system and user data.
	Tstring UserData, AppData;
	std::string UserData2, AppData2;
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

	// Set up Dasher
	CDasherInterface DasherInterface;                     // Fat interface to all of Dasher's core,
	DasherInterface.SetSystemLocation(AppData2);
	DasherInterface.SetUserLocation(UserData2);
	DasherInterface.SetSettingsStore(&WinOptions);        // which will now use Windows Registry
	CDasherWindow DasherWindow(&DasherInterface, &DasherInterface, &DasherInterface); // Main Window
	DasherInterface.SetSettingsUI(&DasherWindow);         // The UI will be updated to reflect settings
	DasherInterface.ChangeLanguageModel(0);
	DasherInterface.AddControlTree(WinMenus::GetWindowMenus());
	DasherWindow.Show(nCmdShow);                          // Now set up. Kill splash screen and display main window

	return DasherWindow.MessageLoop();
}
