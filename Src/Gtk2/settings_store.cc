#include "settings_store.h"

GConfClient *the_gconf_client;

bool get_bool_option_callback(const std::string& Key, bool *value)
{
  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  GError *the_error;

  GConfValue* got_value = gconf_client_get_without_default( the_gconf_client, keypath, &the_error);

  if (got_value==NULL) {
    return false;
  }

  *value = gconf_value_get_bool(got_value);
  gconf_value_free(got_value);
  return( true );
}

bool get_long_option_callback(const std::string& Key, long *value)
{
  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  GError *the_error;

  GConfValue* got_value = gconf_client_get_without_default( the_gconf_client, keypath, &the_error);

  if (got_value==NULL)
    return false;

  *value = gconf_value_get_int(got_value);
  gconf_value_free(got_value);
  return( true );
}

bool get_string_option_callback(const std::string& Key, std::string *value)
{
  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  GError *the_error;

  GConfValue* got_value = gconf_client_get_without_default( the_gconf_client, keypath, &the_error);

  if (got_value==NULL)
    return false;

  *value = gconf_value_get_string(got_value);
  gconf_value_free(got_value);
  return( true );
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
