#include ".\dasherinterface.h"
#include "../DasherInterfaceBase.h"
#include "WinOptions.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CDasherInterface::CDasherInterface(void) {
  m_pSettingsStore = new CWinOptions( "Inference Group", "Dasher3", m_pEventHandler );

  Realize();

}

CDasherInterface::~CDasherInterface(void) {
  delete m_pSettingsStore;
}

CSettingsStore* CDasherInterface::CreateSettingsStore(void) {
  if(m_pSettingsStore)
    return m_pSettingsStore;

  m_pSettingsStore = new CWinOptions( "Inference Group", "Dasher3", m_pEventHandler );
  return m_pSettingsStore;
}
