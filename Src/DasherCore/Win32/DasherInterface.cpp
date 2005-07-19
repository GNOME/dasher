#include ".\dasherinterface.h"
#include "WinOptions.h"

CDasherInterface::CDasherInterface(void) {
	m_pSettingsStore = new CWinOptions( "Inference Group", "Dasher3", m_pEventHandler );
    CDasherSettingsInterface::SetInterface(this);

    ChangeAlphabet( GetStringParameter(SP_ALPHABET_ID) );

}

CDasherInterface::~CDasherInterface(void)
{
    delete m_pSettingsStore;
}

CSettingsStore* CDasherInterface::CreateSettingsStore(void)
{
    if(m_pSettingsStore)
        return m_pSettingsStore;

    m_pSettingsStore = new CWinOptions( "Inference Group", "Dasher3", m_pEventHandler );
    return m_pSettingsStore;
}
