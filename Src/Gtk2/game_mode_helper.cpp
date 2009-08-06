#include "game_mode_helper.h"
#include "GameMessages.h"
#include "dasher_editor_internal.h"
#include <string>

struct _GameModeHelperPrivate {
  DasherEditorInternal* pEditor;
  GtkDasherControl *pControl;
  GtkWidget* pGameGroup;
  GtkLabel* pGameInfoLabel;
  GtkEntry* pScore;
  GtkEntry* pLevel;
  GtkWidget* pGameToggleButton;
  GtkWidget* pDemoToggleButton;
  GtkWidget* pNewSentence;
  GtkWidget* pNewButton;
  GtkWidget* pFullDemo;
  std::string* pstrTarget;
  std::string* pstrOutput;
  int iOutputLength;
};

typedef struct _GameModeHelperPrivate GameModeHelperPrivate;

// Private members
static void game_mode_helper_class_init(GameModeHelperClass * pClass);
static void game_mode_helper_init(GameModeHelper * pControl);
static void game_mode_helper_destroy(GObject * pObject);
//static void game_mode_helper_get_next_string(GameModeHelper *pSelf);
static void game_mode_helper_update_target_label(GameModeHelper *pSelf);
static void game_mode_helper_dialog_box(GameModeHelper* pSelf, const char* message);

extern "C" gboolean game_mode_helper_cb_gametoggle(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean game_mode_helper_cb_next_sentence(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean game_mode_helper_cb_demo(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean game_mode_helper_cb_fulldemo(GtkWidget *pWidget, gpointer pUserData);

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
  GameModeHelper *pHelper = GAME_MODE_HELPER(pObject);
  g_free(pHelper->private_data);

  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

GObject *game_mode_helper_new(GtkBuilder *pXML, void* pDasherEditor) {
  GameModeHelper *pHelper;
  pHelper = GAME_MODE_HELPER(g_object_new(game_mode_helper_get_type(), NULL));
  
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pHelper->private_data));

  pPrivate->pControl = GTK_DASHER_CONTROL(gtk_builder_get_object(pXML, "DasherControl"));
  //  pPrivate->szOutput = 0;

  pPrivate->pEditor = (DasherEditorInternal *)pDasherEditor;
  pPrivate->pGameGroup = GTK_WIDGET(gtk_builder_get_object(pXML, "game_group"));
  pPrivate->pGameInfoLabel = GTK_LABEL(gtk_builder_get_object(pXML, "game_info_label"));
  pPrivate->pGameToggleButton = GTK_WIDGET(gtk_builder_get_object(pXML, "tb_command_game"));
  pPrivate->pNewSentence = GTK_WIDGET(gtk_builder_get_object(pXML, "game_new_sentence"));
  pPrivate->pDemoToggleButton = GTK_WIDGET(gtk_builder_get_object(pXML, "demo_toggle"));
  pPrivate->pFullDemo = GTK_WIDGET(gtk_builder_get_object(pXML, "fulldemo"));
  pPrivate->pScore = GTK_ENTRY(gtk_builder_get_object(pXML, "score_box"));
  pPrivate->pLevel = GTK_ENTRY(gtk_builder_get_object(pXML, "level_box"));
  pPrivate->pstrTarget = new std::string;
  pPrivate->pstrOutput = new std::string;
  
  g_signal_connect(G_OBJECT(pPrivate->pNewSentence), "clicked", G_CALLBACK(game_mode_helper_cb_next_sentence), pHelper);
  g_signal_connect(G_OBJECT(pPrivate->pGameToggleButton), "toggled", G_CALLBACK(game_mode_helper_cb_gametoggle), pHelper);
  g_signal_connect(G_OBJECT(pPrivate->pDemoToggleButton), "toggled", G_CALLBACK(game_mode_helper_cb_demo), pHelper);
  g_signal_connect(G_OBJECT(pPrivate->pFullDemo), "activate", G_CALLBACK(game_mode_helper_cb_fulldemo), pHelper);
  //  game_mode_helper_get_next_string(pHelper);

 gtk_dasher_control_game_helperreg(pPrivate->pControl, (void *)pHelper);

 return G_OBJECT(pHelper);
}

extern "C" gboolean 
game_mode_helper_cb_next_sentence(GtkWidget *pWidget, gpointer pUserData) {
 GameModeHelper *pSelf = (GameModeHelper *)pUserData;
 GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

 gtk_dasher_control_game_messagein(pPrivate->pControl, Dasher::GameMode::GAME_MESSAGE_NEXT, NULL);
 
  return FALSE; // TODO: Scheck semantics of return value
}

extern "C" gboolean 
game_mode_helper_cb_demo(GtkWidget *pWidget, gpointer pUserData) {
 GameModeHelper *pSelf = (GameModeHelper *)pUserData;
 GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

 if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pWidget)))
   gtk_dasher_control_game_messagein(pPrivate->pControl, Dasher::GameMode::GAME_MESSAGE_DEMO_ON, NULL);
 else
   gtk_dasher_control_game_messagein(pPrivate->pControl, Dasher::GameMode::GAME_MESSAGE_DEMO_OFF, NULL);
 
  return FALSE; // TODO: Scheck semantics of return value
}

