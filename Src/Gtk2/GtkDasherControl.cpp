#include "GtkDasherControl.h"
#include "DasherControl.h"

#include <iostream>
#include <vector>
#include <string>

#include <gtk/gtkmarshal.h>

struct _GtkDasherControlPrivate {
  CDasherControl *pControl;
};

typedef struct _GtkDasherControlPrivate GtkDasherControlPrivate;

  // Signals that this control can emit

enum {
  DASHER_CHANGED,
  DASHER_START,
  DASHER_STOP,
  DASHER_EDIT_INSERT,
  DASHER_EDIT_DELETE,
  DASHER_CONTROL,
  DASHER_CONTEXT_REQUEST,
  SIGNAL_NUM
};

static void gtk_dasher_control_class_init(GtkDasherControlClass * pClass);
static void gtk_dasher_control_init(GtkDasherControl * pControl);
static void gtk_dasher_control_destroy(GObject * pObject);

static guint gtk_dasher_control_signals[SIGNAL_NUM] = { 0, 0, 0, 0 };

GType gtk_dasher_control_get_type() {

  static GType gtk_dasher_control_type = 0;

  if(!gtk_dasher_control_type) {

    static const GTypeInfo gtk_dasher_control_info = {
      sizeof(GtkDasherControlClass),
      NULL,
      NULL,
      (GClassInitFunc) gtk_dasher_control_class_init,
      NULL,
      NULL,
      sizeof(GtkDasherControl),
      0,
      (GInstanceInitFunc) gtk_dasher_control_init
    };

    gtk_dasher_control_type = g_type_register_static(GTK_TYPE_VBOX, "GtkDasherControl", &gtk_dasher_control_info, static_cast < GTypeFlags > (0));
  }

  return gtk_dasher_control_type;
}

