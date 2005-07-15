#include "GtkDasherControl.h"
#include "DasherControl.h"

#include <iostream>

extern "C" {

struct _GtkDasherControlPrivate {
  CDasherControl *pControl;
};

typedef struct _GtkDasherControlPrivate GtkDasherControlPrivate;

static void gtk_dasher_control_class_init( GtkDasherControlClass* pClass );
static void gtk_dasher_control_init( GtkDasherControl *pControl );
static void gtk_dasher_control_destroy( GtkObject *pObject );

static GtkVBoxClass *pParentClass = NULL;

guint gtk_dasher_control_get_type() {
  static guint gtk_dasher_control_type = 0;
  
  if( !gtk_dasher_control_type ) {
    
    static const GtkTypeInfo gtk_dasher_control_info = {
      "GtkDasherControl",
      sizeof( GtkDasherControl ),
      sizeof( GtkDasherControlClass ),
      (GtkClassInitFunc) gtk_dasher_control_class_init,
      (GtkObjectInitFunc) gtk_dasher_control_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };

    gtk_dasher_control_type = gtk_type_unique( gtk_vbox_get_type(), &gtk_dasher_control_info );
  }

  return gtk_dasher_control_type;
}

static void gtk_dasher_control_class_init( GtkDasherControlClass *pClass ) {
  GtkObjectClass *pObjectClass = (GtkObjectClass*) pClass;

  pParentClass = GTK_VBOX_CLASS( gtk_type_class( GTK_TYPE_VBOX ) );

  pObjectClass->destroy = gtk_dasher_control_destroy;
}

static void gtk_dasher_control_init( GtkDasherControl *pDasherControl ) {
  GtkDasherControlPrivate* pPrivateData;

  pPrivateData = g_new0( GtkDasherControlPrivate, 1 );
  
  pDasherControl->private_data = pPrivateData;

  pPrivateData->pControl = new CDasherControl( &(pDasherControl->box) );
}

GtkWidget* gtk_dasher_control_new() {
  GtkDasherControl *pDasherControl;

  pDasherControl = GTK_DASHER_CONTROL( gtk_type_new( gtk_dasher_control_get_type() ) );
  
  return GTK_WIDGET( pDasherControl );
}

void gtk_dasher_control_destroy( GtkObject* pObject) {
  GtkDasherControl* pDasherControl;
  
  g_return_if_fail( pObject );
  g_return_if_fail( IS_GTK_DASHER_CONTROL( pObject ));
  
  pDasherControl = GTK_DASHER_CONTROL( pObject );
  
  delete ((GtkDasherControlPrivate *)(pDasherControl->private_data))->pControl;
  g_free( pDasherControl->private_data );
  
  if( GTK_OBJECT_CLASS( pParentClass )->destroy ) {
     std::cout << pParentClass << " " << GTK_OBJECT_CLASS(pParentClass)->destroy << std::endl;
     (*GTK_OBJECT_CLASS( pParentClass )->destroy )(pObject);
  }
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


}


