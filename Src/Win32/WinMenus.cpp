#include "WinMenus.h"

using namespace WinMenus;

void WinMenus::GetWindowMenus() {
	EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()),WNDENUMPROC(WindowProc),LPARAM(0));
	ProcessWindows();
}

BOOL CALLBACK WinMenus::WindowProc(HWND hwnd, LPARAM lParam)
{
	windownames.push_back(hwnd);
	return TRUE;
}

void WinMenus::ProcessWindows() {
	void** AccessibleObject=0;
	for (int i=0; i<windownames.size(); i++) {
		AccessibleObjectFromWindow(windownames[i],OBJID_MENU,IID_IAccessible,AccessibleObject);
	}
}