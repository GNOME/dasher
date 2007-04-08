#include <string>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>

#include <gdk/gdkx.h>

#include <xsettings-client.h>

#include <stdio.h>

using namespace std;

void init_xsettings();

bool get_bool_option_callback(const std::string & Key, bool * value);
bool get_long_option_callback(const std::string & Key, long *value);
bool get_string_option_callback(const std::string & Key, std::string * value);

void set_bool_option_callback(const std::string & Key, bool Value);
void set_long_option_callback(const std::string & Key, long Value);
void set_string_option_callback(const std::string & Key, const std::string & Value);
