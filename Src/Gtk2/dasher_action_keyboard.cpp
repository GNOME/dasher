#include "config.h"
#include "dasher_action_keyboard.h"

#include <string.h>

struct _DasherActionKeyboardPrivate {
  IDasherBufferSet *pBufferSet;
};

typedef struct _DasherActionKeyboardPrivate DasherActionKeyboardPrivate;

static void dasher_action_keyboard_class_init(DasherActionKeyboardClass *pClass);
static void dasher_action_keyboard_init(DasherActionKeyboard *pActionKeyboard);
static void dasher_action_keyboard_destroy(GObject *pObject);
static gboolean dasher_action_keyboard_execute(DasherAction *pSelf, const gchar *szData);
static const gchar *dasher_action_keyboard_get_name(DasherAction *pSelf);

GType dasher_action_keyboard_get_type() {

  static GType dasher_action_keyboard_type = 0;

  if(!dasher_action_keyboard_type) {
    static const GTypeInfo dasher_action_keyboard_info = {
      sizeof(DasherActionKeyboardClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_action_keyboard_class_init,
      NULL,
      NULL,
      sizeof(DasherActionKeyboard),
      0,
      (GInstanceInitFunc) dasher_action_keyboard_init,
      NULL
    };

    dasher_action_keyboard_type = g_type_register_static(TYPE_DASHER_ACTION, "DasherActionKeyboard", &dasher_action_keyboard_info, static_cast < GTypeFlags > (0));
  }

  return dasher_action_keyboard_type;
}

static void dasher_action_keyboard_class_init(DasherActionKeyboardClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_action_keyboard_destroy;

  DasherActionClass *pDasherActionClass = (DasherActionClass *) pClass;
  pDasherActionClass->execute = dasher_action_keyboard_execute;
  pDasherActionClass->get_name = dasher_action_keyboard_get_name;
}

static void dasher_action_keyboard_init(DasherActionKeyboard *pDasherControl) {
  pDasherControl->private_data = new DasherActionKeyboardPrivate;
}

static void dasher_action_keyboard_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

DasherActionKeyboard *dasher_action_keyboard_new(IDasherBufferSet *pBufferSet) {
  DasherActionKeyboard *pDasherControl;
  pDasherControl = (DasherActionKeyboard *)(g_object_new(dasher_action_keyboard_get_type(), NULL));

  ((DasherActionKeyboardPrivate *)(pDasherControl->private_data))->pBufferSet = pBufferSet;

  return pDasherControl;
}

static gboolean dasher_action_keyboard_execute(DasherAction *pSelf, const gchar *szData) {
  DasherActionKeyboardPrivate *pPrivate = (DasherActionKeyboardPrivate *)(DASHER_ACTION_KEYBOARD(pSelf)->private_data);
   
  idasher_buffer_set_insert(pPrivate->pBufferSet, szData);
  return true;
}

static const gchar *dasher_action_keyboard_get_name(DasherAction *pSelf) {
  return "Enter Text";
}