extern "C" gboolean 
game_mode_helper_cb_fulldemo(GtkWidget *pWidget, gpointer pUserData) {
 GameModeHelper *pSelf = (GameModeHelper *)pUserData;
 GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

 gtk_dasher_control_game_messagein(pPrivate->pControl, Dasher::GameMode::GAME_MESSAGE_FULL_DEMO, NULL);
  
  return FALSE; // TODO: Scheck semantics of return value
}

extern "C" gboolean 
game_mode_helper_cb_gametoggle(GtkWidget *pWidget, gpointer pUserData) {
 GameModeHelper *pSelf = (GameModeHelper *)pUserData;
 GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

 bool bTurnOnGameMode = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pWidget));

  if(!bTurnOnGameMode)
    {
      gtk_widget_hide(GTK_WIDGET(pPrivate->pGameGroup));
      gtk_widget_hide(GTK_WIDGET(pPrivate->pGameInfoLabel));
      gtk_dasher_control_game_messagein(pPrivate->pControl, Dasher::GameMode::GAME_MESSAGE_GAME_OFF, NULL);
    }
  else
    {
      gtk_widget_show(GTK_WIDGET(pPrivate->pGameGroup));
      gtk_widget_show(GTK_WIDGET(pPrivate->pGameInfoLabel));
      gtk_dasher_control_game_messagein(pPrivate->pControl, Dasher::GameMode::GAME_MESSAGE_GAME_ON, NULL);
    }

  // XXX PRLW: something like pPrivate->pEditor->clear() may be more apt
  // dasher_editor_internal_cleartext(pPrivate->pEditor);
  return FALSE; // TODO: Scheck semantics of return value
}

/*
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
*/

/*
void game_mode_helper_output(GameModeHelper *pSelf, const gchar *szText) {
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

  //  gtk_entry_set_text(pPrivate->pScore, szText);
  // TODO: potential overflow problems
  //  strcat(pPrivate->szOutput, szText);
  //  pPrivate->iOutputLength += strlen(szText);

  //  if(!strcmp(pPrivate->szOutput, pPrivate->szTarget))
  //    game_mode_helper_get_next_string(pSelf);
}
*/

/*
void game_mode_helper_delete(GameModeHelper *pSelf, int iLength) {
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

  //  pPrivate->szOutput[pPrivate->iOutputLength - iLength] = '\0';
  //  pPrivate->iOutputLength -= iLength;
}
*/

