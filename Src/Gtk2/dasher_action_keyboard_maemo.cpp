// TODO: Make inclusion of this file in build conditional
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef WITH_MAEMO

#include "dasher.h"
#include "dasher_maemo_helper.h"
#include "dasher_action_keyboard_maemo.h"

#include <gdk/gdkx.h>

static void dasher_action_keyboard_maemo_class_init(DasherActionKeyboardMaemoClass *pClass);
static void dasher_action_keyboard_maemo_init(DasherActionKeyboardMaemo *pActionKeyboardMaemo);
static void dasher_action_keyboard_maemo_destroy(GObject *pObject);
static gboolean dasher_action_keyboard_maemo_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx);
static const gchar *dasher_action_keyboard_maemo_get_name(DasherAction *pSelf);

GType dasher_action_keyboard_maemo_get_type() {

  static GType dasher_action_keyboard_maemo_type = 0;

  if(!dasher_action_keyboard_maemo_type) {
    static const GTypeInfo dasher_action_keyboard_maemo_info = {
      sizeof(DasherActionKeyboardMaemoClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_action_keyboard_maemo_class_init,
      NULL,
      NULL,
      sizeof(DasherActionKeyboardMaemo),
      0,
      (GInstanceInitFunc) dasher_action_keyboard_maemo_init,
      NULL
    };

    dasher_action_keyboard_maemo_type = g_type_register_static(TYPE_DASHER_ACTION, "DasherActionKeyboardMaemo", &dasher_action_keyboard_maemo_info, static_cast < GTypeFlags > (0));
  }

  return dasher_action_keyboard_maemo_type;
}

static void dasher_action_keyboard_maemo_class_init(DasherActionKeyboardMaemoClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_action_keyboard_maemo_destroy;

  DasherActionClass *pDasherActionClass = (DasherActionClass *) pClass;
  pDasherActionClass->execute = dasher_action_keyboard_maemo_execute;
  pDasherActionClass->get_name = dasher_action_keyboard_maemo_get_name;
}

static void dasher_action_keyboard_maemo_init(DasherActionKeyboardMaemo *pDasherControl) {
  // Probably a waste of time - we don't really need any private data here
  pDasherControl->private_data = 0;
}

static void dasher_action_keyboard_maemo_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

DasherActionKeyboardMaemo *dasher_action_keyboard_maemo_new() {
  DasherActionKeyboardMaemo *pDasherControl;

  pDasherControl = (DasherActionKeyboardMaemo *)(g_object_new(dasher_action_keyboard_maemo_get_type(), NULL));

  return pDasherControl;
}

static gboolean dasher_action_keyboard_maemo_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx) { 

  const char *szData = dasher_editor_get_all_text(pEditor);

  // TODO: Make this work properly with UTF-8
  GdkEventClient sMyEvent;

  for(int i(0); i < strlen(szData); ++i) {
    sMyEvent.type = GDK_CLIENT_EVENT;
    sMyEvent.window = g_pRandomWindow;
    sMyEvent.send_event = true;
    sMyEvent.message_type = gdk_atom_intern("_HILDON_IM_INSERT_UTF8", true);
    sMyEvent.data_format = 8; // I know this is wrong...
    sMyEvent.data.l[0] = 0;
    sMyEvent.data.l[1] = szData[i];
    sMyEvent.data.l[2] = 0;
    sMyEvent.data.l[3] = 0;
    sMyEvent.data.l[4] = 0;

    gdk_event_send_client_message((GdkEvent *)(&sMyEvent), g_FRandomWindow); 
  }

  return true;
}

static const gchar *dasher_action_keyboard_maemo_get_name(DasherAction *pSelf) {
  return _("Enter Text");
}

#endif