static void gtk_dasher_control_class_init(GtkDasherControlClass *pClass) {

  GObjectClass *pObjectClass = (GObjectClass *) pClass;

  pObjectClass->finalize = gtk_dasher_control_destroy;

  //  pObjectClass->destroy = gtk_dasher_control_destroy;

  gtk_dasher_control_signals[DASHER_CHANGED] = g_signal_new("dasher_changed", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(GtkDasherControlClass, dasher_changed), NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

  gtk_dasher_control_signals[DASHER_START] = g_signal_new("dasher_start", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(GtkDasherControlClass, dasher_start), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gtk_dasher_control_signals[DASHER_STOP] = g_signal_new("dasher_stop", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(GtkDasherControlClass, dasher_stop), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gtk_dasher_control_signals[DASHER_EDIT_INSERT] = g_signal_new("dasher_edit_insert", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(GtkDasherControlClass, dasher_edit_insert), NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  gtk_dasher_control_signals[DASHER_EDIT_DELETE] = g_signal_new("dasher_edit_delete", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(GtkDasherControlClass, dasher_edit_delete), NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  gtk_dasher_control_signals[DASHER_CONTROL] = g_signal_new("dasher_control", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(GtkDasherControlClass, dasher_control), NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

  gtk_dasher_control_signals[DASHER_CHANGED] = g_signal_new("dasher_context_request", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(GtkDasherControlClass, dasher_context_request), NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

//   gtk_dasher_control_signals[DASHER_CONTROL] = g_signal_new("key_press_event", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(GtkDasherControlClass, key_press_event), NULL, NULL, gtk_marshal_BOOLEAN__POINTER, G_TYPE_BOOLEAN, 1, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);

//   gtk_dasher_control_signals[DASHER_CONTROL] = g_signal_new("key_release_event", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(GtkDasherControlClass, key_release_event), NULL, NULL, gtk_marshal_BOOLEAN__POINTER, G_TYPE_BOOLEAN, 1, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);

  pClass->dasher_changed = NULL;
  pClass->dasher_start = NULL;
  pClass->dasher_stop = NULL;
  pClass->dasher_edit_insert = NULL;
  pClass->dasher_edit_delete = NULL;
  pClass->dasher_control = NULL;
  pClass->dasher_context_request = NULL;
  //  pClass->key_press_event = gtk_dasher_control_default_key_press_handler;
  // pClass->key_release_event = gtk_dasher_control_default_key_release_handler;
}

static void gtk_dasher_control_init(GtkDasherControl *pDasherControl) {
  GtkDasherControlPrivate *pPrivateData;

  pPrivateData = g_new0(GtkDasherControlPrivate, 1);

  pDasherControl->private_data = pPrivateData;

  pPrivateData->pControl = new CDasherControl(&(pDasherControl->box), pDasherControl);

  g_signal_connect(G_OBJECT(pDasherControl), "key-press-event", G_CALLBACK(gtk_dasher_control_default_key_press_handler), pPrivateData->pControl);
  g_signal_connect(G_OBJECT(pDasherControl), "key-release-event", G_CALLBACK(gtk_dasher_control_default_key_release_handler), pPrivateData->pControl);

}

static void gtk_dasher_control_destroy(GObject *pObject) {
  GtkDasherControl *pDasherControl = GTK_DASHER_CONTROL(pObject);

  delete static_cast < GtkDasherControlPrivate * >(pDasherControl->private_data)->pControl;
  g_free(pDasherControl->private_data);

  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...

}

GtkWidget *gtk_dasher_control_new() {
  GtkDasherControl *pDasherControl;

  pDasherControl = GTK_DASHER_CONTROL(g_object_new(gtk_dasher_control_get_type(), NULL));

  return GTK_WIDGET(pDasherControl);
}

void gtk_dasher_control_set_parameter_bool(GtkDasherControl *pControl, int iParameter, bool bValue) {
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->SetBoolParameter(iParameter, bValue);
}

void gtk_dasher_control_set_parameter_long(GtkDasherControl *pControl, int iParameter, long lValue) {
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->SetLongParameter(iParameter, lValue);
}

void gtk_dasher_control_set_parameter_string(GtkDasherControl *pControl, int iParameter, const char *szValue) {
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->SetStringParameter(iParameter, szValue);
}

bool gtk_dasher_control_get_parameter_bool(GtkDasherControl *pControl, int iParameter) {
  return ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->GetBoolParameter(iParameter);
}

long gtk_dasher_control_get_parameter_long(GtkDasherControl *pControl, int iParameter) {
  return ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->GetLongParameter(iParameter);
}

void gtk_dasher_control_reset_parameter(GtkDasherControl *pControl, int iParameter) {
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->ResetParameter(iParameter);
}

const char *gtk_dasher_control_get_parameter_string(GtkDasherControl *pControl, int iParameter) {
  return (((GtkDasherControlPrivate *) (pControl->private_data))->pControl->GetStringParameter(iParameter)).c_str();
}

GArray *gtk_dasher_control_get_allowed_values(GtkDasherControl *pControl, int iParameter) {
  return (((GtkDasherControlPrivate *) (pControl->private_data))->pControl->GetAllowedValues(iParameter));
};

void gtk_dasher_control_train(GtkDasherControl *pControl, const gchar *szFilename) {
  return (((GtkDasherControlPrivate *) (pControl->private_data))->pControl->Train(szFilename));
};

void gtk_dasher_control_set_context(GtkDasherControl *pControl, const gchar *szContext) {
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->SetContext(szContext);
}

void gtk_dasher_control_invalidate_context(GtkDasherControl *pControl) {
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->InvalidateContext();
}

void gtk_dasher_control_register_node(GtkDasherControl *pControl, int iID, const gchar *szLabel, int iColour) {
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->RegisterNode(iID, szLabel, iColour);
}

void gtk_dasher_control_connect_node(GtkDasherControl *pControl, int iChild, int iParent, int iAfter) {
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->ConnectNode(iChild, iParent, iAfter);
}

void gtk_dasher_user_log_new_trial(GtkDasherControl * pControl) {
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->UserLogNewTrial();
}
void gtk_dasher_control_set_focus(GtkDasherControl * pControl){
  ((GtkDasherControlPrivate *) (pControl->private_data))->pControl->SetFocus();

}

gboolean gtk_dasher_control_default_key_press_handler(GtkDasherControl *pDasherControl, GdkEventKey *pEvent, gpointer data){
  static_cast<CDasherControl *>(data)->KeyPressEvent(pEvent);
}


gboolean gtk_dasher_control_default_key_release_handler(GtkDasherControl *pDasherControl, GdkEventKey *pEvent, gpointer data) {
 static_cast<CDasherControl *>(data)->KeyReleaseEvent(pEvent);
}
