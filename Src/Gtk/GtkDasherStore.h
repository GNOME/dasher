#ifndef GTKDASHERSTORE_H
#define GTKDASHERSTORE_H

#include <string>
#include <SettingsStore.h>

class GtkDasherStore : public CSettingsStore
{     
 public:
  GtkDasherStore();
 private:
  bool LoadSetting(const std::string& Key, bool* Value);
  bool LoadSetting(const std::string& Key, long* Value);
  bool LoadSetting(const std::string& Key, std::string* Value);
  
  void SaveSetting(const std::string& Key, bool Value);
  void SaveSetting(const std::string& Key, long Value);
  void SaveSetting(const std::string& Key, const std::string& Value);
};

#endif


