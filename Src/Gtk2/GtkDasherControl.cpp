#include "GtkDasherControl.h"
#include "DasherControl.h"

#include <iostream>
#include <vector>
#include <string>

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
  SIGNAL_NUM
};

static void gtk_dasher_control_class_init( GtkDasherControlClass* pClass );
static void gtk_dasher_control_init( GtkDasherControl *pControl );
static void gtk_dasher_control_destroy( GtkObject *pObject );

static guint gtk_dasher_control_signals[SIGNAL_NUM] = { 0,0,0,0 };

static GtkVBoxClass *pParentClass = NULL;

GType gtk_dasher_control_get_type() {
  
  static GType gtk_dasher_control_type = 0;
  
  if( !gtk_dasher_control_type ) {
    
    static const GTypeInfo gtk_dasher_control_info = {
      sizeof( GtkDasherControlClass ),
      NULL,
      NULL,
      (GClassInitFunc) gtk_dasher_control_class_init,
      NULL,
      NULL,
      sizeof( GtkDasherControl ),
      0,
      (GInstanceInitFunc) gtk_dasher_control_init
    };

    gtk_dasher_control_type = g_type_register_static( GTK_TYPE_VBOX,
						      "GtkDasherControl",
						      &gtk_dasher_control_info,
						      static_cast<GTypeFlags>(0) );
  }

  return gtk_dasher_control_type;
}

static void gtk_dasher_control_class_init( GtkDasherControlClass *pClass ) {

  GtkObjectClass *pObjectClass = (GtkObjectClass*) pClass;

  pObjectClass->destroy = gtk_dasher_control_destroy;

  gtk_dasher_control_signals[DASHER_CHANGED] =
    g_signal_new ( "dasher_changed", 
		   G_TYPE_FROM_CLASS( pClass ),
		   static_cast<GSignalFlags>( G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION ),
		   G_STRUCT_OFFSET( GtkDasherControlClass, dasher_changed ),
		   NULL, NULL,
		   g_cclosure_marshal_VOID__INT,
		   G_TYPE_NONE,
		   1,
		   G_TYPE_INT );

  gtk_dasher_control_signals[DASHER_START] =
    g_signal_new ( "dasher_start", 
		   G_TYPE_FROM_CLASS( pClass ),
		   static_cast<GSignalFlags>( G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION ),
		   G_STRUCT_OFFSET( GtkDasherControlClass, dasher_start ),
		   NULL, NULL,
		   g_cclosure_marshal_VOID__VOID,
		   G_TYPE_NONE,
		   0 );

  gtk_dasher_control_signals[DASHER_STOP] =
    g_signal_new ( "dasher_stop", 
		   G_TYPE_FROM_CLASS( pClass ),
		   static_cast<GSignalFlags>( G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION ),
		   G_STRUCT_OFFSET( GtkDasherControlClass, dasher_stop ),
		   NULL, NULL,
		   g_cclosure_marshal_VOID__VOID,
		   G_TYPE_NONE,
		   0 );

  gtk_dasher_control_signals[DASHER_EDIT_INSERT] =
    g_signal_new ( "dasher_edit_insert", 
		   G_TYPE_FROM_CLASS( pClass ),
		   static_cast<GSignalFlags>( G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION ),
		   G_STRUCT_OFFSET( GtkDasherControlClass, dasher_edit_insert ),
		   NULL, NULL,
		   g_cclosure_marshal_VOID__STRING,
		   G_TYPE_NONE,
		   1,
		   G_TYPE_STRING );

  gtk_dasher_control_signals[DASHER_EDIT_DELETE] =
    g_signal_new ( "dasher_edit_delete", 
		   G_TYPE_FROM_CLASS( pClass ),
		   static_cast<GSignalFlags>( G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION ),
		   G_STRUCT_OFFSET( GtkDasherControlClass, dasher_edit_delete ),
		   NULL, NULL,
		   g_cclosure_marshal_VOID__STRING,
		   G_TYPE_NONE,
		   1,
		   G_TYPE_STRING );
		   
  pClass->dasher_changed = NULL;
  pClass->dasher_start = NULL;
  pClass->dasher_stop = NULL;
  pClass->dasher_edit_insert = NULL;
  pClass->dasher_edit_delete = NULL;

}

static void gtk_dasher_control_init( GtkDasherControl *pDasherControl ) {
  GtkDasherControlPrivate* pPrivateData;

  pPrivateData = g_new0( GtkDasherControlPrivate, 1 );
  
  pDasherControl->private_data = pPrivateData;

  pPrivateData->pControl = new CDasherControl( &(pDasherControl->box), pDasherControl );
}

GtkWidget* gtk_dasher_control_new() {
  GtkDasherControl *pDasherControl;

  pDasherControl = GTK_DASHER_CONTROL( g_object_new( gtk_dasher_control_get_type(), NULL ) );
  
  return GTK_WIDGET( pDasherControl );
}

void gtk_dasher_control_destroy( GtkObject* pObject) {
  GtkDasherControl* pDasherControl;
  
  g_return_if_fail( pObject );
  g_return_if_fail( IS_GTK_DASHER_CONTROL( pObject ));
  
  pDasherControl = GTK_DASHER_CONTROL( pObject );
  
  delete ((GtkDasherControlPrivate *)(pDasherControl->private_data))->pControl;
  g_free( pDasherControl->private_data );
}
  
  
void gtk_dasher_control_set_parameter_bool( GtkDasherControl *pControl, int iParameter, bool bValue ) {
  ((GtkDasherControlPrivate *)(pControl->private_data))->pControl->SetBoolParameter( iParameter, bValue );
}

void gtk_dasher_control_set_parameter_long(  GtkDasherControl *pControl, int iParameter, long lValue ) { 
  ((GtkDasherControlPrivate *)(pControl->private_data))->pControl->SetLongParameter( iParameter, lValue );
}

void gtk_dasher_control_set_parameter_string(  GtkDasherControl *pControl, int iParameter, const char *szValue ) { 
  ((GtkDasherControlPrivate *)(pControl->private_data))->pControl->SetStringParameter( iParameter, szValue );
}

bool gtk_dasher_control_get_parameter_bool(  GtkDasherControl *pControl, int iParameter ) {
  return ((GtkDasherControlPrivate *)(pControl->private_data))->pControl->GetBoolParameter( iParameter );
}

long gtk_dasher_control_get_parameter_long(  GtkDasherControl *pControl, int iParameter ) {
  return ((GtkDasherControlPrivate *)(pControl->private_data))->pControl->GetLongParameter( iParameter );
}

const char *gtk_dasher_control_get_parameter_string( GtkDasherControl *pControl,  int iParameter ) {
  return (((GtkDasherControlPrivate *)(pControl->private_data))->pControl->GetStringParameter( iParameter )).c_str();
}

GArray *gtk_dasher_control_get_allowed_values( GtkDasherControl *pControl, int iParameter ) {
  return (((GtkDasherControlPrivate *)(pControl->private_data))->pControl->GetAllowedValues( iParameter ));
};

void gtk_dasher_control_train( GtkDasherControl *pControl, const gchar *szFilename ) {
  return (((GtkDasherControlPrivate *)(pControl->private_data))->pControl->Train( szFilename ));
};
