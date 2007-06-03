#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "dasher_editor_external.h"
#include "dasher_external_buffer.h"
#include "dasher_internal_buffer.h"
#include "dasher_lock_dialogue.h"
#include "dasher_main.h"
#include "game_mode_helper.h"

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

typedef struct _DasherEditorExternalPrivate DasherEditorExternalPrivate;

struct _DasherEditorExternalPrivate {
  DasherMain *pDasherMain;
  GtkTextView *pTextView;
  GtkTextBuffer *pBuffer;
  GtkClipboard *pTextClipboard;
  GtkClipboard *pPrimarySelection;
  IDasherBufferSet *pBufferSet;
  IDasherBufferSet *pExternalBuffer;
  GameModeHelper *pGameModeHelper;
  GtkTextMark *pNewMark;
  DasherAppSettings *pAppSettings;
  gchar *szFilename;
  gboolean bFileModified; // TODO: Make this work properly, export to main for quit etc
};

#define DASHER_EDITOR_EXTERNAL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_DASHER_EDITOR_EXTERNAL, DasherEditorExternalPrivate))

/* Signals */
enum {
  FILENAME_CHANGED,
  BUFFER_CHANGED,
  CONTEXT_CHANGED,
  SIGNAL_NUM
};

static guint dasher_editor_external_signals[SIGNAL_NUM];

static DasherEditorExternal *g_pEditor;

G_DEFINE_TYPE(DasherEditorExternal, dasher_editor_external, TYPE_DASHER_EDITOR);

static void dasher_editor_external_finalize(GObject *pObject);



gboolean dasher_editor_external_command(DasherEditor *pSelf, const gchar *szCommand);
void dasher_editor_external_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GladeXML *pGladeXML, const gchar *szFullPath);

/* Private methods */
static void dasher_editor_external_select_all(DasherEditor *pSelf);
static void dasher_editor_external_display_message(DasherEditor *pSelf, DasherMessageInfo *pMessageInfo);

static void dasher_editor_external_command_new(DasherEditor *pSelf);
static void dasher_editor_external_command_open(DasherEditor *pSelf);
static void dasher_editor_external_command_save(DasherEditor *pSelf, gboolean bPrompt, gboolean bAppend);

static void dasher_editor_external_set_filename(DasherEditor *pSelf, const gchar *szFilename);

// TODO: Should these be public?
static void dasher_editor_external_convert(DasherEditor *pSelf);
static void dasher_editor_external_protect(DasherEditor *pSelf);

static void dasher_editor_external_new_buffer(DasherEditor *pSelf, const gchar *szFilename);

static void dasher_editor_external_create_buffer(DasherEditor *pSelf);

/* To be obsoleted by movement to GTK buffers */
void dasher_editor_external_output(DasherEditor *pSelf, const gchar *szText, int iOffset);
void dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset);
const gchar *dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength);
gint dasher_editor_external_get_offset(DasherEditor *pSelf);

/* Events proagated from main */
void dasher_editor_external_handle_control(DasherEditor *pSelf, int iNodeID);

void dasher_editor_external_grab_focus(DasherEditor *pSelf);

/* TODO: Tutorial editor should be a separate class */
//void dasher_editor_external_start_tutorial(DasherEditor *pSelf);

/* Todo: possibly tidy up the need to have this public (quit in dasher_main possibly too connected) */
gboolean dasher_editor_external_file_changed(DasherEditor *pSelf);
const gchar *dasher_editor_external_get_filename(DasherEditor *pSelf);

const gchar *dasher_editor_external_get_all_text(DasherEditor *pSelf);
const gchar *dasher_editor_external_get_new_text(DasherEditor *pSelf);

