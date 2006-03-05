#include "dasher_action_keyboard.h"

#include <cspi/spi.h>
#include <string.h>

static void dasher_action_keyboard_class_init(DasherActionKeyboardClass *pClass);
static void dasher_action_keyboard_init(DasherActionKeyboard *pActionKeyboard);
static void dasher_action_keyboard_destroy(GObject *pObject);
static gboolean dasher_action_keyboard_execute(DasherAction *pSelf, const gchar *szData);

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
}

static void dasher_action_keyboard_init(DasherActionKeyboard *pDasherControl) {
  // Probably a waste of time - we don't really need any private data here
  pDasherControl->private_data = 0;
}

static void dasher_action_keyboard_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

DasherActionKeyboard *dasher_action_keyboard_new() {
  DasherActionKeyboard *pDasherControl;

  pDasherControl = (DasherActionKeyboard *)(g_object_new(dasher_action_keyboard_get_type(), NULL));

  return pDasherControl;
}

static gboolean dasher_action_keyboard_execute(DasherAction *pSelf, const gchar *szData) {
  char *szNewText;
  szNewText = new char[strlen(szData) + 1];
  strcpy(szNewText, szData);
  
  SPI_generateKeyboardEvent(0, szNewText, SPI_KEY_STRING);
  
  delete[] szNewText;
  return true;
}
