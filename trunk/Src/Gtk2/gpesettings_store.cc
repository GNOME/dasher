#include "../Common/Common.h"

#include "gpesettings_store.h"

XSettingsClient *client;

static GdkFilterReturn xsettings_event_filter(GdkXEvent *xevp, GdkEvent *ev, gpointer p) {
  if(xsettings_client_process_event(client, (XEvent *) xevp))
    return GDK_FILTER_REMOVE;

  return GDK_FILTER_CONTINUE;
}

static void notify_func(const char *name, XSettingsAction action, XSettingsSetting *setting, void *cb_data) {
  size_t length;
  char *settingname;
  if(strncmp(name, "dasher/", 7) == 0) {
    length = strlen(name);
    settingname = (char *)malloc(length * sizeof(char));
    for(size_t i = 0; i < length; i++) {
      settingname[i] = name[i + 7];
    }
    free(settingname);
  }
}

static void watch_func(Window window, Bool is_start, long mask, void *cb_data) {
  GdkWindow *gdkwin;

  gdkwin = gdk_window_lookup(window);

  if(is_start) {
    if(!gdkwin)
      gdkwin = gdk_window_foreign_new(window);
    else
      g_object_ref(gdkwin);

    gdk_window_add_filter(gdkwin, xsettings_event_filter, NULL);
  }
  else {
    g_object_unref(gdkwin);
    gdk_window_remove_filter(gdkwin, xsettings_event_filter, NULL);
  }
}

void init_xsettings() {
  client = xsettings_client_new(GDK_DISPLAY(), DefaultScreen(GDK_DISPLAY()), notify_func, watch_func, NULL);
}

bool get_long_option_callback(const std::string &Key, long *value) {
  char keypath[1024];

  snprintf(keypath, 1024, "dasher/%s", Key.c_str());

  XSettingsSetting *setting;

  if(xsettings_client_get_setting(client, keypath, &setting) != XSETTINGS_SUCCESS) {
    return false;
  }

  if(setting->type == XSETTINGS_TYPE_INT) {
    *value = setting->data.v_int;
  }

  return (true);
}

bool get_bool_option_callback(const std::string &Key, bool *value) {
  long tmp;
  if(get_long_option_callback(Key, &tmp) == false) {
    return false;
  }
  if(tmp) {
    *value = true;
  }
  else {
    *value = false;
  }
  return true;
}

bool get_string_option_callback(const std::string &Key, std::string *value) {
  char keypath[1024];

  snprintf(keypath, 1024, "dasher/%s", Key.c_str());

  XSettingsSetting *setting;

  if(xsettings_client_get_setting(client, keypath, &setting) != XSETTINGS_SUCCESS) {
    return false;
  }

  if(setting->type == XSETTINGS_TYPE_STRING) {
    *value = setting->data.v_string;
  }

  return (true);
}

