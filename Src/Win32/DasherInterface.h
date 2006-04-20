#pragma once

#include "../DasherCore/DasherInterfaceBase.h"

class CSettingsStore;
class CWinOptions;

#include "Common/WinOptions.h"

class CDasherInterface:public Dasher::CDasherInterfaceBase {
public:
  CDasherInterface(void);
  ~CDasherInterface(void);

  void CreateSettingsStore();

// Functions below here are depreciated
  CWinOptions *GetSettingsStore() {
    return reinterpret_cast < CWinOptions * >(m_pSettingsStore);
  }
};
