#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>

#include <glib/gi18n.h>
#ifdef HAVE_GIO
#include <gio/gio.h>
#endif
#include <gtk/gtk.h>

#ifdef WITH_MAEMO
#include "dasher_action_keyboard_maemo.h"
#else
#include "dasher_action_keyboard.h"
#endif

#ifndef WITH_MAEMO
#include "dasher_action_script.h"
#endif

#ifdef GNOME_SPEECH
#include "dasher_action_speech.h"
#endif 

#include "dasher_editor_internal.h"
#include "dasher_external_buffer.h"
#include "dasher_internal_buffer.h"
#include "dasher_lock_dialogue.h"
#include "dasher_main.h"
//#include "game_mode_helper.h"

// TODO: Maybe reimplement something along the lines of the following, which used to be in edit.cc

// void set_mark() {
//   GtkTextIter oBufferEnd;
//   GtkTextIter oBufferStart;
//   gtk_text_buffer_get_bounds( the_text_buffer, &oBufferStart, &oBufferEnd);
//   gtk_text_buffer_create_mark(the_text_buffer, "new_start", &oBufferEnd, true);
// }

// const gchar *get_new_text() {
//   GtkTextIter oNewStart;
//   GtkTextIter oNewEnd;
//   GtkTextIter oDummy;

//   gtk_text_buffer_get_bounds( the_text_buffer, &oDummy, &oNewEnd);
//   gtk_text_buffer_get_iter_at_mark( the_text_buffer, &oNewStart, gtk_text_buffer_get_mark(the_text_buffer, "new_start"));

//   return gtk_text_buffer_get_text( the_text_buffer, &oNewStart, &oNewEnd, false );
  
// }

// ---

#define ACTION_STATE_SHOW 1
#define ACTION_STATE_CONTROL 2
#define ACTION_STATE_AUTO 4

typedef struct _EditorAction EditorAction;

struct _EditorAction {
  DasherAction *pAction;
  EditorAction *pNext;
  EditorAction *pPrevious;
  gint iControlID;
  gint iID; // TODO: does this need to be separate from iControlID?
  gboolean bShow;
  gboolean bControl;
  gboolean bAuto;
  gint iNSub;
};

typedef struct _DasherEditorInternalPrivate DasherEditorInternalPrivate;

struct _DasherEditorInternalPrivate {
  DasherMain *pDasherMain;
  GtkTextView *pTextView;
  GtkTextBuffer *pBuffer;
  GtkVBox *pActionPane;
  GtkClipboard *pTextClipboard;
  GtkClipboard *pPrimarySelection;
  GtkTable *pGameGroup;
  GtkLabel *pGameInfoLabel;
  EditorAction *pActionRing;
  EditorAction *pActionIter;
  gboolean bActionIterStarted;
  gint iNextActionID;
  IDasherBufferSet *pBufferSet;
  IDasherBufferSet *pExternalBuffer;
  //  GameModeHelper *pGameModeHelper;
  GtkTextMark *pNewMark;
  DasherAppSettings *pAppSettings;
  gchar *szFilename;
  gboolean bFileModified; // TODO: Make this work properly, export to main for quit etc
};

#define DASHER_EDITOR_INTERNAL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_EDITOR_INTERNAL, DasherEditorInternalPrivate))

/* Signals */
enum {
  FILENAME_CHANGED,
  BUFFER_CHANGED,
  CONTEXT_CHANGED,
  SIGNAL_NUM
};

//static guint dasher_editor_internal_signals[SIGNAL_NUM];

static DasherEditorInternal *g_pEditor;

G_DEFINE_TYPE(DasherEditorInternal, dasher_editor_internal, DASHER_TYPE_EDITOR);

static void dasher_editor_internal_finalize(GObject *pObject);


static void dasher_editor_internal_handle_font(DasherEditor *pSelf, const gchar *szFont);

gboolean dasher_editor_internal_command(DasherEditor *pSelf, const gchar *szCommand);
void dasher_editor_internal_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GtkBuilder *pXML, const gchar *szFullPath);

/* Private methods */
static void dasher_editor_internal_select_all(DasherEditor *pSelf);
static void dasher_editor_internal_setup_actions(DasherEditor *pSelf);
static void dasher_editor_internal_add_action(DasherEditor *pSelf, DasherAction *pNewAction);
static EditorAction *dasher_editor_internal_get_action_by_id(DasherEditor *pSelf, int iID);
static void dasher_editor_internal_rebuild_action_pane(DasherEditor *pSelf);
//static void dasher_editor_internal_display_message(DasherEditor *pSelf, DasherMessageInfo *pMessageInfo);
static void dasher_editor_internal_check_activity(DasherEditor *pSelf, EditorAction *pAction);
static void dasher_editor_internal_action_save_state(DasherEditor *pSelf, EditorAction *pAction);

static void dasher_editor_internal_command_new(DasherEditor *pSelf);
static void dasher_editor_internal_command_open(DasherEditor *pSelf);
static void dasher_editor_internal_command_save(DasherEditor *pSelf, gboolean bPrompt, gboolean bAppend);

#ifdef HAVE_GIO
static void dasher_editor_internal_gvfs_print_error(DasherEditor *pSelf, GError *error, const char *myfilename);
static GFileOutputStream *append_or_replace_file(GFile *file, bool append, GError **error);
static gboolean dasher_editor_internal_gvfs_open_file(DasherEditor *pSelf, const char *filename, gchar ** buffer, gsize *size);
static gboolean dasher_editor_internal_gvfs_save_file(DasherEditor *pSelf, const char *filename, gchar * buffer, gsize length, bool append);
#else
static gboolean dasher_editor_internal_unix_vfs_open_file(DasherEditor *pSelf, const char *filename, gchar ** buffer, gsize *size);
static gboolean dasher_editor_internal_unix_vfs_save_file(DasherEditor *pSelf, const char *filename, gchar * buffer, gsize length, bool append);
#endif

static void dasher_editor_internal_set_filename(DasherEditor *pSelf, const gchar *szFilename);

// TODO: Should these be public?
static void dasher_editor_internal_convert(DasherEditor *pSelf);
static void dasher_editor_internal_protect(DasherEditor *pSelf);

static void dasher_editor_internal_new_buffer(DasherEditor *pSelf, const gchar *szFilename);

static void dasher_editor_internal_generate_filename(DasherEditor *pSelf);
static void dasher_editor_internal_open(DasherEditor *pSelf, const gchar *szFilename);
static bool dasher_editor_internal_save_as(DasherEditor *pSelf, const gchar *szFilename, bool bAppend);
static void dasher_editor_internal_create_buffer(DasherEditor *pSelf);
static void dasher_editor_internal_clear(DasherEditor *pSelf, gboolean bStore);
static void dasher_editor_internal_clipboard(DasherEditor *pSelf, clipboard_action act);

/* To be obsoleted by movement to GTK buffers */
void dasher_editor_internal_output(DasherEditor *pSelf, const gchar *szText, int iOffset);
void dasher_editor_internal_delete(DasherEditor *pSelf, int iLength, int iOffset);
const gchar *dasher_editor_internal_get_context(DasherEditor *pSelf, int iOffset, int iLength);
gint dasher_editor_internal_get_offset(DasherEditor *pSelf);

/* Events proagated from main */
void dasher_editor_internal_handle_stop(DasherEditor *pSelf);
void dasher_editor_internal_handle_start(DasherEditor *pSelf);
void dasher_editor_internal_handle_control(DasherEditor *pSelf, int iNodeID);

/* Action related methods - TODO: a lot of this should be moved to dasher_main (eg action on stop etc) - that way we get a better level of abstraction, and can incorporate commands from other modules too. Actions should only be externally visible as a list of string commands*/
void dasher_editor_internal_action_button(DasherEditor *pSelf, DasherAction *pAction);
void dasher_editor_internal_actions_start(DasherEditor *pSelf);
bool dasher_editor_internal_actions_more(DasherEditor *pSelf);
void dasher_editor_internal_actions_get_next(DasherEditor *pSelf, const gchar **szName, gint *iID, gboolean *bShow, gboolean *bControl, gboolean *bAuto);
void dasher_editor_internal_action_set_show(DasherEditor *pSelf, int iActionID, bool bValue);
void dasher_editor_internal_action_set_control(DasherEditor *pSelf, int iActionID, bool bValue);
void dasher_editor_internal_action_set_auto(DasherEditor *pSelf, int iActionID, bool bValue);

