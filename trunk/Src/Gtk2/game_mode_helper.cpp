#include "game_mode_helper.h"

struct _GameModeHelperPrivate {
  GtkDasherControl *pControl;
  gchar *szTarget;
  gchar *szOutput;
  int iOutputLength;
};

typedef struct _GameModeHelperPrivate GameModeHelperPrivate;

// Private members
static void game_mode_helper_class_init(GameModeHelperClass * pClass);
static void game_mode_helper_init(GameModeHelper * pControl);
static void game_mode_helper_destroy(GObject * pObject);
static void game_mode_helper_get_next_string(GameModeHelper *pSelf);

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
  pPrivate->szOutput = 0;

  game_mode_helper_get_next_string(pDasherControl);

  return G_OBJECT(pDasherControl);
}

void game_mode_helper_get_next_string(GameModeHelper *pSelf) {
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

  if(pPrivate->szOutput)
    delete[] pPrivate->szOutput;

  pPrivate->szOutput = new gchar[1024];
  pPrivate->szOutput[0] = '\0';
  pPrivate->iOutputLength = 0;

  pPrivate->szTarget = "Hello world, this is a test";
  
  gtk_dasher_control_force_pause(GTK_DASHER_CONTROL(pPrivate->pControl));

  GtkMessageDialog *pDialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "New string: %s", pPrivate->szTarget));
  gtk_dialog_run(GTK_DIALOG(pDialog));
  gtk_widget_destroy(GTK_WIDGET(pDialog));

  gtk_dasher_control_add_game_mode_string(GTK_DASHER_CONTROL(pPrivate->pControl),
					  pPrivate->szTarget);

}

void game_mode_helper_output(GameModeHelper *pSelf, const gchar *szText) {
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

  // TODO: potential overflow problems
  strcat(pPrivate->szOutput, szText);
  pPrivate->iOutputLength += strlen(szText);

  if(!strcmp(pPrivate->szOutput, pPrivate->szTarget))
    game_mode_helper_get_next_string(pSelf);
}

void game_mode_helper_delete(GameModeHelper *pSelf, int iLength) {
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

  pPrivate->szOutput[pPrivate->iOutputLength - iLength] = '\0';
  pPrivate->iOutputLength -= iLength;
}
