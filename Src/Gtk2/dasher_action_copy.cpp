#include "dasher_action_copy.h"

#include <string.h>

static void dasher_action_copy_class_init(DasherActionCopyClass *pClass);
static void dasher_action_copy_init(DasherActionCopy *pActionCopy);
static void dasher_action_copy_destroy(GObject *pObject);
static gboolean dasher_action_copy_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx);
static const gchar *dasher_action_copy_get_name(DasherAction *pSelf);

typedef struct _DasherActionCopyPrivate DasherActionCopyPrivate;

struct _DasherActionCopyPrivate {
  DasherEditor *pEditor;
};

GType dasher_action_copy_get_type() {

  static GType dasher_action_copy_type = 0;

  if(!dasher_action_copy_type) {
    static const GTypeInfo dasher_action_copy_info = {
      sizeof(DasherActionCopyClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_action_copy_class_init,
      NULL,
      NULL,
      sizeof(DasherActionCopy),
      0,
      (GInstanceInitFunc) dasher_action_copy_init,
      NULL
    };

    dasher_action_copy_type = g_type_register_static(TYPE_DASHER_ACTION, "DasherActionCopy", &dasher_action_copy_info, static_cast < GTypeFlags > (0));
  }

  return dasher_action_copy_type;
}

static void dasher_action_copy_class_init(DasherActionCopyClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_action_copy_destroy;

  DasherActionClass *pDasherActionClass = (DasherActionClass *) pClass;
  pDasherActionClass->execute = dasher_action_copy_execute;
  pDasherActionClass->get_name = dasher_action_copy_get_name;
}

static void dasher_action_copy_init(DasherActionCopy *pDasherControl) {
  pDasherControl->private_data = new DasherActionCopyPrivate;
}

static void dasher_action_copy_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

DasherActionCopy *dasher_action_copy_new(DasherEditor *pEditor) {
  DasherActionCopy *pDasherControl;
  pDasherControl = (DasherActionCopy *)(g_object_new(dasher_action_copy_get_type(), NULL));

  DasherActionCopyPrivate *pPrivate((DasherActionCopyPrivate *)pDasherControl->private_data);
  pPrivate->pEditor = pEditor;

  return pDasherControl;
}

static gboolean dasher_action_copy_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx) {
  DasherActionCopyPrivate *pPrivate((DasherActionCopyPrivate *)((DasherActionCopy *)pSelf)->private_data);
  dasher_editor_clipboard(pPrivate->pEditor, CLIPBOARD_COPYALL);
  return true;
}

static const gchar *dasher_action_copy_get_name(DasherAction *pSelf) {
  return "Copy All";
}