void set_long_option_callback(const std::string &Key, long Value) {
  int realvalue = Value;
  long currentvalue;
  Atom gpe_settings_update_atom = XInternAtom(GDK_DISPLAY(), "_GPE_SETTINGS_UPDATE", 0);
  Window manager = XGetSelectionOwner(GDK_DISPLAY(), gpe_settings_update_atom);
  XSettingsType type;
  size_t length, name_len;
  gchar *buffer;
  XClientMessageEvent ev;
  gboolean done = FALSE;
  Window win;
  char keypath[1024];

  get_long_option_callback(Key, &currentvalue);

  if(currentvalue == Value)
    return;

  snprintf(keypath, 1024, "dasher/%s", Key.c_str());

  if(manager == None) {
    fprintf(stderr, "gpe-confd not running.\n");
  }

  win = XCreateSimpleWindow(GDK_DISPLAY(), DefaultRootWindow(GDK_DISPLAY()), 1, 1, 1, 1, 0, 0, 0);

  type = XSETTINGS_TYPE_INT;
  length = 4;

  name_len = strlen(keypath);
  // This crack rounds up to the nearest 4
  name_len = (name_len + 3) & ~3;
  buffer = (gchar *) g_malloc(length + 4 + name_len);
  *buffer = type;
  buffer[1] = 0;
  buffer[2] = name_len & 0xff;
  buffer[3] = (name_len >> 8) & 0xff;
  memcpy(buffer + 4, keypath, name_len);
  *((unsigned long *)(buffer + 4 + name_len)) = realvalue;

  XChangeProperty(GDK_DISPLAY(), win, gpe_settings_update_atom, gpe_settings_update_atom, 8, PropModeReplace, (const unsigned char *)buffer, length + 4 + name_len);
  g_free(buffer);
  XSelectInput(GDK_DISPLAY(), win, PropertyChangeMask);
  ev.type = ClientMessage;
  ev.window = win;
  ev.message_type = gpe_settings_update_atom;
  ev.format = 32;
  ev.data.l[0] = gpe_settings_update_atom;
  XSendEvent(GDK_DISPLAY(), manager, FALSE, 0, (XEvent *) & ev);

  while(!done) {
    XEvent ev;
    XNextEvent(GDK_DISPLAY(), &ev);
    switch (ev.xany.type) {
    case PropertyNotify:
      if(ev.xproperty.window == win && ev.xproperty.atom == gpe_settings_update_atom)
        done = TRUE;
      break;
    }
  }
}

void set_bool_option_callback(const std::string &Key, bool Value) {
  set_long_option_callback(Key, long (Value));
}

void set_string_option_callback(const std::string &Key, const std::string &Value) {
  Atom gpe_settings_update_atom = XInternAtom(GDK_DISPLAY(), "_GPE_SETTINGS_UPDATE", 0);
  Window manager = XGetSelectionOwner(GDK_DISPLAY(), gpe_settings_update_atom);
  XSettingsType type;
  std::string currentvalue;
  size_t length, name_len;
  gchar *buffer;
  XClientMessageEvent ev;
  gboolean done = FALSE;
  Window win;
  char keypath[1024];

  get_string_option_callback(Key, &currentvalue);
  if(currentvalue == Value)
    return;

  snprintf(keypath, 1024, "dasher/%s", Key.c_str());

  if(manager == None) {
    fprintf(stderr, "gpe-confd not running.\n");
  }

  win = XCreateSimpleWindow(GDK_DISPLAY(), DefaultRootWindow(GDK_DISPLAY()), 1, 1, 1, 1, 0, 0, 0);

  type = XSETTINGS_TYPE_STRING;
  length = 4 + (strlen(Value.c_str()) + 3) & ~3;

  name_len = strlen(keypath);
  // As does this crack
  name_len = (name_len + 3) & ~3;
  buffer = (gchar *) g_malloc(length + 4 + name_len);
  *buffer = type;
  buffer[1] = 0;
  buffer[2] = name_len & 0xff;
  buffer[3] = (name_len >> 8) & 0xff;
  memcpy(buffer + 4, keypath, name_len);
  *((unsigned long *)(buffer + 4 + name_len)) = strlen(Value.c_str());
  memcpy(buffer + 8 + name_len, Value.c_str(), strlen(Value.c_str()));
  XChangeProperty(GDK_DISPLAY(), win, gpe_settings_update_atom, gpe_settings_update_atom, 8, PropModeReplace, (const unsigned char *)buffer, length + 4 + name_len);
  g_free(buffer);
  XSelectInput(GDK_DISPLAY(), win, PropertyChangeMask);
  ev.type = ClientMessage;
  ev.window = win;
  ev.message_type = gpe_settings_update_atom;
  ev.format = 32;
  ev.data.l[0] = gpe_settings_update_atom;
  XSendEvent(GDK_DISPLAY(), manager, FALSE, 0, (XEvent *) & ev);
  while(!done) {
    XEvent ev;
    XNextEvent(GDK_DISPLAY(), &ev);

    switch (ev.xany.type) {
    case PropertyNotify:
      if(ev.xproperty.window == win && ev.xproperty.atom == gpe_settings_update_atom)
        done = TRUE;
      break;
    }
  }
}