void dasher_editor_internal_grab_focus(DasherEditor *pSelf);

/* TODO: Tutorial editor should be a separate class */
//void dasher_editor_internal_start_tutorial(DasherEditor *pSelf);

/* Todo: possibly tidy up the need to have this public (quit in dasher_main possibly too connected) */
gboolean dasher_editor_internal_file_changed(DasherEditor *pSelf);
const gchar *dasher_editor_internal_get_filename(DasherEditor *pSelf);

const gchar *dasher_editor_internal_get_all_text(DasherEditor *pSelf);
const gchar *dasher_editor_internal_get_new_text(DasherEditor *pSelf);

static void dasher_editor_internal_handle_parameter_change(DasherEditor *pSelf, gint iParameter);


// Private methods not in class
extern "C" void delete_children_callback(GtkWidget *pWidget, gpointer pUserData);
extern "C" void main_window_realized(DasherMain *pMain, gpointer pUserData);
extern "C" void action_button_callback(GtkWidget *pWidget, gpointer pUserData);
extern "C" void context_changed_handler(GObject *pSource, gpointer pUserData);
extern "C" void buffer_changed_handler(GObject *pSource, gpointer pUserData);
extern "C" void handle_stop_event(GtkDasherControl *pDasherControl, gpointer data);
extern "C" void on_message(GtkDasherControl *pDasherControl, gpointer pMessageInfo, gpointer pUserData);
extern "C" void on_command(GtkDasherControl *pDasherControl, gchar *szCommand, gpointer pUserData);
extern "C" void handle_request_settings(GtkDasherControl * pDasherControl, gpointer data);
extern "C" void gtk2_edit_delete_callback(GtkDasherControl *pDasherControl, const gchar *szText, int iOffset, gpointer user_data);
extern "C" void gtk2_edit_output_callback(GtkDasherControl *pDasherControl, const gchar *szText, int iOffset, gpointer user_data);
extern "C" void convert_cb(GtkDasherControl *pDasherControl, gpointer pUserData);
extern "C" void protect_cb(GtkDasherControl *pDasherControl, gpointer pUserData);

static void 
dasher_editor_internal_class_init(DasherEditorInternalClass *pClass) {
  g_type_class_add_private(pClass, sizeof(DasherEditorInternalPrivate));

  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_editor_internal_finalize;

  DasherEditorClass *pParentClass = (DasherEditorClass *)pClass;
  pParentClass->initialise = dasher_editor_internal_initialise;
  pParentClass->command = dasher_editor_internal_command;
  pParentClass->output = dasher_editor_internal_output;
  pParentClass->delete_text = dasher_editor_internal_delete;
  pParentClass->get_context = dasher_editor_internal_get_context;
  pParentClass->get_offset = dasher_editor_internal_get_offset;
  pParentClass->handle_stop = dasher_editor_internal_handle_stop;
  pParentClass->handle_start = dasher_editor_internal_handle_start;
  pParentClass->handle_control = dasher_editor_internal_handle_control;
  pParentClass->action_button = dasher_editor_internal_action_button;
  pParentClass->actions_start = dasher_editor_internal_actions_start;
  pParentClass->actions_more = dasher_editor_internal_actions_more;
  pParentClass->actions_get_next = dasher_editor_internal_actions_get_next;
  pParentClass->action_set_show = dasher_editor_internal_action_set_show;
  pParentClass->action_set_control = dasher_editor_internal_action_set_control;
  pParentClass->action_set_auto = dasher_editor_internal_action_set_auto;
  pParentClass->grab_focus = dasher_editor_internal_grab_focus;
  pParentClass->file_changed = dasher_editor_internal_file_changed;
  pParentClass->get_filename = dasher_editor_internal_get_filename;
  pParentClass->get_all_text = dasher_editor_internal_get_all_text;
  pParentClass->get_new_text = dasher_editor_internal_get_new_text;
  pParentClass->handle_parameter_change = dasher_editor_internal_handle_parameter_change;
}

static void 
dasher_editor_internal_init(DasherEditorInternal *pSelf) {
  DasherEditorInternalPrivate *pPrivate =
                                   DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  pPrivate->pTextView = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_text_view_set_wrap_mode(pPrivate->pTextView, GTK_WRAP_WORD);
  pPrivate->pBuffer = gtk_text_view_get_buffer(pPrivate->pTextView);
  pPrivate->pBufferSet = NULL;
  pPrivate->pExternalBuffer = NULL;
  pPrivate->szFilename = NULL;
  pPrivate->pTextClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  pPrivate->pPrimarySelection = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  pPrivate->pActionRing = NULL;
  pPrivate->iNextActionID = 0;
  //  pPrivate->pGameModeHelper = NULL;
  GtkTextIter oStartIter;
  gtk_text_buffer_get_start_iter(pPrivate->pBuffer, &oStartIter);
  pPrivate->pNewMark =
       gtk_text_buffer_create_mark(pPrivate->pBuffer, NULL, &oStartIter, TRUE);
  pPrivate->bFileModified = FALSE;

  GtkWidget *pScrolledWindow = gtk_scrolled_window_new(NULL, NULL);

  gtk_container_add(GTK_CONTAINER(pScrolledWindow),
                    GTK_WIDGET(pPrivate->pTextView));

  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pScrolledWindow),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);

  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(pScrolledWindow),
                                      GTK_SHADOW_IN);

  gtk_box_pack_start(GTK_BOX(&(pSelf->parent.box)),
                     pScrolledWindow, true, true, 0);

  gtk_widget_show_all(GTK_WIDGET(&(pSelf->parent.box)));
}

static void 
dasher_editor_internal_finalize(GObject *pObject) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pObject);

  EditorAction *pCurrentAction = pPrivate->pActionRing;

  if(pCurrentAction) {
    bool bStarted = false;
    
    while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
      bStarted = true;
      dasher_action_deactivate(pCurrentAction->pAction);
      g_object_unref(G_OBJECT(pCurrentAction->pAction));
      pCurrentAction = pCurrentAction->pNext;
    }
  }

  if(pPrivate->pBufferSet)
    g_object_unref(G_OBJECT(pPrivate->pBufferSet));

  if(pPrivate->szFilename)
    g_free(pPrivate->szFilename);
}

/* Public methods */
DasherEditorInternal*
dasher_editor_internal_new(void)
{
  return
    DASHER_EDITOR_INTERNAL(g_object_new(DASHER_TYPE_EDITOR_INTERNAL, NULL));
}

void
dasher_editor_internal_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GtkBuilder *pXML, const gchar *szFullPath) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  pPrivate->pAppSettings = pAppSettings;
  pPrivate->pDasherMain = pDasherMain;

  dasher_editor_internal_handle_font(pSelf,
				     dasher_app_settings_get_string(pPrivate->pAppSettings, 
								    APP_SP_EDIT_FONT));
  
  GtkVBox *pActionPane = GTK_VBOX(gtk_builder_get_object(pXML, "vbox39"));


  pPrivate->pActionPane = pActionPane;

  // TODO: is this still needed?
  dasher_editor_internal_create_buffer(pSelf);

  dasher_editor_internal_setup_actions(pSelf);

  // TODO: see note in command_new method
  if(szFullPath)
    dasher_editor_internal_open(pSelf, szFullPath);
  else {
    dasher_editor_internal_generate_filename(pSelf);
    dasher_editor_internal_clear(pSelf, false);
  }
  
//  pPrivate->pGameModeHelper = GAME_MODE_HELPER(game_mode_helper_new(pXML, (void*)pSelf));
}

