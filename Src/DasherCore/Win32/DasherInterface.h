#pragma once
#include "../DasherInterfaceBase.h"

#include "WinOptions.h"

class CDasherInterface:public CDasherInterfaceBase {
public:
  CDasherInterface(void);
  ~CDasherInterface(void);

  CSettingsStore* CreateSettingsStore();

// Functions below here are depreciated
  CWinOptions *GetSettingsStore() {
    return reinterpret_cast < CWinOptions * >(m_pSettingsStore);
  }
};
