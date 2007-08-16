#include "game_mode_helper.h"
#include "GameMessages.h"
#include "dasher_editor_internal.h"

struct _GameModeHelperPrivate {
  DasherEditorInternal* pEditor;
  GtkDasherControl *pControl;
  GtkWidget* pGameGroup;
  GtkLabel* pGameInfoLabel;
  GtkWidget* pGameToggleButton;
  GtkWidget* pDemoToggleButton;
  GtkWidget* pNewSentence;
  GtkWidget* pNewButton;
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

extern "C" gboolean game_mode_helper_cb_gametoggle(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean game_mode_helper_cb_next_sentence(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean game_mode_helper_cb_demo(GtkWidget *pWidget, gpointer pUserData);

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
 
static void game_mode_helper_init(GameModeHelper *pHelper) {
  GameModeHelperPrivate *pPrivateData;
  pPrivateData = g_new0(GameModeHelperPrivate, 1);

  pHelper->private_data = pPrivateData;
}

static void game_mode_helper_destroy(GObject *pObject) {
  GameModeHelper *pDasherControl = GAME_MODE_HELPER(pObject);
  g_free(pDasherControl->private_data);

  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

GObject *game_mode_helper_new(GladeXML *pGladeXML, void* pDasherEditor) {
  GameModeHelper *pHelper;
  pHelper = GAME_MODE_HELPER(g_object_new(game_mode_helper_get_type(), NULL));
  
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pHelper->private_data));

  pPrivate->pControl = GTK_DASHER_CONTROL(glade_xml_get_widget(pGladeXML, "DasherControl"));
  pPrivate->szOutput = 0;

  pPrivate->pEditor = (DasherEditorInternal *)pDasherEditor;
  pPrivate->pGameGroup = glade_xml_get_widget(pGladeXML, "game_group");
  pPrivate->pGameInfoLabel = GTK_LABEL(glade_xml_get_widget(pGladeXML, "game_info_label"));
  pPrivate->pGameToggleButton = glade_xml_get_widget(pGladeXML, "tb_command_game");
  pPrivate->pNewSentence = glade_xml_get_widget(pGladeXML, "game_new_sentence");
  pPrivate->pDemoToggleButton = glade_xml_get_widget(pGladeXML, "demo_toggle");
  pPrivate->pNewButton = glade_xml_get_widget(pGladeXML, "tb_command_new");
  
  g_signal_connect(G_OBJECT(pPrivate->pNewSentence), "clicked", G_CALLBACK(game_mode_helper_cb_next_sentence), pHelper);
  g_signal_connect(G_OBJECT(pPrivate->pGameToggleButton), "clicked", G_CALLBACK(game_mode_helper_cb_gametoggle), pHelper);
  g_signal_connect(G_OBJECT(pPrivate->pDemoToggleButton), "clicked", G_CALLBACK(game_mode_helper_cb_demo), pHelper);
  //  game_mode_helper_get_next_string(pHelper);

  return G_OBJECT(pHelper);
}

extern "C" gboolean 
game_mode_helper_cb_next_sentence(GtkWidget *pWidget, gpointer pUserData) {
 GameModeHelper *pSelf = (GameModeHelper *)pUserData;
 GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));



 gtk_dasher_control_game_messagein(pPrivate->pControl, GAME_MESSAGE_NEXT, NULL);
 
  return FALSE; // TODO: Scheck semantics of return value
}

extern "C" gboolean 
game_mode_helper_cb_demo(GtkWidget *pWidget, gpointer pUserData) {
 GameModeHelper *pSelf = (GameModeHelper *)pUserData;
 GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

 gtk_dasher_control_game_messagein(pPrivate->pControl, GAME_MESSAGE_DEMO, NULL);

  return FALSE; // TODO: Scheck semantics of return value
}

extern "C" gboolean 
game_mode_helper_cb_gametoggle(GtkWidget *pWidget, gpointer pUserData) {
 GameModeHelper *pSelf = (GameModeHelper *)pUserData;
 GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

 gtk_dasher_control_game_messagein(pPrivate->pControl, GAME_MESSAGE_REGHELPER, (void *)pSelf);

 bool bGameMode = gtk_dasher_control_get_parameter_bool(pPrivate->pControl, BP_GAME_MODE );
  if(bGameMode)
    {
      gtk_widget_hide(GTK_WIDGET(pPrivate->pGameGroup));
      gtk_widget_hide(GTK_WIDGET(pPrivate->pGameInfoLabel));
    }
  else
    {
      gtk_widget_show(GTK_WIDGET(pPrivate->pGameGroup));
      gtk_widget_show(GTK_WIDGET(pPrivate->pGameInfoLabel));
    }

  //  dasher_editor_internal_clear(pSelf, false);

  gtk_dasher_control_set_parameter_bool(pPrivate->pControl,  BP_GAME_MODE , !bGameMode);
  
  dasher_editor_internal_cleartext(pPrivate->pEditor);
  return FALSE; // TODO: Scheck semantics of return value
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
  //  strcat(pPrivate->szOutput, szText);
  //  pPrivate->iOutputLength += strlen(szText);

  //  if(!strcmp(pPrivate->szOutput, pPrivate->szTarget))
  //    game_mode_helper_get_next_string(pSelf);
}

void game_mode_helper_delete(GameModeHelper *pSelf, int iLength) {
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

  //  pPrivate->szOutput[pPrivate->iOutputLength - iLength] = '\0';
  //  pPrivate->iOutputLength -= iLength;
}

void game_mode_helper_message(GameModeHelper *pSelf, int message, const void * messagedata) {
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

  if(message == GAME_MESSAGE_SET_STRING)
    {
      gtk_misc_set_alignment(GTK_MISC(pPrivate->pGameInfoLabel), 0.0, 0.0);
      gtk_label_set_text(pPrivate->pGameInfoLabel, (const char*)messagedata);
      dasher_editor_internal_cleartext(pPrivate->pEditor);
    }
  if(message == GAME_MESSAGE_WELCOME)
    {
      gtk_misc_set_alignment(GTK_MISC(pPrivate->pGameInfoLabel), 0.5, 0.0);
      gtk_label_set_text(pPrivate->pGameInfoLabel, "Welcome to Dasher Game Mode!");
    }

}
