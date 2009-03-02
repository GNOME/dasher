#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../Common/Common.h"

#include "GnomeSettingsStore.h"

#include <iostream>

// FIXME - need to handle gconf errors better here
#ifdef WITH_GCONF
extern "C" void ListenerCallback(GConfClient * pClient, guint iCNXN_ID, GConfEntry * pEntry, gpointer pUserData);
#endif

CGnomeSettingsStore::CGnomeSettingsStore(Dasher::CEventHandler *pEventHandler):CSettingsStore(pEventHandler) {
#ifdef WITH_GCONF
  // GError *gconferror;

  //    gconf_init( argc, argv, &gconferror );
  the_gconf_client = gconf_client_get_default();

  gconf_client_add_dir(the_gconf_client, "/apps/dasher4", GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);

  gconf_client_notify_add(the_gconf_client, "/apps/dasher4", ListenerCallback, this, NULL, NULL);
#endif

  LoadPersistent();
};

CGnomeSettingsStore::~CGnomeSettingsStore() {
#ifdef WITH_GCONF
  g_object_unref(the_gconf_client);
#endif

};

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, bool *Value) {
#ifdef WITH_GCONF
  char keypath[1024];

  snprintf(keypath, 1024, "/apps/dasher4/%s", Key.c_str());

  GError *the_error = NULL;

  GConfValue *got_value = gconf_client_get_without_default(the_gconf_client, keypath, &the_error);

  if(got_value == NULL) {
    return false;
  }

  *Value = gconf_value_get_bool(got_value);
  gconf_value_free(got_value);

  return true;
#else
  return false;
#endif
};

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, long *Value) {
#ifdef WITH_GCONF
  char keypath[1024];

  snprintf(keypath, 1024, "/apps/dasher4/%s", Key.c_str());

  GError *the_error = NULL;

  GConfValue *got_value = gconf_client_get_without_default(the_gconf_client, keypath, &the_error);

  if(got_value == NULL) {
    return false;
  }

  *Value = gconf_value_get_int(got_value);
  gconf_value_free(got_value);

  return true;
#else
  return false;
#endif
};

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, std::string *Value) {
#ifdef WITH_GCONF
  char keypath[1024];

  snprintf(keypath, 1024, "/apps/dasher4/%s", Key.c_str());

  GError *the_error = NULL;

  GConfValue *got_value = gconf_client_get_without_default(the_gconf_client, keypath, &the_error);

  if(got_value == NULL) {
    return false;
  }

  *Value = gconf_value_get_string(got_value);
  gconf_value_free(got_value);

  return true;
#else
  return false;
#endif
};

void CGnomeSettingsStore::SaveSetting(const std::string &Key, bool Value) {
#ifdef WITH_GCONF
  char keypath[1024];

  snprintf(keypath, 1024, "/apps/dasher4/%s", Key.c_str());

  GError *the_error = NULL;

  gconf_client_set_bool(the_gconf_client, keypath, Value, &the_error);
#endif
};

void CGnomeSettingsStore::SaveSetting(const std::string &Key, long Value) {
#ifdef WITH_GCONF
  char keypath[1024];

  snprintf(keypath, 1024, "/apps/dasher4/%s", Key.c_str());

  GError *the_error = NULL;

  gconf_client_set_int(the_gconf_client, keypath, Value, &the_error);
#endif
};

void CGnomeSettingsStore::SaveSetting(const std::string &Key, const std::string &Value) {
#ifdef WITH_GCONF
  char keypath[1024];

  snprintf(keypath, 1024, "/apps/dasher4/%s", Key.c_str());

  GError *the_error = NULL;

  gconf_client_set_string(the_gconf_client, keypath, Value.c_str(), &the_error);
#endif
};

#ifdef WITH_GCONF
void CGnomeSettingsStore::NotificationCallback(GConfClient *pClient, guint iCNXN_ID, GConfEntry *pEntry) {
  return;

  std::string strKey(pEntry->key);
  std::string strKeyName(strKey.substr(strKey.find_last_of("/") + 1));

  for(int i(0); i < NUM_OF_BPS; ++i) {
    if(s_oParamTables.BoolParamTable[i].regName == strKeyName) {

      bool bValue;
      LoadSetting(s_oParamTables.BoolParamTable[i].regName, &bValue);

      bool bOldValue(s_oParamTables.BoolParamTable[i].value);

      // We need to check that the value has actually changed, otherwise we'll get a loop

      if(bValue != bOldValue) {
        SetBoolParameter(i + FIRST_BP, bValue);
      }

      return;
    }
  }

  for(int i(0); i < NUM_OF_LPS; ++i) {
    if(s_oParamTables.LongParamTable[i].regName == strKeyName) {

      long lValue;
      LoadSetting(s_oParamTables.LongParamTable[i].regName, &lValue);

      long lOldValue(s_oParamTables.LongParamTable[i].value);

      if(lValue != lOldValue) {
        SetLongParameter(i + FIRST_LP, lValue);
      }

      return;
    }
  }

  for(int i(0); i < NUM_OF_SPS; ++i) {
    if(s_oParamTables.StringParamTable[i].regName == strKeyName) {

      std::string strValue;
      LoadSetting(s_oParamTables.StringParamTable[i].regName, &strValue);

      std::string strOldValue(s_oParamTables.StringParamTable[i].value);

      if(strValue != strOldValue) {
        SetStringParameter(i + FIRST_SP, strValue);
      }

      return;
    }
  }
}

extern "C" void ListenerCallback(GConfClient *pClient, guint iCnxn_ID, GConfEntry *pEntry, gpointer pUserData) {
  static_cast < CGnomeSettingsStore * >(pUserData)->NotificationCallback(pClient, iCnxn_ID, pEntry);
}
#endif