static void 
dasher_editor_internal_clipboard(DasherEditor *pSelf, clipboard_action act) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(pPrivate->pBuffer), start, 0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(pPrivate->pBuffer), end, -1);

  gchar *the_text = gtk_text_buffer_get_text(pPrivate->pBuffer, start, end, TRUE);

  switch (act) {
  case CLIPBOARD_CUT:
    gtk_text_buffer_cut_clipboard(pPrivate->pBuffer, pPrivate->pTextClipboard, TRUE);
    break;
  case CLIPBOARD_COPY:
    gtk_text_buffer_copy_clipboard(pPrivate->pBuffer, pPrivate->pTextClipboard);
    break;
  case CLIPBOARD_PASTE:
    gtk_text_buffer_paste_clipboard(pPrivate->pBuffer, pPrivate->pTextClipboard, NULL, TRUE);
    break;
  case CLIPBOARD_COPYALL:
    gtk_clipboard_set_text(pPrivate->pTextClipboard, the_text, strlen(the_text));
    gtk_clipboard_set_text(pPrivate->pPrimarySelection, the_text, strlen(the_text));

    break;
  case CLIPBOARD_SELECTALL:
    dasher_editor_internal_select_all(pSelf);
    break;
  case CLIPBOARD_CLEAR:
    gtk_text_buffer_set_text(pPrivate->pBuffer, "", 0);
    break;
  }
  g_free(the_text);

  delete start;
  delete end;
}

void dasher_editor_internal_cleartext(DasherEditorInternal *pSelf)
{
  dasher_editor_internal_clear((DasherEditor *)pSelf, true);
}

void 
dasher_editor_internal_handle_stop(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  
  // See if anything is set to auto:
  EditorAction *pCurrentAction = pPrivate->pActionRing;

  if(pCurrentAction) {
    bool bStarted = false;
    
    while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
      bStarted = true;
      if(pCurrentAction->bAuto)
	dasher_action_execute(pCurrentAction->pAction, DASHER_EDITOR(pSelf), -1); 
      pCurrentAction = pCurrentAction->pNext;
    }
  }
}

void 
dasher_editor_internal_handle_start(DasherEditor *pSelf) {
  // The edit box keeps track of where we started 

  // TODO: This should be filtered through the buffer, rather than directly to the edit box
  //  set_mark();
}

/* TODO: This is obsolete - sort this out when commands are reconsidered */
void 
dasher_editor_internal_handle_control(DasherEditor *pSelf, int iNodeID) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if(iNodeID == Dasher::CControlManager::CTL_USER + 1)
    dasher_editor_internal_clear(pSelf, false); // Clear node is a special case (it shouldn't be)
  else {
    EditorAction *pCurrentAction = pPrivate->pActionRing;
    bool bStarted = false;
    
    while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
      bStarted = true;
      if((iNodeID >= pCurrentAction->iControlID) && (iNodeID <= pCurrentAction->iControlID + pCurrentAction->iNSub)) {
	dasher_action_execute(pCurrentAction->pAction, DASHER_EDITOR(pSelf), iNodeID - pCurrentAction->iControlID - 1); 
	//	dasher_editor_internal_clear(pSelf, true); 
      }
      pCurrentAction = pCurrentAction->pNext;
    }
  }


  // TODO: Think about changing signals so we don't need to do this translation

  struct SControlMap {
    int iEvent;
    int iDir;
    int iDist;
    bool bDelete;
  };

  static struct SControlMap sMap[] = {
    {Dasher::CControlManager::CTL_MOVE_FORWARD_CHAR, EDIT_FORWARDS, EDIT_CHAR, false},
    {Dasher::CControlManager::CTL_MOVE_FORWARD_WORD, EDIT_FORWARDS, EDIT_WORD, false},
    {Dasher::CControlManager::CTL_MOVE_FORWARD_LINE, EDIT_FORWARDS, EDIT_LINE, false},
    {Dasher::CControlManager::CTL_MOVE_FORWARD_FILE, EDIT_FORWARDS, EDIT_FILE, false},
    {Dasher::CControlManager::CTL_MOVE_BACKWARD_CHAR, EDIT_BACKWARDS, EDIT_CHAR, false},
    {Dasher::CControlManager::CTL_MOVE_BACKWARD_WORD, EDIT_BACKWARDS, EDIT_WORD, false},
    {Dasher::CControlManager::CTL_MOVE_BACKWARD_LINE, EDIT_BACKWARDS, EDIT_LINE, false},
    {Dasher::CControlManager::CTL_MOVE_BACKWARD_FILE, EDIT_BACKWARDS, EDIT_FILE, false},
    {Dasher::CControlManager::CTL_DELETE_FORWARD_CHAR, EDIT_FORWARDS, EDIT_CHAR, true},
    {Dasher::CControlManager::CTL_DELETE_FORWARD_WORD, EDIT_FORWARDS, EDIT_WORD, true},
    {Dasher::CControlManager::CTL_DELETE_FORWARD_LINE, EDIT_FORWARDS, EDIT_LINE, true},
    {Dasher::CControlManager::CTL_DELETE_FORWARD_FILE, EDIT_FORWARDS, EDIT_FILE, true},
    {Dasher::CControlManager::CTL_DELETE_BACKWARD_CHAR, EDIT_BACKWARDS, EDIT_CHAR, true},
    {Dasher::CControlManager::CTL_DELETE_BACKWARD_WORD, EDIT_BACKWARDS, EDIT_WORD, true},
    {Dasher::CControlManager::CTL_DELETE_BACKWARD_LINE, EDIT_BACKWARDS, EDIT_LINE, true},
    {Dasher::CControlManager::CTL_DELETE_BACKWARD_FILE, EDIT_BACKWARDS, EDIT_FILE, true}
  };    

  if(pPrivate->pBufferSet) {
    for(unsigned int i(0); i < sizeof(sMap)/sizeof(struct SControlMap); ++i) {
      if(sMap[i].iEvent == iNodeID) {
	if(sMap[i].bDelete) 
	  idasher_buffer_set_edit_delete(pPrivate->pBufferSet, sMap[i].iDir, sMap[i].iDist);
	else
	  idasher_buffer_set_edit_move(pPrivate->pBufferSet, sMap[i].iDir, sMap[i].iDist);	
      }
    }
  }
}


void 
dasher_editor_internal_action_button(DasherEditor *pSelf, DasherAction *pAction) {
  if(pAction) {
    dasher_action_execute(pAction, DASHER_EDITOR(pSelf), -1); 
    dasher_editor_internal_clear(pSelf, true);
  }
  else { // Clear button
    dasher_editor_internal_clear(pSelf, false);
  }
}

static void 
dasher_editor_internal_clear(DasherEditor *pSelf, gboolean bStore) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if(IS_DASHER_INTERNAL_BUFFER(pPrivate->pBufferSet))
    dasher_internal_buffer_clear(DASHER_INTERNAL_BUFFER(pPrivate->pBufferSet));
}


void 
dasher_editor_internal_actions_start(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  pPrivate->bActionIterStarted = false;
  pPrivate->pActionIter = pPrivate->pActionRing;
}

bool 
dasher_editor_internal_actions_more(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

 return(!pPrivate->bActionIterStarted || (pPrivate->pActionIter != pPrivate->pActionRing));
}

void 
dasher_editor_internal_actions_get_next(DasherEditor *pSelf, const gchar **szName, gint *iID, gboolean *bShow, gboolean *bControl, gboolean *bAuto) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  
  *szName = dasher_action_get_name(pPrivate->pActionIter->pAction);
  *iID = pPrivate->pActionIter->iID;
  *bShow = pPrivate->pActionIter->bShow; 
  *bControl = pPrivate->pActionIter->bControl;
  *bAuto = pPrivate->pActionIter->bAuto;
  
  pPrivate->pActionIter = pPrivate->pActionIter->pNext;
  pPrivate->bActionIterStarted = true;
}

