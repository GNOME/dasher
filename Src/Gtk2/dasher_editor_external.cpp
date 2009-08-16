#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "dasher_editor_external.h"
#include "dasher_external_buffer.h"
#include "dasher_lock_dialogue.h"
#include "dasher_main.h"
//#include "game_mode_helper.h"

typedef struct _DasherEditorExternalPrivate DasherEditorExternalPrivate;

struct _DasherEditorExternalPrivate {
  DasherMain *pDasherMain;
  DasherAppSettings *pAppSettings;
  IDasherBufferSet *pBufferSet;
};

#define DASHER_EDITOR_EXTERNAL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_EDITOR_EXTERNAL, DasherEditorExternalPrivate))

static DasherEditorExternal *g_pEditor;

G_DEFINE_TYPE(DasherEditorExternal, dasher_editor_external, DASHER_TYPE_EDITOR);


static void dasher_editor_external_finalize(GObject *pObject);

/* Private Method Declarations */

static gboolean dasher_editor_external_command(DasherEditor *pSelf, const gchar *szCommand);
static void dasher_editor_external_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, 
					      DasherMain *pDasherMain, GtkBuilder *pXML, 
					      const gchar *szFullPath);
static void dasher_editor_external_convert(DasherEditor *pSelf);
static void dasher_editor_external_protect(DasherEditor *pSelf);
static void dasher_editor_external_create_buffer(DasherEditor *pSelf);
static void dasher_editor_external_output(DasherEditor *pSelf, const gchar *szText, int iOffset);
static void dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset);
static const gchar *dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength);
static gint dasher_editor_external_get_offset(DasherEditor *pSelf);
static void dasher_editor_external_handle_control(DasherEditor *pSelf, int iNodeID);
static void dasher_editor_external_grab_focus(DasherEditor *pSelf);
static const gchar *dasher_editor_external_get_all_text(DasherEditor *pSelf);
static const gchar *dasher_editor_external_get_new_text(DasherEditor *pSelf);

/* Callback Declarations */

extern "C" void external_context_changed_handler(GObject *pSource, gpointer pUserData);
extern "C" void external_buffer_changed_handler(GObject *pSource, gpointer pUserData);

/* Method Definitions */

static void 
dasher_editor_external_class_init(DasherEditorExternalClass *pClass) {

  g_type_class_add_private(pClass, sizeof(DasherEditorExternalPrivate));

  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_editor_external_finalize;

  DasherEditorClass *pParentClass = (DasherEditorClass *)pClass;

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

  pPrivate->pDasherMain = NULL;
  pPrivate->pAppSettings = NULL;
  pPrivate->pBufferSet = NULL;
}

static void 
dasher_editor_external_finalize(GObject *pObject) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pObject);

  if(pPrivate->pBufferSet)
    g_object_unref(G_OBJECT(pPrivate->pBufferSet));
}

/* Public methods */
DasherEditorExternal *
dasher_editor_external_new() {
  // TODO: Handle singleton class a little more sensibly

  DasherEditorExternal *pDasherEditor;
  pDasherEditor =
        DASHER_EDITOR_EXTERNAL(g_object_new(DASHER_TYPE_EDITOR_EXTERNAL, NULL));
  g_pEditor = pDasherEditor;

  // TODO: Just add a simple placeholder widget here (eventually need
  // to figure out how to have a null (ie non-visible) widget.

  g_message("Creating new external editor");

  return pDasherEditor;
}

static void
dasher_editor_external_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GtkBuilder *pXML, const gchar *szFullPath) {

  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  pPrivate->pAppSettings = pAppSettings;
  pPrivate->pDasherMain = pDasherMain;

  dasher_editor_external_create_buffer(pSelf);
}

/* TODO: This is obsolete - sort this out when commands are reconsidered */
static void 
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


static void 
dasher_editor_external_grab_focus(DasherEditor *pSelf) {
}

static void 
dasher_editor_external_create_buffer(DasherEditor *pSelf) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  if(!(pPrivate->pBufferSet))
    pPrivate->pBufferSet = IDASHER_BUFFER_SET(dasher_external_buffer_new());

  g_signal_connect(G_OBJECT(pPrivate->pBufferSet), "offset_changed", G_CALLBACK(external_context_changed_handler), pSelf);
  g_signal_connect(G_OBJECT(pPrivate->pBufferSet), "buffer_changed", G_CALLBACK(external_buffer_changed_handler), pSelf);
}

static void 
dasher_editor_external_output(DasherEditor *pSelf, const gchar *szText, int iOffset) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pBufferSet)
    idasher_buffer_set_insert(pPrivate->pBufferSet, szText, iOffset);
}

static void 
dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  if(pPrivate->pBufferSet)
    idasher_buffer_set_delete(pPrivate->pBufferSet, iLength, iOffset);
}

static const gchar *
dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
  // TODO: Check where this function is used
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  gchar *szContext;

  if(pPrivate->pBufferSet)
    szContext = idasher_buffer_set_get_context(pPrivate->pBufferSet, iOffset, iLength);
  else
    szContext = "";

  return szContext;
}

static gint 
dasher_editor_external_get_offset(DasherEditor *pSelf) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);
  return idasher_buffer_set_get_offset(pPrivate->pBufferSet);
}

static gboolean 
dasher_editor_external_command(DasherEditor *pSelf, const gchar *szCommand) {
  return FALSE;
}


// TODO: We shouldn't need to know about the buffer here - make this a method of the buffer set
static const gchar *
dasher_editor_external_get_all_text(DasherEditor *pSelf) { 
  return NULL;
}

static const gchar *
dasher_editor_external_get_new_text(DasherEditor *pSelf) { 
  return NULL;
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

/* Callback Functions */

extern "C" void 
external_context_changed_handler(GObject *pSource, gpointer pUserData) {
  DasherEditorExternal *pSelf = DASHER_EDITOR_EXTERNAL(pUserData);

  // TODO: plumb signal back into control
  g_signal_emit_by_name(G_OBJECT(pSelf), "context_changed", G_OBJECT(pSelf), NULL, NULL);
}

extern "C" void 
external_buffer_changed_handler(GObject *pSource, gpointer pUserData) {
  DasherEditorExternal *pSelf = DASHER_EDITOR_EXTERNAL(pUserData);

  // TODO: plumb signal back into control
  g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);
}
