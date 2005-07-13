#ifndef __gnomedasherinterface_h__
#define __gnomedasherinterface_h__

#include "DasherInterfaceBase.h"
#include "GnomeSettingsStore.h"

namespace Dasher {
  class CDasherInterface;
}

class Dasher::CDasherInterface : public CDasherInterfaceBase {
 public:
  CDasherInterface() {
    m_pSettingsStore = new CGnomeSettingsStore( m_pEventHandler );
    //    SettingsDefaults( m_pSettingsStore );

    ChangeAlphabet( GetStringParameter(SP_ALPHABET_ID) );
  };

};

#endif





