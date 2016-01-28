#include "XmlSettingsStore.h"
#include "DasherInterfaceBase.h"

#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>


namespace Dasher {
namespace {

template <typename T>
bool Read(const std::map<std::string, T> values, const std::string& key,
          T* value) {
  auto i = values.find(key);
  if (i == values.end()) {
    return false;
  }
  *value = i->second;
  return true;
}

}  // namespace

XmlSettingsStore::XmlSettingsStore(const std::string& filename, CFileUtils* fileUtils,
                                   CMessageDisplay* pDisplay)
    : AbstractXMLParser(pDisplay), filename_(filename),fileutils_(fileUtils) {}

void XmlSettingsStore::Load() {
  fileutils_->ScanFiles(this, filename_);
  // Load all the settings or create defaults for the ones that don't exist.
  // The superclass 'ParseFile' saves default settings if not found.
  mode_ = EXPLICIT_SAVE;
  LoadPersistent();
  mode_ = SAVE_IMMEDIATELY;
}

bool XmlSettingsStore::LoadSetting(const std::string& key, bool* value) {
  return Read(boolean_settings_, key, value);
}

bool XmlSettingsStore::LoadSetting(const std::string& key, long* value) {
  return Read(long_settings_, key, value);
}

bool XmlSettingsStore::LoadSetting(const std::string& key, std::string* value) {
  return Read(string_settings_, key, value);
}

void XmlSettingsStore::SaveSetting(const std::string& key, bool value) {
  boolean_settings_[key] = value;
  SaveIfNeeded();
}

void XmlSettingsStore::SaveSetting(const std::string& key, long value) {
  long_settings_[key] = value;
  SaveIfNeeded();
}

void XmlSettingsStore::SaveSetting(const std::string& key,
                                   const std::string& value) {
  string_settings_[key] = value;
  SaveIfNeeded();
}

void XmlSettingsStore::SaveIfNeeded() {
  modified_ = true;
  if (mode_ == SAVE_IMMEDIATELY) {
    Save();
  }
}

bool XmlSettingsStore::Save() {
  if (!modified_) {
    return true;
  }
  
    modified_ = false;
    std::stringstream out;
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    out << "<!DOCTYPE settings SYSTEM \"settings.dtd\">\n";

    out << "<settings>\n";
    for (const auto& p : long_settings_) {
      out << "<long name=\"" << p.first << "\" value=\"" << p.second
          << "\"/>\n";
    }
    for (const auto& p : boolean_settings_) {
      std::string value = p.second ? "True" : "False";
      out << "<bool name=\"" << p.first << "\" value=\"" << value << "\"/>\n";
    }
    for (const auto& p : string_settings_) {
      std::string attribute = p.second;
      XML_Escape(attribute, true /* attribute */);
      out << "<string name=\"" << p.first << "\" value=\"" << attribute
          << "\"/>\n";
    }
    out << "</settings>\n";
    return fileutils_->WriteUserDataFile(filename_, out.str(),false);

}

bool XmlSettingsStore::GetNameAndValue(const XML_Char** attributes,
                                       std::string* name, std::string* value) {
  bool found_name = false, found_value = false;
  for (; *attributes != nullptr; attributes += 2) {
    if (strcmp(attributes[0], "value") == 0) {
      if (found_value) {
        m_pMsgs->Message(
            "XML configuration: the 'value' attribute can only be present "
            "once in a tag",
            true /* interrupt */);
        return false;
      }
      *value = attributes[1];
      found_value = true;
    } else if (strcmp(attributes[0], "name") == 0) {
      if (found_name) {
        m_pMsgs->Message(
            "XML configuration: the 'name' attribute can only be present "
            "once in a tag",
            true /* interrupt */);
        return false;
      }
      *name = attributes[1];
      if (name->empty()) {
        m_pMsgs->Message(
            "XML configuration: the 'name' attribute can not be empty.",
            true /* interrupt */);
        return false;
      }
      found_name = true;
    } else {
      m_pMsgs->FormatMessageWithString(
          "XML configuration: invalid attribute: %s", *attributes);
      return false;
    }
  }
  if (!found_name || !found_value) {
    m_pMsgs->Message("XML configuration: missing name or value in a tag.",
                     true /* interrupt */);
    return false;
  }
  return true;
}

void XmlSettingsStore::XmlStartHandler(const XML_Char* element_name,
                                       const XML_Char** attributes) {
  std::string element = element_name;
  if (element == "settings") {
    return;
  }
  std::string name, value;
  if (!GetNameAndValue(attributes, &name, &value)) {
    return;
  }
  if (element == "string") {
    string_settings_[name] = value;
  } else if (element == "long") {
    errno = 0;
    long v = std::strtol(value.c_str(), nullptr, 0 /* base */);
    if (errno != 0) {
      m_pMsgs->FormatMessageWith2Strings(
          "XML configuration: invalid numeric value '%s' for '%s'",
          value.c_str(), name.c_str());
    }
    long_settings_[name] = v;
  } else if (element == "bool") {

    if (strcmp(value.c_str(), "True") == 0) {
      boolean_settings_[name] = true;
    } else if (strcmp(value.c_str(), "False") == 0) {
      boolean_settings_[name] = false;
    } else {
      m_pMsgs->FormatMessageWith2Strings(
          "XML configuration: boolean value should be 'True' or 'False' found "
          "%s = '%s'",
          name.c_str(), value.c_str());
    }
  } else {
    m_pMsgs->FormatMessageWithString("XML configuration: unknown tag '%s'",
                                     element.c_str());
  }
}

void XmlSettingsStore::XmlEndHandler(const XML_Char* ) {}
}  // namespace Dasher
