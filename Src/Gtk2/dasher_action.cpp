#include "dasher_action.h"

struct _DasherActionPrivate {
  gboolean bActive;
};

typedef struct _DasherActionPrivate DasherActionPrivate;

#define DASHER_ACTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_DASHER_ACTION, DasherActionPrivate))

G_DEFINE_ABSTRACT_TYPE(DasherAction, dasher_action, G_TYPE_OBJECT);

/* Gobject boilerplate */
static void 
dasher_action_class_init(DasherActionClass *pClass) {
  g_type_class_add_private(pClass, sizeof(DasherActionPrivate));

  pClass->execute = 0;
  pClass->preview = 0;
  pClass->activate = 0;
  pClass->deactivate = 0;
  pClass->get_name = 0;
  pClass->get_sub_count = 0;
  pClass->get_sub_name = 0;
}

static void 
dasher_action_init(DasherAction *pDasherAction) {
  DasherActionPrivate *pDasherActionPrivate = DASHER_ACTION_GET_PRIVATE(pDasherAction);

  pDasherActionPrivate->bActive = false;
}

/* Public methods */
DasherAction *
dasher_action_new() {
  DasherAction *pDasherControl;

  pDasherControl = (DasherAction *)(g_object_new(dasher_action_get_type(), NULL));

  return pDasherControl;
}

gboolean 
dasher_action_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx) {
  // TODO: Need to make sure that the action is active first

  if(DASHER_ACTION_GET_CLASS(pSelf)->execute)
    return DASHER_ACTION_GET_CLASS(pSelf)->execute(pSelf, pEditor, iIdx);
  else
    return false;
}

gboolean 
dasher_action_preview(DasherAction *pSelf, DasherEditor *pEditor) {
  // TODO: Need to make sure that the action is active first

  if(DASHER_ACTION_GET_CLASS(pSelf)->preview)
    return DASHER_ACTION_GET_CLASS(pSelf)->preview(pSelf, pEditor);
  else
    return false;
}

const gchar *
dasher_action_get_name(DasherAction *pSelf) {
  if(DASHER_ACTION_GET_CLASS(pSelf)->get_name)
    return DASHER_ACTION_GET_CLASS(pSelf)->get_name(pSelf);
  else
    return 0;
}

int 
dasher_action_get_sub_count(DasherAction *pSelf) {
  if(DASHER_ACTION_GET_CLASS(pSelf)->get_sub_count)
    return DASHER_ACTION_GET_CLASS(pSelf)->get_sub_count(pSelf);
  else
    return 0;
}

const gchar *
dasher_action_get_sub_name(DasherAction *pSelf, int iIdx) {
  if(DASHER_ACTION_GET_CLASS(pSelf)->get_sub_name)
    return DASHER_ACTION_GET_CLASS(pSelf)->get_sub_name(pSelf, iIdx);
  else
    return NULL;
}

gboolean 
dasher_action_activate(DasherAction *pSelf) {
  DasherActionPrivate *pDasherActionPrivate = DASHER_ACTION_GET_PRIVATE(pSelf);

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

gboolean 
dasher_action_deactivate(DasherAction *pSelf) { 
  DasherActionPrivate *pDasherActionPrivate = DASHER_ACTION_GET_PRIVATE(pSelf);

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

gboolean 
dasher_action_get_active(DasherAction *pSelf) {
  DasherActionPrivate *pDasherActionPrivate = DASHER_ACTION_GET_PRIVATE(pSelf);

  return pDasherActionPrivate->bActive;
}
