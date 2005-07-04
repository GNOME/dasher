#ifndef __gnomesettingsstore_h__
#define __gnomesettingsstore_h__

#include <string>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-enum-types.h>

#include <stdio.h>

// NOTE TO SELF - STOP WRITING EVERYTHING IN THE HEADER FILES!!!

// FIXME - need to handle gconf errors better here

class CGnomeSettingsStore : public CSettingsStore
{
 public:
  CGnomeSettingsStore( Dasher::CEventHandler *pEventHandler, int argc, char **argv ) : CSettingsStore( pEventHandler ) {

    GError *gconferror;
    
    gconf_init( argc, argv, &gconferror );
    the_gconf_client = gconf_client_get_default();

  };

  ~CGnomeSettingsStore() {

    g_object_unref(the_gconf_client);

  };

 private:
  bool LoadSetting(const std::string& Key, bool* Value)
    {
      char keypath[1024];
      
      snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );
      
      GError *the_error=NULL;
      
      GConfValue* got_value = gconf_client_get_without_default( the_gconf_client, keypath, &the_error);
      
      if (got_value==NULL) {
	return false;
      }
      
      *Value = gconf_value_get_bool(got_value);
      gconf_value_free(got_value);

      return true;
    };

  bool LoadSetting(const std::string& Key, long* Value)
    {
      char keypath[1024];
      
      snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );
      
      GError *the_error=NULL;
      
      GConfValue* got_value = gconf_client_get_without_default( the_gconf_client, keypath, &the_error);
      
      if (got_value==NULL) {
	return false;
      }
      
      *Value = gconf_value_get_int(got_value);
      gconf_value_free(got_value);

      return true;
    };

  bool LoadSetting(const std::string& Key, std::string* Value)
    {
      char keypath[1024];
      
      snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );
      
      GError *the_error=NULL;
      
      GConfValue* got_value = gconf_client_get_without_default( the_gconf_client, keypath, &the_error);
      
      if (got_value==NULL) {
	return false;
      }
      
      *Value = gconf_value_get_string(got_value);
      gconf_value_free(got_value);

      return true;
    };

  virtual void SaveSetting(const std::string& Key, bool Value)
    {
      char keypath[1024];

      snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );
      
      GError *the_error=NULL;
      
      gconf_client_set_bool( the_gconf_client, keypath, Value, &the_error );
    };

  void SaveSetting(const std::string& Key, long Value)
    {
      char keypath[1024];

      snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );
      
      GError *the_error=NULL;
      
      gconf_client_set_int( the_gconf_client, keypath, Value, &the_error );
    };

  void SaveSetting(const std::string& Key, const std::string& Value)
    {
      char keypath[1024];

      snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );
      
      GError *the_error=NULL;
      
      gconf_client_set_string( the_gconf_client, keypath, Value.c_str(), &the_error );
    };

  GConfClient *the_gconf_client;
  GConfEngine *gconfengine;
};

#endif
