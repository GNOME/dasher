#include "dasher_action.h"

static void dasher_action_class_init(DasherActionClass *pClass);
static void dasher_action_init(DasherAction *pAction);
static void dasher_action_destroy(GObject *pObject);

GType dasher_action_get_type() {

  static GType dasher_action_type = 0;

  if(!dasher_action_type) {
    static const GTypeInfo dasher_action_info = {
      sizeof(DasherActionClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_action_class_init,
      NULL,
      NULL,
      sizeof(DasherAction),
      0,
      (GInstanceInitFunc) dasher_action_init,
      NULL
    };

    dasher_action_type = g_type_register_static(G_TYPE_OBJECT, "DasherAction", &dasher_action_info, static_cast < GTypeFlags > (0));
  }

  return dasher_action_type;
}

static void dasher_action_class_init(DasherActionClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_action_destroy;

  pClass->execute = 0;
}

static void dasher_action_init(DasherAction *pDasherControl) {
  // Probably a waste of time - we don't really need any private data here
  pDasherControl->private_data = 0;
}

static void dasher_action_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

DasherAction *dasher_action_new() {
  DasherAction *pDasherControl;

  pDasherControl = (DasherAction *)(g_object_new(dasher_action_get_type(), NULL));

  return pDasherControl;
}

gboolean dasher_action_execute(DasherAction *pSelf, const gchar *szData) {
  if(DASHER_ACTION_GET_CLASS(pSelf)->execute)
    return DASHER_ACTION_GET_CLASS(pSelf)->execute(pSelf, szData);
  else
    return false;
}

const gchar *dasher_action_get_name(DasherAction *pSelf) {
  if(DASHER_ACTION_GET_CLASS(pSelf)->get_name)
    return DASHER_ACTION_GET_CLASS(pSelf)->get_name(pSelf);
  else
    return 0;
}
