#include "GnomeSettingsStore.h"

CGnomeSettingsStore::CGnomeSettingsStore(Dasher::CEventHandler *pEventHandler):CSettingsStore(pEventHandler) {
  LoadPersistent();
}

CGnomeSettingsStore::~CGnomeSettingsStore() {
}

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, bool *Value) {
  return false;
}

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, long *Value) {
  return false;
}

bool CGnomeSettingsStore::LoadSetting(const std::string &Key, std::string *Value) {
  return false;
}

void CGnomeSettingsStore::SaveSetting(const std::string &Key, bool Value) {
}

void CGnomeSettingsStore::SaveSetting(const std::string &Key, long Value) {
}

void CGnomeSettingsStore::SaveSetting(const std::string &Key, const std::string &Value) {
}
