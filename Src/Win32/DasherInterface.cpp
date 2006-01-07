
#include "WinCommon.h"
#include "Dasherinterface.h"
#include "../../DasherCore/DasherInterfaceBase.h"
#include "WinOptions.h"

CDasherInterface::CDasherInterface(void) {
  m_pSettingsStore = new CWinOptions( "Inference Group", "Dasher3", m_pEventHandler );
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
