#include "resource.h"
#include "WinWrap.h"
#include <vector>
#include <Oleacc.h>
#include "../DasherCore/DasherTypes.h"

using namespace Dasher;

namespace WinMenus {
	BOOL CALLBACK WindowProc(HWND hwnd, LPARAM lParam);

	ControlTree* GetWindowMenus();
	ControlTree* buildcontroltree();
	ControlTree* buildmovetree(ControlTree* tree);
	ControlTree* builddeletetree(ControlTree* tree);
	ControlTree* ProcessWindows(ControlTree* tree);
	bool ProcessObject(IAccessible* AccessibleObject);
	UINT GetObjectState(IAccessible* pacc, VARIANT* pvarChild, LPTSTR lpszState, UINT cchState);
	bool AddObjectToTree(IAccessible* AccessibleObject, ControlTree* TreeParent);

};