void 
dasher_editor_internal_action_set_show(DasherEditor *pSelf, int iActionID, bool bValue) {
  EditorAction *pAction;
  pAction = dasher_editor_internal_get_action_by_id(pSelf, iActionID);

  if(pAction) {
    pAction->bShow = bValue;
    dasher_editor_internal_check_activity(pSelf, pAction);
    dasher_editor_internal_rebuild_action_pane(pSelf);

    dasher_editor_internal_action_save_state(pSelf, pAction);
  }
}

void 
dasher_editor_internal_action_set_control(DasherEditor *pSelf, int iActionID, bool bValue) {
  // TODO: Need to actually change behaviour in resonse to these calls

  // TODO: Reimplement

//   EditorAction *pAction;
//   pAction = dasher_editor_internal_get_action_by_id(pSelf, iActionID);
  
//   if(pAction) {
//     pAction->bControl = bValue;
//     dasher_editor_internal_check_activity(pSelf, pAction);
//     if(bValue)
//       gtk_dasher_control_connect_node(GTK_DASHER_CONTROL(pDasherWidget), pAction->iControlID, Dasher::CControlManager::CTL_USER, -2);
//     else
//       gtk_dasher_control_disconnect_node(GTK_DASHER_CONTROL(pDasherWidget), pAction->iControlID, Dasher::CControlManager::CTL_USER);
    
//     dasher_editor_internal_action_save_state(pSelf, pAction);
//   }
}

void 
dasher_editor_internal_action_set_auto(DasherEditor *pSelf, int iActionID, bool bValue) { 
EditorAction *pAction;
  pAction = dasher_editor_internal_get_action_by_id(pSelf, iActionID);

  if(pAction) {
    pAction->bAuto = bValue;
    dasher_editor_internal_check_activity(pSelf, pAction);
    
    dasher_editor_internal_action_save_state(pSelf, pAction);
  }
}

void 
dasher_editor_internal_grab_focus(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pTextView)
    gtk_widget_grab_focus(GTK_WIDGET(pPrivate->pTextView));
}


static void 
dasher_editor_internal_create_buffer(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  /* Make an external buffer anyway, for keyboard command */
  /* TODO: Review this */
  if(!(pPrivate->pExternalBuffer))
    pPrivate->pExternalBuffer = IDASHER_BUFFER_SET(dasher_external_buffer_new());
    
  if(!(pPrivate->pBufferSet))
    pPrivate->pBufferSet = IDASHER_BUFFER_SET(dasher_internal_buffer_new(pPrivate->pTextView));

  // TODO: Fix this
  g_signal_connect(G_OBJECT(pPrivate->pBufferSet), "offset_changed", G_CALLBACK(context_changed_handler), pSelf);
  g_signal_connect(G_OBJECT(pPrivate->pBufferSet), "buffer_changed", G_CALLBACK(buffer_changed_handler), pSelf);
}

void 
dasher_editor_internal_output(DasherEditor *pSelf, const gchar *szText, int iOffset) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  // TODO: tidy this up, actionlookup by name, more flexible
  // definition of space
  // Python scripting?
  if(!strcmp(szText, " ")) {
    gboolean bActionIterStarted = false;
    EditorAction *pActionIter = pPrivate->pActionRing;
    
    while((pActionIter != pPrivate->pActionRing) || !bActionIterStarted) {
      bActionIterStarted = true;
      
      if(!strcmp(dasher_action_get_name(pActionIter->pAction), "Speak")) {
	dasher_action_preview(pActionIter->pAction, DASHER_EDITOR(pSelf));
      }
      
      pActionIter = pActionIter->pNext;
    }
  }

  if(pPrivate->pBufferSet)
    idasher_buffer_set_insert(pPrivate->pBufferSet, szText, iOffset);

//   if(pPrivate->pGameModeHelper)
//     game_mode_helper_output(pPrivate->pGameModeHelper, szText);

  pPrivate->bFileModified = TRUE;
}

void 
dasher_editor_internal_delete(DasherEditor *pSelf, int iLength, int iOffset) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pBufferSet)
    idasher_buffer_set_delete(pPrivate->pBufferSet, iLength, iOffset);

//   if(pPrivate->pGameModeHelper)
//     game_mode_helper_delete(pPrivate->pGameModeHelper, iLength);

  pPrivate->bFileModified = TRUE;
}

const gchar *
dasher_editor_internal_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
  // TODO: Check where this function is used
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  const gchar *szContext;

  if(pPrivate->pBufferSet)
    szContext = idasher_buffer_set_get_context(pPrivate->pBufferSet, iOffset, iLength);
  else
    szContext = "";

  // TODO: reimplement
//   if(szContext && (strlen(szContext) > 0))
//     gtk_dasher_control_set_context( GTK_DASHER_CONTROL(pDasherWidget), szContext );
  return szContext;
}

gint 
dasher_editor_internal_get_offset(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  return idasher_buffer_set_get_offset(pPrivate->pBufferSet);
}

static void 
dasher_editor_internal_generate_filename(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  
  gchar *szNewFilename = NULL;

  if( dasher_app_settings_get_bool(pPrivate->pAppSettings,  APP_BP_TIME_STAMP )) {
    // Build a filename based on the current time and date
    tm *t_struct;
    time_t ctime;
    char cwd[1000];
    char tbuffer[200];

    ctime = time(NULL);

    t_struct = localtime(&ctime);

    getcwd(cwd, 1000);
    snprintf(tbuffer, 200, "dasher-%04d%02d%02d-%02d%02d.txt", (t_struct->tm_year + 1900), (t_struct->tm_mon + 1), t_struct->tm_mday, t_struct->tm_hour, t_struct->tm_min);

    szNewFilename = g_build_path("/", cwd, tbuffer, NULL);
  }

  dasher_editor_internal_set_filename(pSelf, szNewFilename);

  g_free(szNewFilename);
}

static void 
dasher_editor_internal_open(DasherEditor *pSelf, const gchar *szFilename) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  gsize size;
  gchar *buffer;

#ifdef HAVE_GIO
  if(!dasher_editor_internal_gvfs_open_file(pSelf, szFilename, &buffer, &size)) {
    return;
  }
#else
  if(!dasher_editor_internal_unix_vfs_open_file(pSelf, szFilename, &buffer, &size)) {
    return;
  }
#endif

  // FIXME - REIMPLEMENT (shouldn't happen through core)
  //  dasher_clear();

  if(size != 0) {
    // Don't attempt to insert new text if the file is empty as it makes
    // GTK cry
    if(!g_utf8_validate(buffer, size, NULL)) { // PRLW: size as gssize = signed int
      // It's not UTF8, so we do the best we can...

      // If there are zero bytes in the file then we have a problem -
      // for now, just assert that we can't load these files.
      for(gsize i = 0; i < size; ++i)
	if(buffer[i] == 0) {
// 	  GtkWidget *pErrorBox = gtk_message_dialog_new(GTK_WINDOW(window), 
// 							GTK_DIALOG_MODAL, 
// 							GTK_MESSAGE_ERROR,
// 							GTK_BUTTONS_OK, 
// 							"Could not open the file \"%s\". Please note that Dasher cannot load files containing binary data, which may be the cause of this error.\n", 
// 							myfilename);
	  GtkWidget *pErrorBox = gtk_message_dialog_new(NULL, 
							GTK_DIALOG_MODAL, 
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_OK, 
							"Could not open the file \"%s\". Please note that Dasher cannot load files containing binary data, which may be the cause of this error.\n", 
							szFilename);
	  gtk_dialog_run(GTK_DIALOG(pErrorBox));
	  gtk_widget_destroy(pErrorBox);
	  return;
	}

      pPrivate->bFileModified = TRUE;

      gsize iNewSize;
      gchar *buffer2 = g_strdup(g_locale_to_utf8(buffer, size, NULL, &iNewSize, NULL));

      // TODO: This function probably needs more thought

      //      const gchar *pEnd;
      //gboolean bValid = g_utf8_validate(buffer2, -1, &pEnd);

      g_free(buffer);
      buffer = buffer2;
      size = iNewSize;
    }
    gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(pPrivate->pBuffer), buffer, size);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(pPrivate->pTextView), gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(pPrivate->pBuffer)));
  }

  dasher_editor_internal_set_filename(pSelf, szFilename);
}

