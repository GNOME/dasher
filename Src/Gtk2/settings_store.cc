#include "settings_store.h"

GConfClient *the_gconf_client;

bool get_bool_option_callback(const std::string& Key, bool *value)
{
  //  cout << "Get bool " << Key << endl;

  bool got_value;

  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  //  cout << "Searching for key " << keypath << endl;

  GError *the_error;

  if (gconf_engine_get( gconfengine, keypath, &the_error)==FALSE)
    return (false);

//   if( gconf_client_unset( the_gconf_client, keypath, &the_error ) )
//     return( false );

  got_value = gconf_client_get_bool( the_gconf_client, keypath, &the_error);

//   if( the_error->code == GCONF_ERROR_SUCCESS )
//     {
  //      cout << "got value " << got_value << endl;
      *value = got_value;
      return( true );
     
 //    }
//   else
//     {
//       cout << "could not get value - error code was " << the_error->code << endl;
//       return( false );
//     }

      *value = got_value;

  return( true );
}

bool get_long_option_callback(const std::string& Key, long *value)
{
  //  cout << "Get long " << Key << endl;

  long got_value;

  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  if (gconf_engine_get( gconfengine, keypath, NULL)==FALSE)
    return (false);


  //  cout << "Searching for key " << keypath << endl;


//   if( gconf_client_unset( the_gconf_client, keypath, NULL ) )
//     {
//       cout << "error was:" << endl;
//       return( false );
//     }


  got_value = gconf_client_get_int( the_gconf_client, keypath, NULL);

  *value = got_value;

  return( true );
}

bool get_string_option_callback(const std::string& Key, std::string *value)
{
  //  cout << "Get string " << Key << endl;
char * got_value;

  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  if (gconf_engine_get( gconfengine, keypath, NULL)==FALSE)
    return (false);

  //  cout << "Searching for key " << keypath << endl;

  //GError *the_error;

 //  if( gconf_client_unset( the_gconf_client, keypath, NULL ) )
//     {
//       cout << "Key didn't exist" << endl;
//       return( false );
//     }

  //  cout << "I survived the check" << endl;

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
  //  cout << "Setting bool option " << Key << " to " << Value << endl; 

  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  //  cout << "Searching for key " << keypath << endl;

  GError *the_error;

  gconf_client_set_bool( the_gconf_client, keypath, Value, &the_error );

}

void set_long_option_callback(const std::string& Key, long Value)
{
  // cout << "Setting bool option " << Key << " to " << Value << endl; 

  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  //  cout << "Searching for key " << keypath << endl;

  GError *the_error;

  gconf_client_set_int( the_gconf_client, keypath, Value, &the_error );
}

void set_string_option_callback(const std::string& Key, const std::string& Value)
{
  // cout << "Setting bool option " << Key << " to " << Value << endl; 

  char keypath[1024];

  snprintf( keypath, 1024, "/apps/dasher/%s", Key.c_str() );

  //  cout << "Searching for key " << keypath << endl;

  GError *the_error;

  gconf_client_set_string( the_gconf_client, keypath, Value.c_str(), &the_error );
}