// Private methods not in class
extern "C" void delete_children_callback(GtkWidget *pWidget, gpointer pUserData);
extern "C" void main_window_realized(DasherMain *pMain, gpointer pUserData);
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
dasher_editor_external_class_init(DasherEditorExternalClass *pClass) {
  g_debug("Initialising DasherEditor");

  g_type_class_add_private(pClass, sizeof(DasherEditorExternalPrivate));

  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_editor_external_finalize;

  // /* Setup signals */
//   dasher_editor_external_signals[FILENAME_CHANGED] = g_signal_new("filename-changed", G_TYPE_FROM_CLASS(pClass), 
// 							 static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), 
// 							 G_STRUCT_OFFSET(DasherEditorClass, filename_changed), 
// 							 NULL, NULL, g_cclosure_marshal_VOID__VOID, 
// 							 G_TYPE_NONE, 0);

//   dasher_editor_external_signals[BUFFER_CHANGED] = g_signal_new("buffer-changed", G_TYPE_FROM_CLASS(pClass), 
// 						       static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), 
// 						       G_STRUCT_OFFSET(DasherEditorClass, buffer_changed), 
// 						       NULL, NULL, g_cclosure_marshal_VOID__VOID, 
// 						       G_TYPE_NONE, 0);

//   dasher_editor_external_signals[CONTEXT_CHANGED] = g_signal_new("context-changed", G_TYPE_FROM_CLASS(pClass), 
// 							static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), 
// 							G_STRUCT_OFFSET(DasherEditorClass, context_changed), 
// 							NULL, NULL, g_cclosure_marshal_VOID__VOID, 
// 							G_TYPE_NONE, 0);

  DasherEditorClass *pParentClass = (DasherEditorClass *)pClass;

  // TODO: Most of these aren't actually needed here

  pParentClass->initialise = dasher_editor_external_initialise;
  pParentClass->command = dasher_editor_external_command;
  pParentClass->output = dasher_editor_external_output;
  pParentClass->delete_text = dasher_editor_external_delete;
  pParentClass->get_context = dasher_editor_external_get_context;
  pParentClass->get_offset = dasher_editor_external_get_offset;
  pParentClass->handle_control = dasher_editor_external_handle_control;
  pParentClass->grab_focus = dasher_editor_external_grab_focus;
  pParentClass->get_all_text = dasher_editor_external_get_all_text;
  pParentClass->get_new_text = dasher_editor_external_get_new_text;

}

static void 
dasher_editor_external_init(DasherEditorExternal *pDasherControl) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pDasherControl);

  pPrivate->pBufferSet = NULL;
  pPrivate->pExternalBuffer = NULL;
  pPrivate->szFilename = NULL;
  pPrivate->pTextClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  pPrivate->pPrimarySelection = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  pPrivate->pGameModeHelper = NULL;
  pPrivate->bFileModified = FALSE;
}

static void 
dasher_editor_external_finalize(GObject *pObject) {
  g_debug("Finalising DasherEditor");

  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pObject);

  if(pPrivate->pBufferSet)
    g_object_unref(G_OBJECT(pPrivate->pBufferSet));

  if(pPrivate->szFilename)
    g_free(pPrivate->szFilename);
}

/* Public methods */
DasherEditorExternal *
dasher_editor_external_new() {
  DasherEditorExternal *pDasherEditor;
  pDasherEditor = (DasherEditorExternal *)(g_object_new(dasher_editor_external_get_type(), NULL));


  g_pEditor = pDasherEditor;

  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pDasherEditor);


  // TODO: Just add a simple placeholder widget here (eventually need
  // to figure out how to have a null (ie non-visible) widget.

  //  GtkPaned *pPane = GTK_PANED(glade_xml_get_widget(pGladeXML, "main_divider"));
  
  GtkWidget *pScrolledWindow = gtk_scrolled_window_new(NULL, NULL);

  GtkWidget *pTextView = gtk_text_view_new();

  pPrivate->pTextView = GTK_TEXT_VIEW(pTextView);

  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pTextView),
			      GTK_WRAP_WORD);

  gtk_container_add(GTK_CONTAINER(pScrolledWindow), pTextView);

  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pScrolledWindow),
				 GTK_POLICY_AUTOMATIC,
				 GTK_POLICY_AUTOMATIC);

  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(pScrolledWindow),
				      GTK_SHADOW_IN);

  gtk_box_pack_start(GTK_BOX(&(pDasherEditor->parent.box)), pScrolledWindow, true, true, 0);
  
  
  gtk_widget_show_all(GTK_WIDGET(&(pDasherEditor->parent.box)));

  return pDasherEditor;
}