static bool 
dasher_editor_internal_save_as(DasherEditor *pSelf, const gchar *szFilename, bool bAppend) { 
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  unsigned long long length;
  gchar *inbuffer, *outbuffer = NULL;
  //  gsize bytes_read, bytes_written;
  gsize bytes_written;
  //  GError *error = NULL;
  GtkTextIter *start, *end;
  //  GIConv cd;

  start = new GtkTextIter;
  end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(pPrivate->pBuffer), start, 0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(pPrivate->pBuffer), end, -1);

  inbuffer = gtk_text_iter_get_slice(start, end);

  //  g_message("String %s", inbuffer);

  //length = gtk_text_iter_get_offset(end) - gtk_text_iter_get_offset(start);
  //length = gtk_text_buffer_get_byte_count(GTK_TEXT_BUFFER(the_text_buffer));

  // I'm pretty certain that this is null terminated, but not 100%
  length = strlen(inbuffer);

  //  g_message("Length is %d", length);

  outbuffer = (char *)malloc((length + 1) * sizeof(gchar));
  memcpy((void *)outbuffer, (void *)inbuffer, length * sizeof(gchar));
  outbuffer[length] = 0;
  g_free(inbuffer);
  inbuffer = outbuffer;
  outbuffer = NULL;

//   switch (fileencoding) {
//   case Dasher::Opts::UserDefault:
//   case Dasher::Opts::AlphabetDefault:
//     //FIXME - need to call GetAlphabetType and do appropriate stuff regarding 
//     //the character set. Arguably we should always be saving in either UTF-8 or
//     //the user's locale (which may, of course, be UTF-8) because otherwise 
//     //we're going to read in rubbish, and we shouldn't be encouraging weird 
//     //codepage madness any further

//     //FIXME - error handling
//     outbuffer = g_locale_from_utf8(inbuffer, -1, &bytes_read, &bytes_written, &error);
//     if(outbuffer == NULL) {
//       // We can't represent the text in the current locale, so fall back to
//       // UTF-8
//       outbuffer = inbuffer;
//       bytes_written = length;
//     }
//   case Dasher::Opts::UTF8:
//     outbuffer = inbuffer;
//     bytes_written = length;
//     break;
//     // Does /anyone/ want to save text files in UTF16?
//     // (in any case, my opinions regarding encouragement of data formats with
//     // endianness damage are almost certainly unprintable)

//   case Dasher::Opts::UTF16LE:
//     cd = g_iconv_open("UTF16LE", "UTF8");
//     outbuffer = g_convert_with_iconv(inbuffer, -1, cd, &bytes_read, &bytes_written, &error);
//     break;
//   case Dasher::Opts::UTF16BE:
//     cd = g_iconv_open("UTF16BE", "UTF8");
//     outbuffer = g_convert_with_iconv(inbuffer, -1, cd, &bytes_read, &bytes_written, &error);
//     break;
//   default:
    outbuffer = inbuffer;
    bytes_written = length;
    //  }

#ifdef HAVE_GIO
  if(!dasher_editor_internal_gvfs_save_file(pSelf, szFilename, outbuffer, bytes_written, bAppend)) {
    return false;
  }
#else
  if(!dasher_editor_internal_unix_vfs_save_file(pSelf, szFilename, outbuffer, bytes_written, bAppend)) {
    return false;
  }
#endif

  pPrivate->bFileModified = FALSE;

  dasher_editor_internal_set_filename(pSelf, szFilename);

  return true;
}

// void 
// dasher_editor_internal_start_tutorial(DasherEditor *pSelf) {
//    DasherEditorInternalPrivate *pPrivate = (DasherEditorInternalPrivate *)(pSelf->private_data);

//   // TODO: reimplement
//   //  pPrivate->pGameModeHelper = GAME_MODE_HELPER(game_mode_helper_new(GTK_DASHER_CONTROL(pDasherWidget)));
// }

gboolean 
dasher_editor_internal_command(DasherEditor *pSelf, const gchar *szCommand) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if(!strcmp(szCommand, "action_new")) { //select_new_file
    dasher_editor_internal_command_new(pSelf);
    return TRUE;
  }
  
  if(!strcmp(szCommand, "action_open")) { //select open file
    dasher_editor_internal_command_open(pSelf);
    return TRUE;
  }
  
  if(!strcmp(szCommand, "action_save")) { //save_file
    dasher_editor_internal_command_save(pSelf, FALSE, FALSE);
    return TRUE;
  }

  if(!strcmp(szCommand, "action_saveas")) { // select_save_file_as
    dasher_editor_internal_command_save(pSelf, TRUE, FALSE);
    return TRUE;
  }

  if(!strcmp(szCommand, "action_append")) { // select_append_file
    dasher_editor_internal_command_save(pSelf, TRUE, TRUE);
    return TRUE;
  }

  if(!strcmp(szCommand, "action_cut")) { // clipboard_cut
    dasher_editor_internal_clipboard(pSelf, CLIPBOARD_CUT);
    return TRUE;
  }

  if(!strcmp(szCommand, "action_copy")) { // clipboard_copy
    dasher_editor_internal_clipboard(pSelf, CLIPBOARD_COPY);
    return TRUE;
  }

  if(!strcmp(szCommand, "action_copyall")) { // clipboard_copyall 
    dasher_editor_internal_clipboard(pSelf, CLIPBOARD_COPYALL);
    return TRUE;
  }

  if(!strcmp(szCommand, "action_paste")) { // clipboard_paste
    dasher_editor_internal_clipboard(pSelf, CLIPBOARD_PASTE);
    return TRUE;
  }
 
  // TODO: This isn't actually accessible from anywhere
  if(!strcmp(szCommand, "action_selectall")) { // clipboard_paste
    dasher_editor_internal_clipboard(pSelf, CLIPBOARD_SELECTALL);
    return TRUE;
  }
 
  /* TODO: We need a rethink here */
  const gchar *szForwardCommand = NULL;
  gint iSubCommand = 0;

  if(!strcmp(szCommand, "speakall")) {
    szForwardCommand = "Speak";
    iSubCommand = 0;
  }
  else if(!strcmp(szCommand, "speaklast")) {
    szForwardCommand = "Speak";
    iSubCommand = 1;
  }
  else if(!strcmp(szCommand, "speakrepeat")) {
    szForwardCommand = "Speak";
    iSubCommand = 2;
  }

  if(szForwardCommand) {
    gboolean bActionIterStarted = false;
    EditorAction *pActionIter = pPrivate->pActionRing;
    
    while((pActionIter != pPrivate->pActionRing) || !bActionIterStarted) {
      bActionIterStarted = true;
      
      if(!strcmp(dasher_action_get_name(pActionIter->pAction), szForwardCommand)) {
	dasher_action_execute(pActionIter->pAction, DASHER_EDITOR(pSelf), iSubCommand);
	return TRUE;
      }
      
      pActionIter = pActionIter->pNext;
    }
    return TRUE;
  }

  return FALSE;
}


void 
dasher_editor_internal_handle_font(DasherEditor *pSelf, const gchar *szFont) {
  if(strcmp(szFont, "")) {
    DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
    
    PangoFontDescription *pFD = pango_font_description_from_string(szFont);
    gtk_widget_modify_font(GTK_WIDGET(pPrivate->pTextView), pFD);
  }
}


gboolean 
dasher_editor_internal_file_changed(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  return pPrivate->bFileModified;
}

const gchar *
dasher_editor_internal_get_filename(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  return pPrivate->szFilename;
}

// TODO: We shouldn't need to know about the buffer here - make this a method of the buffer set
const gchar *
dasher_editor_internal_get_all_text(DasherEditor *pSelf) { 
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  GtkTextIter oStart;
  GtkTextIter oEnd;

  gtk_text_buffer_get_start_iter(pPrivate->pBuffer, &oStart);
  gtk_text_buffer_get_end_iter(pPrivate->pBuffer, &oEnd);

  pPrivate->pNewMark = gtk_text_buffer_create_mark(pPrivate->pBuffer, NULL, &oEnd, TRUE);

  return gtk_text_buffer_get_text(pPrivate->pBuffer, &oStart, &oEnd, false );
}

