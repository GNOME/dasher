#ifndef GTKDASHERSTORE_H
#define GTKDASHERSTORE_H

#include <string>
#include <SettingsStore.h>
#include <map>

class GtkDasherStore : public CSettingsStore
{     
 public:
  GtkDasherStore();
  ~GtkDasherStore();
 private:
  bool LoadSetting(const std::string& Key, bool* Value);
  bool LoadSetting(const std::string& Key, long* Value);
  bool LoadSetting(const std::string& Key, std::string* Value);
  
  void SaveSetting(const std::string& Key, bool Value);
  void SaveSetting(const std::string& Key, long Value);
  void SaveSetting(const std::string& Key, const std::string& Value);

  std::map<std::string, bool> bmap;
  std::map<std::string, long> lmap;
  std::map<std::string, std::string> smap;

  void write_to_file();
  void read_from_file();
};

#endif


