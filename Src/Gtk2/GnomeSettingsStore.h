#ifndef __gnomesettingsstore_h__
#define __gnomesettingsstore_h__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>

#ifdef WITH_GCONF
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-enum-types.h>
#endif

#include "SettingsStore.h"

#include <stdio.h>

class CGnomeSettingsStore:public CSettingsStore {
public:
  CGnomeSettingsStore(Dasher::CEventHandler * pEventHandler);
  ~CGnomeSettingsStore();

#ifdef WITH_GCONF
  void NotificationCallback(GConfClient * pClient, guint iCNXN_ID, GConfEntry * pEntry);
#endif

private:
  bool LoadSetting(const std::string & Key, bool * Value);
  bool LoadSetting(const std::string & Key, long * Value);
  bool LoadSetting(const std::string & Key, std::string * Value);

  void SaveSetting(const std::string & Key, bool Value);
  void SaveSetting(const std::string & Key, long Value);
  void SaveSetting(const std::string & Key, const std::string & Value);

#ifdef WITH_GCONF
  GConfClient *the_gconf_client;
  GConfEngine *gconfengine;
#endif
};

#endif
