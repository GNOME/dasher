#include "game_mode_helper.h"

struct _GameModeHelperPrivate {
  GtkDasherControl *pControl;
};

typedef struct _GameModeHelperPrivate GameModeHelperPrivate;

// Private members
static void game_mode_helper_class_init(GameModeHelperClass * pClass);
static void game_mode_helper_init(GameModeHelper * pControl);
static void game_mode_helper_destroy(GObject * pObject);

GType game_mode_helper_get_type() {

  static GType game_mode_helper_type = 0;

  if(!game_mode_helper_type) {

    static const GTypeInfo game_mode_helper_info = {
      sizeof(GameModeHelperClass),
      NULL,
      NULL,
      (GClassInitFunc) game_mode_helper_class_init,
      NULL,
      NULL,
      sizeof(GameModeHelper),
      0,
      (GInstanceInitFunc) game_mode_helper_init,
      NULL
    };

    game_mode_helper_type = g_type_register_static(GTK_TYPE_OBJECT, "GameModeHelper", &game_mode_helper_info, static_cast < GTypeFlags > (0));
  }

  return game_mode_helper_type;
}

static void game_mode_helper_class_init(GameModeHelperClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = game_mode_helper_destroy;
}
 
static void game_mode_helper_init(GameModeHelper *pDasherControl) {
  GameModeHelperPrivate *pPrivateData;
  pPrivateData = g_new0(GameModeHelperPrivate, 1);

  pDasherControl->private_data = pPrivateData;
}

static void game_mode_helper_destroy(GObject *pObject) {
  GameModeHelper *pDasherControl = GAME_MODE_HELPER(pObject);
  g_free(pDasherControl->private_data);

  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

GObject *game_mode_helper_new(GtkDasherControl *pControl) {
  GameModeHelper *pDasherControl;
  pDasherControl = GAME_MODE_HELPER(g_object_new(game_mode_helper_get_type(), NULL));
  
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pDasherControl->private_data));

  pPrivate->pControl = pControl;

  //
  gtk_dasher_control_add_game_mode_string(GTK_DASHER_CONTROL(pControl), "Hello world, this is a test");

  return G_OBJECT(pDasherControl);
}
