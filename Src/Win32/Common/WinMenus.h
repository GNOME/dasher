#ifndef __WinMenus_h__
#define __WinMenus_h__

#include "WinWrap.h"
#include <vector>
#include "../../DasherCore/DasherTypes.h"
#include <Oleacc.h>

// Should we really pollute the global namespace with this using directive in a header file?
using namespace Dasher;

namespace WinMenus {
  
BOOL CALLBACK WindowProc(HWND hwnd, LPARAM lParam);
  
ControlTree * GetWindowMenus();
  
ControlTree * buildcontroltree();
  
ControlTree * buildmovetree(ControlTree * tree);
  
ControlTree * builddeletetree(ControlTree * tree);
  
ControlTree * buildspeaktree(ControlTree * tree);
  
ControlTree * ProcessWindows(ControlTree * tree);
  
bool ProcessObject(IAccessible * AccessibleObject);
  
UINT GetObjectState(IAccessible * pacc, VARIANT * pvarChild, LPTSTR lpszState, UINT cchState);
  
bool AddObjectToTree(IAccessible * AccessibleObject, ControlTree * TreeParent);


};

#endif
