#pragma once

#include "../DasherInterfaceBase.h"

class CSettingsStore;
class CWinOptions;

#include "WinOptions.h"

class CDasherInterface:public Dasher::CDasherInterfaceBase {
public:
  CDasherInterface(void);
  ~CDasherInterface(void);

  CSettingsStore* CreateSettingsStore();

// Functions below here are depreciated
  CWinOptions *GetSettingsStore() {
    return reinterpret_cast < CWinOptions * >(m_pSettingsStore);
  }
};
