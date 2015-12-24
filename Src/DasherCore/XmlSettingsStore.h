#ifndef XML_SETTING_STORE_H_
#define XML_SETTING_STORE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <map>

#include "SettingsStore.h"
#include "AbstractXMLParser.h"

class CFileUtils;

namespace Dasher {
// This class is not thread-safe.
class XmlSettingsStore : public Dasher::CSettingsStore, public AbstractXMLParser {
 public:
  XmlSettingsStore(const std::string& filename, CFileUtils* fileUtils, CMessageDisplay* pDisplay);
  ~XmlSettingsStore() override = default;
  // Load the XML file and fills in the default values needed.
  // Returns true on success.
  void Load();
  // Saves the XML file, returns true on success.
  bool Save();

 private:
  bool LoadSetting(const std::string& Key, bool* Value) override;
  bool LoadSetting(const std::string& Key, long* Value) override;
  bool LoadSetting(const std::string& Key, std::string* Value) override;

  void SaveSetting(const std::string& Key, bool Value) override;
  void SaveSetting(const std::string& Key, long Value) override;
  void SaveSetting(const std::string& Key, const std::string& Value) override;

  void XmlStartHandler(const XML_Char* name, const XML_Char** atts) override;
  virtual void XmlEndHandler(const XML_Char* name) override;

  // Parses the tag attributes expecting exactly one 'value' and one 'name'
  // attribute.
  bool GetNameAndValue(const XML_Char** attributes, std::string* name,
                       std::string* value);

  // Save if the mode is 'SAVE_IMMEDIATELY', otherwise just set 'modified_' to
  // true.
  void SaveIfNeeded();

  enum Mode {
    // Save each time 'SaveSetting' is called.
    SAVE_IMMEDIATELY,
    // Save only when 'Save' is called.
    EXPLICIT_SAVE
  };

  Mode mode_ = EXPLICIT_SAVE;
  std::string filename_;
  CFileUtils* fileutils_;
  bool modified_ = false;
  std::map<std::string, bool> boolean_settings_;
  std::map<std::string, long> long_settings_;
  std::map<std::string, std::string> string_settings_;
};

}  // namespace Dasher

#endif  // XML_SETTING_STORE_H_
