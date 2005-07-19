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
    m_pSettingsStore = CreateSettingsStore();
  };

  virtual CSettingsStore *CreateSettingsStore() {
    return new CGnomeSettingsStore( m_pEventHandler );
  };

};

#endif