void
dasher_editor_external_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GladeXML *pGladeXML, const gchar *szFullPath) {

  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  pPrivate->pAppSettings = pAppSettings;
  pPrivate->pDasherMain = pDasherMain;


    //GtkTextView *pTextView = GTK_TEXT_VIEW(glade_xml_get_widget(pGladeXML, "the_text_view"));
  pPrivate->pBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pPrivate->pTextView));

  gtk_widget_show_all(GTK_WIDGET(pPrivate->pTextView));

  GtkTextIter oStartIter;
  gtk_text_buffer_get_start_iter(pPrivate->pBuffer, &oStartIter);
  pPrivate->pNewMark = gtk_text_buffer_create_mark(pPrivate->pBuffer, NULL, &oStartIter, TRUE);

  // TODO: is this still needed?
  dasher_editor_external_create_buffer(pSelf);


  // TODO: see note in command_new method
  if(szFullPath) {
  }
  else {
  }
}

/* TODO: This is obsolete - sort this out when commands are reconsidered */
void 
dasher_editor_external_handle_control(DasherEditor *pSelf, int iNodeID) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);



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
dasher_editor_external_grab_focus(DasherEditor *pSelf) {
}


static void 
dasher_editor_external_create_buffer(DasherEditor *pSelf) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  // TODO: No need to have separate pBufferSet and pExternalBuffer
  if(!(pPrivate->pExternalBuffer))
    pPrivate->pExternalBuffer = IDASHER_BUFFER_SET(dasher_external_buffer_new());

  pPrivate->pBufferSet = pPrivate->pExternalBuffer;
 
  // TODO: Fix this
  g_signal_connect(G_OBJECT(pPrivate->pBufferSet), "offset_changed", G_CALLBACK(context_changed_handler), pSelf);
  g_signal_connect(G_OBJECT(pPrivate->pBufferSet), "buffer_changed", G_CALLBACK(buffer_changed_handler), pSelf);
}

void 
dasher_editor_external_output(DasherEditor *pSelf, const gchar *szText, int iOffset) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pBufferSet)
    idasher_buffer_set_insert(pPrivate->pBufferSet, szText, iOffset);
}

void 
dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pBufferSet)
    idasher_buffer_set_delete(pPrivate->pBufferSet, iLength, iOffset);
}

const gchar *
dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
  // TODO: Check where this function is used
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  gchar *szContext;

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
dasher_editor_external_get_offset(DasherEditor *pSelf) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);
  return idasher_buffer_set_get_offset(pPrivate->pBufferSet);
}

gboolean 
dasher_editor_external_command(DasherEditor *pSelf, const gchar *szCommand) {

  return FALSE;
}


// TODO: We shouldn't need to know about the buffer here - make this a method of the buffer set
const gchar *
dasher_editor_external_get_all_text(DasherEditor *pSelf) { 
}

const gchar *
dasher_editor_external_get_new_text(DasherEditor *pSelf) { 
}


static void 
dasher_editor_external_convert(DasherEditor *pSelf) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pBufferSet)
    idasher_buffer_set_edit_convert(pPrivate->pBufferSet);
}

static void 
dasher_editor_external_protect(DasherEditor *pSelf) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pBufferSet)
    idasher_buffer_set_edit_protect(pPrivate->pBufferSet);
}

static void 
dasher_editor_external_new_buffer(DasherEditor *pSelf, const gchar *szFilename) {
  /* TODO: eventually rewrite this without references to external functions */
  
  if(szFilename) {
  }
  else {
  }

  //  g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);
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
context_changed_handler(GObject *pSource, gpointer pUserData) {
  DasherEditorExternal *pSelf = DASHER_EDITOR_EXTERNAL(pUserData);

  // TODO: plumb signal back into control
  g_signal_emit_by_name(G_OBJECT(pSelf), "context_changed", G_OBJECT(pSelf), NULL, NULL);
}


extern "C" void 
buffer_changed_handler(GObject *pSource, gpointer pUserData) {
  DasherEditorExternal *pSelf = DASHER_EDITOR_EXTERNAL(pUserData);

  // TODO: plumb signal back into control
  g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);
}





