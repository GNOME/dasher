#include <string>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-enum-types.h>

#include <stdio.h>

using namespace std;

bool get_bool_option_callback(const std::string& Key, bool *value);
bool get_long_option_callback(const std::string& Key, long *value);
bool get_string_option_callback(const std::string& Key, std::string *value);
  
void set_bool_option_callback(const std::string& Key, bool Value);
void set_long_option_callback(const std::string& Key, long Value);
void set_string_option_callback(const std::string& Key, const std::string& Value);

extern GConfClient *the_gconf_client;
