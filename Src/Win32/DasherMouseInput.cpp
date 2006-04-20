#include "Common\WinCommon.h"

#include ".\dashermouseinput.h"

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CDasherMouseInput::CDasherMouseInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, HWND _hwnd)
: CDasherInput(pEventHandler, pSettingsStore, 0, 0, "Mouse Input"), m_hwnd(_hwnd) {
}

CDasherMouseInput::~CDasherMouseInput(void) {
}

int CDasherMouseInput::GetCoordinates(int iN, myint *pCoordinates) {

  POINT mousepos;
  GetCursorPos(&mousepos);

  ScreenToClient(m_hwnd, &mousepos);

  pCoordinates[0] = mousepos.x;
  pCoordinates[1] = mousepos.y;

  return 0;
}
