#include "GnomeSettingsStore.h"

#define GCONF_KEY_ROOT "/apps/dasher4/"

CGnomeSettingsStore::CGnomeSettingsStore() {
  the_gconf_client = gconf_client_get_default();

  gconf_client_add_dir(the_gconf_client, "/apps/dasher4", GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);

  LoadPersistent();
}

CGnomeSettingsStore::~CGnomeSettingsStore() {
  g_object_unref(the_gconf_client);
}

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, bool *Value) {
  std::string keypath(GCONF_KEY_ROOT);
  keypath += Key;

  GError *the_error = NULL;

  GConfValue *got_value = gconf_client_get_without_default(the_gconf_client, keypath.c_str(), &the_error);

  if(got_value == NULL) {
    return false;
  }

  *Value = gconf_value_get_bool(got_value);
  gconf_value_free(got_value);

  return true;
}

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, long *Value) {
  std::string keypath(GCONF_KEY_ROOT);
  keypath += Key;

  GError *the_error = NULL;

  GConfValue *got_value = gconf_client_get_without_default(the_gconf_client, keypath.c_str(), &the_error);

  if(got_value == NULL) {
    return false;
  }

  *Value = gconf_value_get_int(got_value);
  gconf_value_free(got_value);

  return true;
}

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, std::string *Value) {
  std::string keypath(GCONF_KEY_ROOT);
  keypath += Key;

  GError *the_error = NULL;

  GConfValue *got_value = gconf_client_get_without_default(the_gconf_client, keypath.c_str(), &the_error);

  if(got_value == NULL) {
    return false;
  }

  *Value = gconf_value_get_string(got_value);
  gconf_value_free(got_value);

  return true;
}

void CGnomeSettingsStore::SaveSetting(const std::string &Key, bool Value) {
  std::string keypath(GCONF_KEY_ROOT);
  keypath += Key;

  GError *the_error = NULL;

  gconf_client_set_bool(the_gconf_client, keypath.c_str(), Value, &the_error);
}

void CGnomeSettingsStore::SaveSetting(const std::string &Key, long Value) {
  std::string keypath(GCONF_KEY_ROOT);
  keypath += Key;

  GError *the_error = NULL;

  gconf_client_set_int(the_gconf_client, keypath.c_str(), Value, &the_error);
}

void CGnomeSettingsStore::SaveSetting(const std::string &Key, const std::string &Value) {
  std::string keypath(GCONF_KEY_ROOT);
  keypath += Key;

  GError *the_error = NULL;

  gconf_client_set_string(the_gconf_client, keypath.c_str(), Value.c_str(), &the_error);
}
