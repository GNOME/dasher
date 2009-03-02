#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "dasher_action_keyboard.h"
#include "dasher_editor.h"

#include <string.h>

struct _DasherActionKeyboardPrivate {
  IDasherBufferSet *pBufferSet;
};

typedef struct _DasherActionKeyboardPrivate DasherActionKeyboardPrivate;

#define DASHER_ACTION_KEYBOARD_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_DASHER_ACTION_KEYBOARD, DasherActionKeyboardPrivate))

G_DEFINE_TYPE(DasherActionKeyboard, dasher_action_keyboard, TYPE_DASHER_ACTION);

static gboolean dasher_action_keyboard_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx);
static const gchar *dasher_action_keyboard_get_name(DasherAction *pSelf);

static void 
dasher_action_keyboard_class_init(DasherActionKeyboardClass *pClass) {
  g_type_class_add_private(pClass, sizeof(DasherActionKeyboardPrivate));

  DasherActionClass *pDasherActionClass = (DasherActionClass *) pClass;
  pDasherActionClass->execute = dasher_action_keyboard_execute;
  pDasherActionClass->get_name = dasher_action_keyboard_get_name;
}

static void 
dasher_action_keyboard_init(DasherActionKeyboard *pDasherActionKeyboard) {
  DasherActionKeyboardPrivate *pPrivate = DASHER_ACTION_KEYBOARD_GET_PRIVATE(pDasherActionKeyboard);

  pPrivate->pBufferSet = NULL;
}

DasherActionKeyboard *
dasher_action_keyboard_new(IDasherBufferSet *pBufferSet) {
  DasherActionKeyboard *pDasherActionKeyboard;
  pDasherActionKeyboard = (DasherActionKeyboard *)(g_object_new(dasher_action_keyboard_get_type(), NULL));

  DasherActionKeyboardPrivate *pPrivate = DASHER_ACTION_KEYBOARD_GET_PRIVATE(pDasherActionKeyboard);
  pPrivate->pBufferSet = pBufferSet;

  return pDasherActionKeyboard;
}

static gboolean 
dasher_action_keyboard_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx) {
  DasherActionKeyboardPrivate *pPrivate = DASHER_ACTION_KEYBOARD_GET_PRIVATE(pSelf);
   
  if(pPrivate->pBufferSet)
    /* TODO: Fix offset here */
    idasher_buffer_set_insert(pPrivate->pBufferSet, dasher_editor_get_all_text(pEditor), 0);

  return true;
}

static const gchar *
dasher_action_keyboard_get_name(DasherAction *pSelf) {
  return _("Enter Text");
}
