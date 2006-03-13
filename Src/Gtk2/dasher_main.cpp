#include "dasher_main.h"

struct _DasherMainPrivate {
  int iDummy;
};

typedef struct _DasherMainPrivate DasherMainPrivate;

// Private member functions

static void dasher_main_class_init(DasherMainClass *pClass);
static void dasher_main_init(DasherMain *pMain);
static void dasher_main_destroy(GObject *pObject);

GType dasher_main_get_type() {

  static GType dasher_main_type = 0;

  if(!dasher_main_type) {
    static const GTypeInfo dasher_main_info = {
      sizeof(DasherMainClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_main_class_init,
      NULL,
      NULL,
      sizeof(DasherMain),
      0,
      (GInstanceInitFunc) dasher_main_init,
      NULL
    };

    dasher_main_type = g_type_register_static(G_TYPE_OBJECT, "DasherMain", &dasher_main_info, static_cast < GTypeFlags > (0));
  }

  return dasher_main_type;
}

static void dasher_main_class_init(DasherMainClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_main_destroy;
}

static void dasher_main_init(DasherMain *pDasherControl) {
  pDasherControl->private_data = new DasherMainPrivate;
}

static void dasher_main_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

// Public methods

DasherMain *dasher_main_new(int argc, char **argv) {
  DasherMain *pDasherControl;
  pDasherControl = (DasherMain *)(g_object_new(dasher_main_get_type(), NULL));

  return pDasherControl;
}