const gchar *
dasher_editor_internal_get_new_text(DasherEditor *pSelf) { 
  // TODO: Implement this properly
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  GtkTextIter oStart;
  GtkTextIter oEnd;

  gtk_text_buffer_get_end_iter(pPrivate->pBuffer, &oEnd);
  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &oStart, pPrivate->pNewMark);

  const gchar *szRetVal = gtk_text_buffer_get_text(pPrivate->pBuffer, &oStart, &oEnd, false );

  pPrivate->pNewMark = gtk_text_buffer_create_mark(pPrivate->pBuffer, NULL, &oEnd, TRUE);

  return szRetVal;
}


/* Private methods */
static void 
dasher_editor_internal_select_all(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(pPrivate->pBuffer), start, 0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(pPrivate->pBuffer), end, -1);

  GtkTextMark *selection = gtk_text_buffer_get_mark(pPrivate->pBuffer, "selection_bound");
  GtkTextMark *cursor = gtk_text_buffer_get_mark(pPrivate->pBuffer, "insert");

  gtk_text_buffer_move_mark(pPrivate->pBuffer, selection, start);
  gtk_text_buffer_move_mark(pPrivate->pBuffer, cursor, end);

  delete start;
  delete end;
}

static void 
dasher_editor_internal_setup_actions(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  
  // TODO: Activate and deactivate methods for actions
  // TODO: Clear shouldn't be a special case (include support for false in clear method)

#ifdef GNOME_SPEECH
  dasher_editor_internal_add_action(pSelf, DASHER_ACTION(dasher_action_speech_new()));
#endif

  dasher_editor_internal_add_action(pSelf, DASHER_ACTION(dasher_action_keyboard_new(pPrivate->pExternalBuffer)));

#ifdef WITH_MAEMO
  dasher_editor_internal_add_action(pSelf, DASHER_ACTION(dasher_action_keyboard_maemo_new()));
#else
  //  dasher_editor_internal_add_action(pSelf, DASHER_ACTION(dasher_action_copy_new(pSelf)));

  GDir *pDirectory;
  G_CONST_RETURN gchar *szFilename;


  gchar *szUserScriptDir = new gchar[strlen(dasher_app_settings_get_string(pPrivate->pAppSettings, SP_USER_LOC))+9];
  strcpy(szUserScriptDir, dasher_app_settings_get_string(pPrivate->pAppSettings, SP_USER_LOC));
  strcat(szUserScriptDir, "scripts/");

  pDirectory = g_dir_open(szUserScriptDir, 0, NULL);

  if(pDirectory) {
    while((szFilename = g_dir_read_name(pDirectory))) {
      dasher_editor_internal_add_action(pSelf, DASHER_ACTION(dasher_action_script_new(szUserScriptDir, szFilename)));
    }
    
    g_dir_close(pDirectory);
  }
  
  delete[] szUserScriptDir;

  gchar *szSystemScriptDir = new gchar[strlen(dasher_app_settings_get_string(pPrivate->pAppSettings, SP_SYSTEM_LOC))+9];
  strcpy(szSystemScriptDir, dasher_app_settings_get_string(pPrivate->pAppSettings, SP_SYSTEM_LOC));
  strcat(szSystemScriptDir, "scripts/");

  pDirectory = g_dir_open(szSystemScriptDir, 0, NULL);

  if(pDirectory) {
    while((szFilename = g_dir_read_name(pDirectory))) {
      dasher_editor_internal_add_action(pSelf, DASHER_ACTION(dasher_action_script_new(szSystemScriptDir, szFilename)));
    }
    
    g_dir_close(pDirectory);
  }

  delete[] szSystemScriptDir;
#endif

  // TODO: Reimplement

//   // TODO: This doesn't get re-called if the preferences change

//   gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER, "Actions", -1 );
//   gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER, Dasher::CControlManager::CTL_ROOT, -2);
//   int iControlOffset(1);

//   gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset, "Clear", -1 );
//   gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset, Dasher::CControlManager::CTL_USER, -2);
//   gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), -1, Dasher::CControlManager::CTL_USER + iControlOffset, -2);
//   ++iControlOffset;

//   EditorAction *pCurrentAction = pPrivate->pActionRing;
//   bool bStarted = false;

//   while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
//     bStarted = true;

//     if(pCurrentAction->bControl) {
//       gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset, dasher_action_get_name(pCurrentAction->pAction), -1 );
//       gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset, Dasher::CControlManager::CTL_USER, -2);
      
//       int iNSub(dasher_action_get_sub_count(pCurrentAction->pAction));
      
//       if(iNSub == 0) {
// 	gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), -1, Dasher::CControlManager::CTL_USER + iControlOffset, -2);
//       }
//       else {
// 	for(int i(0); i < iNSub; ++i) {
// 	  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset + i + 1, dasher_action_get_sub_name(pCurrentAction->pAction, i), -1 );
// 	  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset + i + 1, Dasher::CControlManager::CTL_USER + iControlOffset, -2);
// 	  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), -1, Dasher::CControlManager::CTL_USER + iControlOffset + i + 1, -2);
// 	}
//       }
      
//       pCurrentAction->iControlID = Dasher::CControlManager::CTL_USER + iControlOffset;
//       pCurrentAction->iNSub = iNSub;
//       iControlOffset += iNSub + 1;
//     }
      
//     pCurrentAction = pCurrentAction->pNext;
//   }

#ifndef WITH_MAEMOFULLSCREEN
  //  dasher_editor_internal_rebuild_action_pane(pSelf);
#endif
}

static void 
dasher_editor_internal_add_action(DasherEditor *pSelf, DasherAction *pNewAction) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  EditorAction *pNewEditorAction = new EditorAction;
  pNewEditorAction->pAction = pNewAction;
  pNewEditorAction->iID = pPrivate->iNextActionID;
  ++pPrivate->iNextActionID;

  gchar szRegistryName[256];
  strncpy(szRegistryName, "Action_", 256);
  strncat(szRegistryName, dasher_action_get_name(pNewEditorAction->pAction), 255 - strlen(szRegistryName));

  for(unsigned int i(0); i < strlen(szRegistryName); ++i)
    if(szRegistryName[i] == ' ')
      szRegistryName[i] = '_';

  gint iState;

  if(!dasher_app_settings_get_free_long(pPrivate->pAppSettings, szRegistryName, iState)) {
    if(!strcmp(dasher_action_get_name(pNewEditorAction->pAction), "Speak"))
      iState = 0;
    else
      iState = ACTION_STATE_SHOW | ACTION_STATE_CONTROL;

    dasher_app_settings_set_free_long(pPrivate->pAppSettings, szRegistryName, iState);
  }

  pNewEditorAction->bShow = iState & ACTION_STATE_SHOW;
  pNewEditorAction->bControl = iState & ACTION_STATE_CONTROL;
  pNewEditorAction->bAuto = iState & ACTION_STATE_AUTO;

  dasher_editor_internal_check_activity(pSelf, pNewEditorAction);

  if(pPrivate->pActionRing) {
    pNewEditorAction->pNext = pPrivate->pActionRing;
    pNewEditorAction->pPrevious = pPrivate->pActionRing->pPrevious;
    pPrivate->pActionRing->pPrevious->pNext = pNewEditorAction;
    pPrivate->pActionRing->pPrevious = pNewEditorAction;
  }
  else {
    pNewEditorAction->pNext = pNewEditorAction;
    pNewEditorAction->pPrevious = pNewEditorAction;
  }

  pPrivate->pActionRing = pNewEditorAction;
  
  // TODO: Reimplement
//   if(iState & ACTION_STATE_SHOW)
//     gtk_dasher_control_add_action_button(GTK_DASHER_CONTROL(pDasherWidget), dasher_action_get_name(pNewEditorAction->pAction));
}

