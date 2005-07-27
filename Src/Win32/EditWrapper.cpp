#include "./EditWrapper.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CEditWrapper::CEditWrapper(void) {
  m_pEventHandler = NULL;
}

CEditWrapper::~CEditWrapper(void) {
}

void CEditWrapper::HandleEvent(Dasher::CEvent *pEvent) {

  // Just pass the event to the event handler if it exists

  if(m_pEventHandler) {
    m_pEventHandler->HandleEvent(pEvent);
  }
}
