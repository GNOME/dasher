#include "settings_store.h"

GConfClient *the_gconf_client;

bool get_bool_option_callback(const std::string& Key, bool *value)
{
  bool got_value;

  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  GError *the_error;

  if (gconf_engine_get( gconfengine, keypath, &the_error)==FALSE)
    return (false);

  got_value = gconf_client_get_bool( the_gconf_client, keypath, &the_error);

  *value = got_value;
  return( true );
}

bool get_long_option_callback(const std::string& Key, long *value)
{
  long got_value;

  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  if (gconf_engine_get( gconfengine, keypath, NULL)==FALSE)
    return (false);

  got_value = gconf_client_get_int( the_gconf_client, keypath, NULL);

  *value = got_value;

  return( true );
}

bool get_string_option_callback(const std::string& Key, std::string *value)
{
  char * got_value;

  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  if (gconf_engine_get( gconfengine, keypath, NULL)==FALSE)
    return (false);

  got_value = gconf_client_get_string( the_gconf_client, keypath, NULL);

  if( got_value != NULL )
    {
      *value = std::string( got_value );

      return( true );
    }
  else
    return( false );
}
  
void set_bool_option_callback(const std::string& Key, bool Value)
{
  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  GError *the_error;

  gconf_client_set_bool( the_gconf_client, keypath, Value, &the_error );
}

void set_long_option_callback(const std::string& Key, long Value)
{
  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  GError *the_error;

  gconf_client_set_int( the_gconf_client, keypath, Value, &the_error );
}

void set_string_option_callback(const std::string& Key, const std::string& Value)
{
  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  GError *the_error;

  gconf_client_set_string( the_gconf_client, keypath, Value.c_str(), &the_error );
}
