#include "DasherGUI.h"
#include "SplashScreen.h"

CDasherGUI::CDasherGUI(CDasherInterface* AttachInterface) : DasherInterface(AttachInterface)
{
	// Create splash screen
	CSplash Splash(0);
	Sleep(10000);
	
	// Read in Settings
	/*
	string AlphName = Settings::GetAlph();
	CAlphabet Alph = AlphIO::Get(AlphName);
	*/
	
	// Lay out window
	
	
	// Set up Dasher engine
	/*
	DasherInterface->ChangeAlphabet(Alph);
	DasherInterface->ChangeScreen(NewScreen)
	DasherInterface->ChangeEdit(NewEdit)
	*/
	
	// Train language model
		// Some sort of loop through a training file here
	DasherInterface->Train("hello", false);
	
}


void CDasherGUI::MainLoop()
{
	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
//		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
//		}
	}
}
