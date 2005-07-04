#ifndef __gnomedasherinterface_h__
#define __gnomedasherinterface_h__

#include "DasherInterfaceBase.h"
#include "GnomeSettingsStore.h"

namespace Dasher {
  class CDasherInterface;
}

class Dasher::CDasherInterface : public CDasherInterfaceBase {
 public:
  CDasherInterface( int argc, char **argv ) {
    m_SettingsStore = new CGnomeSettingsStore( m_pEventHandler, argc, argv );
    SettingsDefaults( m_SettingsStore );
  };

};

#endif





