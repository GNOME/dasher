#ifndef __MockSettingsStore_h__
#define __MockSettingsStore_h__

#include "../DasherCore/SettingsStore.h"

using namespace Dasher;

/**
 * A useless, but concrete implementation of CSettingsStore.
 * used for unit testing purposes. Allows us to instantiate 
 * CSettingsStore without using platform specific code.
 */
class CMockSettingsStore : public CSettingsStore {

  public:
  
    CMockSettingsStore(Dasher::CEventHandler * pEventHandler) : CSettingsStore(pEventHandler) {}
    
    bool LoadSetting(const std::string & Key, bool * Value) { return true; }
    
    bool LoadSetting(const std::string & Key, long * Value) { return true; }
    
    void SaveSetting(const std::string & Key, bool Value) {}
    
    void SaveSetting(const std::string & Key, long Value) {}
    
    void SaveSetting(const std::string & Key, const std::string & Value) {}
};

#endif
