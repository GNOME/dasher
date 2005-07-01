#include ".\dasherinterface.h"
#include "WinOptions.h"

CDasherInterface::CDasherInterface(void) {
	m_SettingsStore = new CWinOptions( "Inference Group", "Dasher3", m_pEventHandler );
	SettingsDefaults( m_SettingsStore );
}

CDasherInterface::~CDasherInterface(void)
{
}