static EditorAction *
dasher_editor_internal_get_action_by_id(DasherEditor *pSelf, int iID){
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  EditorAction *pCurrentAction = pPrivate->pActionRing;
  bool bStarted = false;
  
  while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
    bStarted = true;
    if(pCurrentAction->iID == iID)
      return pCurrentAction;
    pCurrentAction = pCurrentAction->pNext;
  }
  
  return 0;
}

static void 
dasher_editor_internal_rebuild_action_pane(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  
  // Delete any existing widgets
  gtk_container_foreach(GTK_CONTAINER(pPrivate->pActionPane), delete_children_callback, 0);

  // Add the cancel button
  GtkButton *pNewButton = GTK_BUTTON(gtk_button_new_with_label("Clear"));
  gtk_widget_show(GTK_WIDGET(pNewButton));

  void **pUserData = new void *[2];
  pUserData[0] = (void *)pSelf;
  pUserData[1] = 0;
  
  g_signal_connect(G_OBJECT(pNewButton), "clicked", G_CALLBACK(action_button_callback), pUserData);
#ifdef WITH_MAEMO
  // For Maemo we want the packing to expand
  gtk_box_pack_start(GTK_BOX(pPrivate->pActionPane), GTK_WIDGET(pNewButton), true, true, 0);
#else
  gtk_box_pack_start(GTK_BOX(pPrivate->pActionPane), GTK_WIDGET(pNewButton), false, false, 0);
#endif
 

  EditorAction *pCurrentAction = pPrivate->pActionRing;
  bool bStarted = false;

  while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
    bStarted = true;
    if(pCurrentAction->bShow) {
      GtkButton *pNewButton = GTK_BUTTON(gtk_button_new_with_label(dasher_action_get_name(pCurrentAction->pAction)));
      gtk_widget_show(GTK_WIDGET(pNewButton));
      
      pUserData = new void *[2];
      pUserData[0] = (void *)pSelf;
      pUserData[1] = (void *)(pCurrentAction->pAction);
      
      g_signal_connect(G_OBJECT(pNewButton), "clicked", G_CALLBACK(action_button_callback), pUserData);
#ifdef WITH_MAEMO
      // For Maemo we want the packing to expand
      gtk_box_pack_start(GTK_BOX(pPrivate->pActionPane), GTK_WIDGET(pNewButton), true, true, 0);
#else
      gtk_box_pack_start(GTK_BOX(pPrivate->pActionPane), GTK_WIDGET(pNewButton), false, false, 0);
#endif
    }
    pCurrentAction = pCurrentAction->pNext;
  }
}

// TODO: This shouldn't be a part of the editor
//static void 
//dasher_editor_internal_display_message(DasherEditor *pSelf, DasherMessageInfo *pMessageInfo) {
//  GtkMessageDialog *pDialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, pMessageInfo->szMessage));
//  gtk_dialog_run(GTK_DIALOG(pDialog));
//  gtk_widget_destroy(GTK_WIDGET(pDialog));
//}

static void 
dasher_editor_internal_check_activity(DasherEditor *pSelf, EditorAction *pAction) {
  gboolean bNeedActive(pAction->bShow || pAction->bControl || pAction->bAuto);
  gboolean bActive(dasher_action_get_active(pAction->pAction));

  if(bNeedActive && !bActive)
    dasher_action_activate(pAction->pAction);
  else if(!bNeedActive && bActive)
    dasher_action_deactivate(pAction->pAction);
}

static void 
dasher_editor_internal_action_save_state(DasherEditor *pSelf, EditorAction *pAction) {
  gchar szRegistryName[256];
  strncpy(szRegistryName, "Action_", 256);
  strncat(szRegistryName, dasher_action_get_name(pAction->pAction), 255 - strlen(szRegistryName));

  for(unsigned int i(0); i < strlen(szRegistryName); ++i)
    if(szRegistryName[i] == ' ')
      szRegistryName[i] = '_';

  gint iState = 0;

  if(pAction->bShow)
    iState += ACTION_STATE_SHOW;
  
  if(pAction->bControl)
    iState += ACTION_STATE_CONTROL;

  if(pAction->bAuto)
    iState += ACTION_STATE_AUTO;

  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  dasher_app_settings_set_free_long(pPrivate->pAppSettings, szRegistryName, iState);
}

static void 
dasher_editor_internal_command_new(DasherEditor *pSelf) {
  dasher_editor_internal_new_buffer(pSelf, NULL);
}

static void 
dasher_editor_internal_command_open(DasherEditor *pSelf) { 
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  GtkWidget *pTopLevel = gtk_widget_get_toplevel(GTK_WIDGET(pPrivate->pTextView));
  GtkWidget *filesel = gtk_file_chooser_dialog_new(_("Select File"), 
						   GTK_WINDOW(pTopLevel),
						   GTK_FILE_CHOOSER_ACTION_OPEN,
						   GTK_STOCK_OPEN, 
						   GTK_RESPONSE_ACCEPT, 
						   GTK_STOCK_CANCEL, 
						   GTK_RESPONSE_CANCEL, NULL);

#ifdef HAVE_GIO
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filesel), FALSE);
#endif

  if(gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
#ifdef HAVE_GIO
    char *filename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filesel));
#else
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
#endif
    dasher_editor_internal_new_buffer(pSelf, filename);
    g_free(filename);
  }

  gtk_widget_destroy(filesel);
}

static void 
dasher_editor_internal_command_save(DasherEditor *pSelf, gboolean bPrompt, gboolean bAppend) { 
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  gchar *szFilename = NULL;

  // Hmm... this makes no sense - surely this always evaluates to true?
  if(bPrompt || !szFilename) {
    GtkWidget *pTopLevel = gtk_widget_get_toplevel(GTK_WIDGET(pPrivate->pTextView));
    GtkWidget *filesel = gtk_file_chooser_dialog_new(_("Select File"), 
						     GTK_WINDOW(pTopLevel), 
						     GTK_FILE_CHOOSER_ACTION_SAVE, 
						     GTK_STOCK_SAVE, 
						     GTK_RESPONSE_ACCEPT, 
						     GTK_STOCK_CANCEL, 
						     GTK_RESPONSE_CANCEL, NULL);
    
#ifdef HAVE_GIO
    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filesel), FALSE);
#endif
    
    if(gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
#ifdef HAVE_GIO
      szFilename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filesel));
#else
      szFilename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
#endif
    }
    else {
      gtk_widget_destroy(filesel);
      return;
    }
    
    gtk_widget_destroy(filesel);
  }
  else {
    szFilename = g_strdup(pPrivate->szFilename);
  }

  dasher_editor_internal_save_as(pSelf, szFilename, bAppend);
  g_free(szFilename);
}

#ifdef HAVE_GIO
static void 
dasher_editor_internal_gvfs_print_error(DasherEditor *pSelf, GError *error, const char *myfilename) {
  // Turns a GVFS error into English
  GtkWidget *error_dialog;
  error_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open the file \"%s\"\n%s\n", myfilename, error->message);
  gtk_dialog_set_default_response(GTK_DIALOG(error_dialog), GTK_RESPONSE_OK);
  gtk_window_set_resizable(GTK_WINDOW(error_dialog), FALSE);
  gtk_dialog_run(GTK_DIALOG(error_dialog));
  gtk_widget_destroy(error_dialog);
  return;
}

