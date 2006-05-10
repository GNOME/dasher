#include "dasher_action.h"

struct _DasherActionPrivate {
  gboolean bActive;
};

typedef struct _DasherActionPrivate DasherActionPrivate;

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
  pClass->activate = 0;
  pClass->deactivate = 0;
}

static void dasher_action_init(DasherAction *pDasherControl) {
  pDasherControl->private_data = new DasherActionPrivate;
  DasherActionPrivate *pDasherActionPrivate = (DasherActionPrivate *)(pDasherControl->private_data);

  pDasherActionPrivate->bActive = false;
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
  // TODO: Need to make sure that the action is active first

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

gboolean dasher_action_activate(DasherAction *pSelf) {
  DasherActionPrivate *pDasherActionPrivate = (DasherActionPrivate *)(((DasherAction *)pSelf)->private_data);

  if(pDasherActionPrivate->bActive)
    return true;

  if(DASHER_ACTION_GET_CLASS(pSelf)->activate) {
    pDasherActionPrivate->bActive = DASHER_ACTION_GET_CLASS(pSelf)->activate(pSelf);
    return pDasherActionPrivate->bActive;
  }
  else {
    pDasherActionPrivate->bActive = true;
    return true;
  }
}

gboolean dasher_action_deactivate(DasherAction *pSelf) { 
  DasherActionPrivate *pDasherActionPrivate = (DasherActionPrivate *)(((DasherAction *)pSelf)->private_data);

  if(!(pDasherActionPrivate->bActive))
    return true;

  if(DASHER_ACTION_GET_CLASS(pSelf)->deactivate) {
    pDasherActionPrivate->bActive = !DASHER_ACTION_GET_CLASS(pSelf)->deactivate(pSelf);
    return !(pDasherActionPrivate->bActive);
  }
  else {
    pDasherActionPrivate->bActive = false;
    return true;
  }
}

gboolean dasher_action_get_active(DasherAction *pSelf) {
  DasherActionPrivate *pDasherActionPrivate = (DasherActionPrivate *)(((DasherAction *)pSelf)->private_data);

  return pDasherActionPrivate->bActive;
}
