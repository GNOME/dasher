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

CDasherMouseInput::CDasherMouseInput(HWND _hwnd)
: CScreenCoordInput(0, "Mouse Input"), m_hwnd(_hwnd) {
}

CDasherMouseInput::~CDasherMouseInput(void) {
}

bool CDasherMouseInput::GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView) {

  POINT mousepos;
  GetCursorPos(&mousepos);

  ScreenToClient(m_hwnd, &mousepos);

  iX = mousepos.x;
  iY = mousepos.y;

  return true;
}