static gboolean 
dasher_editor_internal_gvfs_open_file(DasherEditor *pSelf, const char *uri, gchar **buffer, gsize *size)
{
  GFile *file;
  GFileInputStream *read_handle;
  GError *error;
  GFileInfo *info;
  gssize bytes_read;

  file = g_file_new_for_uri (uri);

  read_handle = g_file_read (file, NULL, &error);

  /* If URI didn't work, try path */
  if (read_handle == NULL)
    { // PRLW: g_object_unref isn't actually stipulated by g_file_new_for_uri
      g_object_unref (file);
      file = g_file_new_for_path (uri);
      read_handle = g_file_read (file, NULL, &error);
    }

  if (read_handle == NULL)
    {
      dasher_editor_internal_gvfs_print_error (pSelf, error, uri);
      g_object_unref (file);
      return FALSE;
    }

  info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_SIZE,
                            G_FILE_QUERY_INFO_NONE, NULL, &error);

  if (info == NULL)
    {
      dasher_editor_internal_gvfs_print_error (pSelf, error, uri);
      g_input_stream_close (G_INPUT_STREAM(read_handle), NULL, &error);
      g_object_unref (read_handle);
      g_object_unref (file);
      return FALSE;
    }

  // XXX PRLW: cases info > max(size) as max(size) < max(uint64),
  // and bytes_read < size aren't handled.
  *size = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_STANDARD_SIZE);
  *buffer = (gchar *) g_malloc (*size);
  bytes_read = g_input_stream_read (G_INPUT_STREAM(read_handle), *buffer,
                                    *size, NULL, &error);

  if (bytes_read == -1)
    {
      dasher_editor_internal_gvfs_print_error (pSelf, error, uri);
      g_input_stream_close (G_INPUT_STREAM(read_handle), NULL, &error);
      g_object_unref (read_handle);
      g_object_unref (info);
      g_object_unref (file);
      return FALSE;
    }

  g_input_stream_close (G_INPUT_STREAM(read_handle), NULL, NULL);
  g_object_unref (read_handle);
  g_object_unref (info);
  g_object_unref (file);
  return TRUE;
}

static GFileOutputStream *append_or_replace_file(GFile *file, bool append, GError **error)
{
  GFileOutputStream *write_handle;
  if (append)
      write_handle = g_file_append_to (file, G_FILE_CREATE_NONE, NULL, error);
  else
      write_handle = g_file_replace (file, NULL, FALSE, G_FILE_CREATE_NONE,
                                     NULL, error);

  return write_handle;
}

static gboolean 
dasher_editor_internal_gvfs_save_file(DasherEditor *pSelf, const char *uri, gchar *buffer, gsize length, bool append)
{
  GFile *file;
  GFileOutputStream *write_handle;
  GError *error = NULL;
  gssize bytes_written;

  file = g_file_new_for_uri (uri);

  write_handle = append_or_replace_file (file, append, &error);

  /* If URI didn't work, try path */
  if (write_handle == NULL)
    {
      g_object_unref (file);
      file = g_file_new_for_path (uri);
      write_handle = append_or_replace_file (file, append, &error);
    }

  if (write_handle == NULL)
    {
      dasher_editor_internal_gvfs_print_error (pSelf, error, uri);
      g_object_unref (file);
      return FALSE;
    }

  if (append)
    {
      if (!g_seekable_seek(G_SEEKABLE(write_handle),0,G_SEEK_END, NULL, &error))
        {
          dasher_editor_internal_gvfs_print_error (pSelf, error, uri);
          g_object_unref (write_handle);
          g_object_unref (file);
          return FALSE;
        }
    }

  bytes_written = g_output_stream_write (G_OUTPUT_STREAM(write_handle), buffer,
                                         length, NULL, &error);
  // XXX PRLW: case bytes_written < length not handled.
  if (bytes_written == -1)
    {
      dasher_editor_internal_gvfs_print_error (pSelf, error, uri);
      g_output_stream_close (G_OUTPUT_STREAM(write_handle), NULL, NULL);
      g_object_unref (write_handle);
      g_object_unref (file);
      return FALSE;
    }

  g_output_stream_close (G_OUTPUT_STREAM(write_handle), NULL, NULL);
  g_object_unref (write_handle);
  g_object_unref (file);
  return TRUE;
}

#else /* not HAVE_GIO */

static gboolean 
dasher_editor_internal_unix_vfs_open_file(DasherEditor *pSelf, const char *myfilename, gchar **buffer, gsize *size) {
  GtkWidget *error_dialog;

  struct stat file_stat;
  FILE *fp;

  stat(myfilename, &file_stat);
  fp = fopen(myfilename, "r");

  if(fp == NULL || S_ISDIR(file_stat.st_mode)) {
    //    error_dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open the file \"%s\".\n", myfilename);
    error_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not open the file \"%s\".\n", myfilename);
    gtk_dialog_set_default_response(GTK_DIALOG(error_dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(error_dialog), FALSE);
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return FALSE;
  }

  *size = file_stat.st_size; // PRLW: is off_t = uint64_t, size is size_t, MD
  *buffer = (gchar *) g_malloc(*size);
  fread(*buffer, *size, 1, fp);
  fclose(fp);
  return TRUE;
}

static gboolean 
dasher_editor_internal_unix_vfs_save_file(DasherEditor *pSelf, const char *myfilename, gchar *buffer, gsize length, bool append) {
  int opened = 1;
  GtkWidget *error_dialog;

  FILE *fp;

  if(append == true) {
    fp = fopen(myfilename, "a");

    if(fp == NULL) {
      opened = 0;
    }
  }
  else {
    fp = fopen(myfilename, "w");
    if(fp == NULL) {
      opened = 0;
    }
  }

  if(!opened) {
    //    error_dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not save the file \"%s\".\n", myfilename);
    error_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Could not save the file \"%s\".\n", myfilename);
    gtk_dialog_set_default_response(GTK_DIALOG(error_dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(error_dialog), FALSE);
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return false;
  }

  fwrite(buffer, 1, length, fp);
  fclose(fp);
  return true;
}
#endif

static void 
dasher_editor_internal_set_filename(DasherEditor *pSelf, const gchar *szFilename) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  
  if(pPrivate->szFilename)
    g_free((void *)pPrivate->szFilename);

  if(szFilename)
    pPrivate->szFilename = g_strdup(szFilename);
  else
    pPrivate->szFilename = NULL;

  g_signal_emit_by_name(G_OBJECT(pSelf), "filename_changed", G_OBJECT(pSelf), NULL, NULL);
}

static void 
dasher_editor_internal_convert(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pBufferSet)
    idasher_buffer_set_edit_convert(pPrivate->pBufferSet);
}

static void 
dasher_editor_internal_protect(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pBufferSet)
    idasher_buffer_set_edit_protect(pPrivate->pBufferSet);
}

static void 
dasher_editor_internal_new_buffer(DasherEditor *pSelf, const gchar *szFilename) {
  /* TODO: eventually rewrite this without references to external functions */
  
  if(szFilename) {
    dasher_editor_internal_open(pSelf, szFilename);
  }
  else {
    dasher_editor_internal_generate_filename(pSelf);
    dasher_editor_internal_clear(pSelf, false);
  }

  //  g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);
}

static void
dasher_editor_internal_handle_parameter_change(DasherEditor *pSelf, gint iParameter) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  switch(iParameter) {
  case APP_SP_EDIT_FONT:
    dasher_editor_internal_handle_font(pSelf,
				       dasher_app_settings_get_string(pPrivate->pAppSettings, APP_SP_EDIT_FONT));
    break;
  }
}

/* Callback Functions */

extern "C" void 
delete_children_callback(GtkWidget *pWidget, gpointer pUserData) {
  gtk_widget_destroy(pWidget);
}

extern "C" void 
main_window_realized(DasherMain *pMain, gpointer pUserData) {
}

extern "C" void 
action_button_callback(GtkWidget *pWidget, gpointer pUserData) { 
  void **pPointers((void **)pUserData);
  dasher_editor_internal_action_button((DasherEditor *)pPointers[0], (DasherAction *)pPointers[1]);
}

extern "C" void 
context_changed_handler(GObject *pSource, gpointer pUserData) {
  DasherEditorInternal *pSelf = DASHER_EDITOR_INTERNAL(pUserData);

  // TODO: plumb signal back into control
  g_signal_emit_by_name(G_OBJECT(pSelf), "context_changed", G_OBJECT(pSelf), NULL, NULL);
}


extern "C" void 
buffer_changed_handler(GObject *pSource, gpointer pUserData) {
  DasherEditorInternal *pSelf = DASHER_EDITOR_INTERNAL(pUserData);

  // TODO: plumb signal back into control
  g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);
}





