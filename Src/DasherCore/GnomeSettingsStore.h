#ifndef __gnomesettingsstore_h__
#define __gnomesettingsstore_h__

#include <string>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-enum-types.h>

#include "SettingsStore.h"

#include <stdio.h>

class CGnomeSettingsStore:public CSettingsStore {
public:
  CGnomeSettingsStore(Dasher::CEventHandler * pEventHandler);
  ~CGnomeSettingsStore();

  void NotificationCallback(GConfClient * pClient, guint iCNXN_ID, GConfEntry * pEntry);

private:
    bool LoadSetting(const std::string & Key, bool * Value);
  bool LoadSetting(const std::string & Key, long *Value);
  bool LoadSetting(const std::string & Key, std::string * Value);

  void SaveSetting(const std::string & Key, bool Value);
  void SaveSetting(const std::string & Key, long Value);
  void SaveSetting(const std::string & Key, const std::string & Value);

  GConfClient *the_gconf_client;
  GConfEngine *gconfengine;
};

#endif
