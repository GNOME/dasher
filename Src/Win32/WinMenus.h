#include "resource.h"
#include "WinWrap.h"
#include <vector>
#include <Oleacc.h>
#include "../DasherCore/DasherTypes.h"

using namespace Dasher;

namespace WinMenus {
	BOOL CALLBACK WindowProc(HWND hwnd, LPARAM lParam);

	ControlTree* GetWindowMenus();
	ControlTree* ProcessWindows();
	bool ProcessObject(IAccessible* AccessibleObject);
	UINT GetObjectState(IAccessible* pacc, VARIANT* pvarChild, LPTSTR lpszState, UINT cchState);
	bool AddObjectToTree(IAccessible* AccessibleObject, ControlTree* TreeParent);

};