void game_mode_helper_message(GameModeHelper *pSelf, int message, const void * messagedata) {
  using namespace Dasher::GameMode;
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));
  const std::string* pStr;
  std::string strText ="<span background=\"purple\" foreground=\"white\">";
  switch(message) {
  case GAME_MESSAGE_SET_TARGET_STRING:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    *(pPrivate->pstrTarget) = (*pStr);
    gtk_misc_set_alignment(GTK_MISC(pPrivate->pGameInfoLabel), 0.0, 0.0);
    gtk_label_set_justify(pPrivate->pGameInfoLabel, GTK_JUSTIFY_CENTER);
    gtk_label_set_text(pPrivate->pGameInfoLabel, pPrivate->pstrTarget->c_str());
    break;
  case GAME_MESSAGE_DISPLAY_TEXT:
    gtk_misc_set_alignment(GTK_MISC(pPrivate->pGameInfoLabel), 0.5, 0.0);
    gtk_label_set_use_markup(pPrivate->pGameInfoLabel, true);
    gtk_label_set_justify(pPrivate->pGameInfoLabel, GTK_JUSTIFY_CENTER);
    //    std::string strText ="<span background=\"purple\">";
    strText+=(reinterpret_cast<const std::string*>(messagedata)->c_str());
    strText+="</span>";
    gtk_label_set_markup(pPrivate->pGameInfoLabel, strText.c_str());
    break;
  case GAME_MESSAGE_EDIT:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    *(pPrivate->pstrOutput) += (*pStr);
    game_mode_helper_update_target_label(pSelf);
    break;
  case GAME_MESSAGE_EDIT_DELETE:
    *(pPrivate->pstrOutput) = pPrivate->pstrOutput->substr( 0, pPrivate->pstrOutput->size() -
							    *reinterpret_cast<const int *>(messagedata));
    game_mode_helper_update_target_label(pSelf);
    break;
  case GAME_MESSAGE_SET_SCORE:
    gtk_entry_set_text(pPrivate->pScore, reinterpret_cast<const std::string*>(messagedata)->c_str());
    break;
  case GAME_MESSAGE_SET_LEVEL:
    gtk_entry_set_text(pPrivate->pLevel, reinterpret_cast<const std::string*>(messagedata)->c_str());
    break;
  case GAME_MESSAGE_CLEAR_BUFFER:
  // XXX PRLW: something like pPrivate->pEditor->clear() may be more apt
  //  dasher_editor_internal_cleartext(pPrivate->pEditor);
    pPrivate->pstrOutput->clear();
    break;
  case GAME_MESSAGE_HELP_MESSAGE:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    game_mode_helper_dialog_box(pSelf, pStr->c_str());
    break;
  }
}


void game_mode_helper_update_target_label(GameModeHelper *pSelf) {
  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));
  const std::string& strTarget = *(pPrivate->pstrTarget);
  const std::string& strOutput = *(pPrivate->pstrOutput);
  unsigned int i=0, ct=0; // i,j count chars. ct, co for utf8 symbols
  unsigned int j=0, co=0;
  while(i < strTarget.length() && j < strOutput.length())
    {
      std::string utf8TargetChar = std::string(1,strTarget[i]);
      std::string utf8OutputChar = std::string(1,strOutput[j]);
      if(strTarget[i] & 0x80)
	{    // Character is more than 1 byte long
	  int extras = 1;
	  for(unsigned int bit = 0x20; (strTarget[i] & bit) != 0; bit >>= 1)
	    extras++;

	  if(extras > 5) {}  // Malformed character
	  
	  while(extras-- > 0) 
	    {
	      utf8TargetChar += strTarget[++i];
	    }
	}
      if(strOutput[j] & 0x80)
	{    // Character is more than 1 byte long
	  int extras = 1;
	  for(unsigned int bit = 0x20; (strOutput[j] & bit) != 0; bit >>= 1)
	    extras++;
	  
	  if(extras > 5) {}  // Malformed character
	  
	  while(extras-- > 0) 
	    {
	      utf8OutputChar += strOutput[++j];
	    }
	}
      
      if(utf8TargetChar != utf8OutputChar) break;
      ct=++i;
      co=++j;
    }

  std::string labelStr = strTarget;
  labelStr.insert(ct, "</span>");
  labelStr.insert(0, "<span background=\"green\">");
  gtk_label_set_markup(pPrivate->pGameInfoLabel, labelStr.c_str());
}

void game_mode_helper_dialog_box(GameModeHelper* pSelf, const gchar* message)
{
  //  GameModeHelperPrivate *pPrivate((GameModeHelperPrivate *)(pSelf->private_data));

  GtkWidget* pDialog;
  pDialog = gtk_message_dialog_new(NULL,
				   GTK_DIALOG_MODAL,
				   GTK_MESSAGE_INFO,
				   GTK_BUTTONS_OK,
				   message);
  gtk_dialog_run (GTK_DIALOG (pDialog));
  gtk_widget_destroy (pDialog);

  //gtk_widget_show(GTK_WIDGET(pDialog));
 /* Destroy the dialog when the user responds to it (e.g. clicks a button) */
  // g_signal_connect_swapped (pDialog, "response",
  //                       G_CALLBACK (gtk_widget_destroy),
  //                       pDialog);

}
