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

#include "../DasherCore/Win32/DasherInterface.h"
#include "Dasher.h"
using namespace Dasher;
using namespace std;

// DJW 20031029 - tip - don't use LPCWSTR explicitely
// instead TCHAR (or our Tstring) is your friend - it type-defs to char or wchar depending whether or not you have UNICODE defined



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

//  hInstApp = hInstance;
	// We don't want to starve other interactive applications
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);

  int iRet=0;

 
	CDasherWindow DasherWindow;
	
		//The UI will be updated to reflect settings

		DasherWindow.Show(nCmdShow);
		iRet = DasherWindow.MessageLoop();

// Close the COM library on the current thread
	CoUninitialize();

	return iRet;
}
