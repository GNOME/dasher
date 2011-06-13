#include "GnomeSettingsStore.h"

#include <iostream>

CGnomeSettingsStore::CGnomeSettingsStore() {
  settings = g_settings_new("org.gnome.Dasher");
  LoadPersistent();
}

CGnomeSettingsStore::~CGnomeSettingsStore() {
  g_object_unref(settings);
}

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, bool *Value) {
  // If Key doesn't exist, or is not boolean, g_settings_get_boolean will
  // return false.
  *Value = g_settings_get_boolean(settings, Key.c_str());
  return true;
}

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, long *Value) {
  // It seems that this has never been able to store a long, only an int.
  *Value = g_settings_get_int(settings, Key.c_str());
  return true;
}

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, std::string *Value) {
  *Value = g_settings_get_string(settings, Key.c_str());
  return true;
}

void CGnomeSettingsStore::SaveSetting(const std::string &Key, bool Value) {
  // g_settings_set_boolean returns fals if the key was not writable.
  g_settings_set_boolean(settings, Key.c_str(), Value);
}

void CGnomeSettingsStore::SaveSetting(const std::string &Key, long Value) {
  g_settings_set_int(settings, Key.c_str(), Value);
}

void CGnomeSettingsStore::SaveSetting(const std::string &Key, const std::string &Value) {
  g_settings_set_string(settings, Key.c_str(), Value.c_str());
}
