#ifndef __gnomesettingsstore_h__
#define __gnomesettingsstore_h__



class CGnomeSettingsStore : public CSettingsStore
{
 public:
  CGnomeSettingsStore( Dasher::CEventHandler *pEventHandler ) : CSettingsStore( pEventHandler ) {};
 private:
  bool LoadSetting(const std::string& Key, bool* Value)
    {
      //      return( handle_get_bool_option( Key, Value ) );
      return false;
    };

  bool LoadSetting(const std::string& Key, long* Value)
    {
      //      return( handle_get_long_option( Key, Value ) );
      return false;
    };

  bool LoadSetting(const std::string& Key, std::string* Value)
    {
      //      return( handle_get_string_option( Key, Value ) );
      return false;
    };

  virtual void SaveSetting(const std::string& Key, bool Value)
    {
      //      handle_set_bool_option( Key, Value );
    };

  void SaveSetting(const std::string& Key, long Value)
    {
      //      handle_set_long_option( Key, Value );
    };

  void SaveSetting(const std::string& Key, const std::string& Value)
    {
      //      handle_set_string_option( Key, Value );
    };
};

#endif
