#include "resource.h"
#include "WinWrap.h"
#include <vector>
#include <Oleacc.h>

namespace WinMenus {
	BOOL CALLBACK WindowProc(HWND hwnd, LPARAM lParam);

	void GetWindowMenus();
	void ProcessWindows();

	std::vector <HWND> windownames;
};