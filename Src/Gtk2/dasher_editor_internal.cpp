#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>
#include <glib/gi18n.h>
#ifdef HAVE_GIO
#include <gio/gio.h>
#endif
#include <gtk/gtk.h>

#ifdef GNOME_A11Y
#include <cspi/spi.h>
#endif

#include "dasher_editor_internal.h"
#include "dasher_editor_external.h"
#include "dasher_lock_dialogue.h"
#include "dasher_main.h"
#include "../DasherCore/ControlManager.h"

typedef struct _DasherEditorInternalPrivate DasherEditorInternalPrivate;

struct _DasherEditorInternalPrivate {
  DasherMain *pDasherMain;
  GtkTextView *pTextView;
  GtkTextBuffer *pBuffer;
  GtkClipboard *pTextClipboard;
  GtkClipboard *pPrimarySelection;
  GtkTextMark *pNewMark;
  DasherAppSettings *pAppSettings;
  gchar *szFilename;
  gboolean bFileModified; // TODO: Make this work properly, export to main for quit etc

  // for conversion mode:
  GtkTextTag *pOutputTag;
  GtkTextTag *pHiddenTag;
  GtkTextTag *pVisibleTag;
  gboolean bConversionMode;
  gint iLastOffset;
  gint iCurrentState; // 0 = unconverted, 1 = converted

  // for direct mode:
#ifdef GNOME_A11Y
  AccessibleEventListener *pFocusListener;
  AccessibleEventListener *pCaretListener;
  AccessibleText *pAccessibleText;
#endif

  //Paralleling the previous approach in dasher_main, we _don't_ send context_changed
  // events if we're in the middle of executing a control action (as this would rebuild
  // the canvas)
  gboolean bInControlAction;
};

#define DASHER_EDITOR_INTERNAL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_EDITOR_INTERNAL, DasherEditorInternalPrivate))

G_DEFINE_TYPE(DasherEditorInternal, dasher_editor_internal, DASHER_TYPE_EDITOR);

static void dasher_editor_internal_finalize(GObject *pObject);

static void dasher_editor_internal_handle_font(DasherEditor *pSelf, const gchar *szFont);

gboolean dasher_editor_internal_command(DasherEditor *pSelf, const gchar *szCommand);
void dasher_editor_internal_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GtkBuilder *pXML, const gchar *szFullPath);

/* Private methods */
static void dasher_editor_internal_select_all(DasherEditor *pSelf);

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

static void dasher_editor_internal_edit_convert(DasherEditor *pSelf);
static void dasher_editor_internal_edit_protect(DasherEditor *pSelf);
using Dasher::CControlManager;
static gint dasher_editor_internal_ctrl_delete(DasherEditor *pSelf, bool bForwards, CControlManager::EditDistance iDist);
static gint dasher_editor_internal_ctrl_move(DasherEditor *pSelf, bool bForwards, CControlManager::EditDistance iDist);

static void dasher_editor_internal_new_buffer(DasherEditor *pSelf, const gchar *szFilename);

static void dasher_editor_internal_generate_filename(DasherEditor *pSelf);
static void dasher_editor_internal_open(DasherEditor *pSelf, const gchar *szFilename);
static bool dasher_editor_internal_save_as(DasherEditor *pSelf, const gchar *szFilename, bool bAppend);
static void dasher_editor_internal_create_buffer(DasherEditor *pSelf);
static void dasher_editor_internal_clipboard(DasherEditor *pSelf, clipboard_action act);

/* To be obsoleted by movement to GTK buffers */
void dasher_editor_internal_output(DasherEditor *pSelf, const gchar *szText, int iOffset);
void dasher_editor_internal_delete(DasherEditor *pSelf, int iLength, int iOffset);
const gchar *dasher_editor_internal_get_context(DasherEditor *pSelf, int iOffset, int iLength);
gint dasher_editor_internal_get_offset(DasherEditor *pSelf);
void dasher_editor_internal_mark_changed(DasherEditorInternal *pSelf, GtkTextIter *pIter, GtkTextMark *pMark);

/* Events proagated from main */
void dasher_editor_internal_handle_stop(DasherEditor *pSelf);
void dasher_editor_internal_handle_start(DasherEditor *pSelf);

void dasher_editor_internal_grab_focus(DasherEditor *pSelf);

/* Todo: possibly tidy up the need to have this public (quit in dasher_main possibly too connected) */
gboolean dasher_editor_internal_file_changed(DasherEditor *pSelf);
const gchar *dasher_editor_internal_get_filename(DasherEditor *pSelf);

static void dasher_editor_internal_clear(DasherEditor *pSelf);
const gchar *dasher_editor_internal_get_all_text(DasherEditor *pSelf);
const gchar *dasher_editor_internal_get_new_text(DasherEditor *pSelf);
static GtkTextBuffer *dasher_editor_internal_game_text_buffer(DasherEditor *pEditor);

static void dasher_editor_internal_handle_parameter_change(DasherEditor *pSelf, gint iParameter);


// Private methods not in class
extern "C" void delete_children_callback(GtkWidget *pWidget, gpointer pUserData);
extern "C" void main_window_realized(DasherMain *pMain, gpointer pUserData);
extern "C" void action_button_callback(GtkWidget *pWidget, gpointer pUserData);
extern "C" void mark_set_handler(GtkWidget *widget, GtkTextIter *pIter, GtkTextMark *pMark, gpointer pUserData);
extern "C" void handle_stop_event(GtkDasherControl *pDasherControl, gpointer data);
extern "C" void handle_request_settings(GtkDasherControl * pDasherControl, gpointer data);
extern "C" void gtk2_edit_delete_callback(GtkDasherControl *pDasherControl, const gchar *szText, int iOffset, gpointer user_data);
extern "C" void gtk2_edit_output_callback(GtkDasherControl *pDasherControl, const gchar *szText, int iOffset, gpointer user_data);

static gboolean
isdirect(DasherAppSettings *pAppSettings) {
  return (dasher_app_settings_get_long(pAppSettings, APP_LP_STYLE) == APP_STYLE_DIRECT);
}

static void
dasher_editor_internal_class_init(DasherEditorInternalClass *pClass) {
  g_type_class_add_private(pClass, sizeof(DasherEditorInternalPrivate));

  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_editor_internal_finalize;

  DasherEditorClass *pParentClass = (DasherEditorClass *)pClass;

  pParentClass->initialise = dasher_editor_internal_initialise;
  pParentClass->game_text_buffer = dasher_editor_internal_game_text_buffer;
  pParentClass->command = dasher_editor_internal_command;
  pParentClass->output = dasher_editor_internal_output;
  pParentClass->delete_text = dasher_editor_internal_delete;
  pParentClass->get_context = dasher_editor_internal_get_context;
  pParentClass->get_offset = dasher_editor_internal_get_offset;
  pParentClass->ctrl_move = dasher_editor_internal_ctrl_move;
  pParentClass->ctrl_delete = dasher_editor_internal_ctrl_delete;
  pParentClass->edit_convert = dasher_editor_internal_edit_convert;
  pParentClass->edit_protect = dasher_editor_internal_edit_protect;

  pParentClass->handle_stop = dasher_editor_internal_handle_stop;
  pParentClass->handle_start = dasher_editor_internal_handle_start;
  pParentClass->grab_focus = dasher_editor_internal_grab_focus;
  pParentClass->file_changed = dasher_editor_internal_file_changed;
  pParentClass->get_filename = dasher_editor_internal_get_filename;
  pParentClass->clear = dasher_editor_internal_clear;
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
  pPrivate->szFilename = NULL;
  pPrivate->pTextClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  pPrivate->pPrimarySelection = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  GtkTextIter oStartIter;
  gtk_text_buffer_get_start_iter(pPrivate->pBuffer, &oStartIter);
  pPrivate->pNewMark =
       gtk_text_buffer_create_mark(pPrivate->pBuffer, NULL, &oStartIter, TRUE);
  pPrivate->bFileModified = FALSE;
  pPrivate->bInControlAction = FALSE;

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

#ifdef GNOME_A11Y
  SPI_deregisterGlobalEventListener(pPrivate->pFocusListener, "focus:");
  SPI_deregisterGlobalEventListener(pPrivate->pCaretListener, "object:text-caret-moved");
#endif

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

  if (isdirect(pAppSettings))
    dasher_editor_external_create_buffer(pSelf);
  else
    // TODO: is this still needed?
    dasher_editor_internal_create_buffer(pSelf);

  // TODO: see note in command_new method
  if(szFullPath)
    dasher_editor_internal_open(pSelf, szFullPath);
  else {
    dasher_editor_internal_generate_filename(pSelf);
    dasher_editor_internal_clear(pSelf);
  }
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

void
dasher_editor_internal_handle_stop(DasherEditor *pSelf) {
}

void
dasher_editor_internal_handle_start(DasherEditor *pSelf) {
  // The edit box keeps track of where we started

  // TODO: This should be filtered through the buffer, rather than directly to the edit box
  //  set_mark();
}


static void
dasher_editor_internal_clear(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  GtkTextIter start, end;

  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &start, 0);
  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &end, -1);

  gtk_text_buffer_delete(pPrivate->pBuffer, &start, &end);

  /* TODO: this probably shouldn't emit a signal */
  //ACL but since it did...internal_buffer emitted "buffer_changed" signal,
  // which was picked up by callback registered by editor_internal, which
  // then emitted a "buffer_changed" signal from the editor_internal. So
  // emit directly from the editor_internal...
  g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);

  pPrivate->iCurrentState = 0;
  pPrivate->iLastOffset = 0;
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


  pPrivate->pOutputTag = gtk_text_buffer_create_tag(pPrivate->pBuffer, NULL, NULL);

#if GTK_CHECK_VERSION(2,8,0)
  pPrivate->pHiddenTag = gtk_text_buffer_create_tag(pPrivate->pBuffer, NULL, "invisible", TRUE, NULL);
#else
  // TODO: We really ought to do something a little more sensible with conversion in GTK < 2.8
  pPrivate->pHiddenTag = gtk_text_buffer_create_tag(pPrivate->pBuffer, NULL, NULL);
#endif

  pPrivate->pVisibleTag = gtk_text_buffer_create_tag(pPrivate->pBuffer, NULL, "foreground", "red", NULL);

  pPrivate->bConversionMode = FALSE;
  pPrivate->iLastOffset = 1;
  pPrivate->iCurrentState = 0;

  g_signal_connect(G_OBJECT(pPrivate->pBuffer), "mark-set", G_CALLBACK(mark_set_handler), pSelf);
}

void
dasher_editor_internal_output(DasherEditor *pSelf, const gchar *szText, int iOffset) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if (isdirect(pPrivate->pAppSettings))
    return dasher_editor_external_output(pSelf, szText, iOffset);

  gtk_text_buffer_delete_selection(pPrivate->pBuffer, false, true );

  GtkTextIter sIter;
  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &sIter, gtk_text_buffer_get_insert(pPrivate->pBuffer));

  GtkTextTag *pCurrentTag = NULL;

  if(!pPrivate->bConversionMode)
    pCurrentTag = pPrivate->pOutputTag;
  else {
    switch(pPrivate->iCurrentState) {
    case 0:
      pCurrentTag = pPrivate->pVisibleTag;
      break;
    case 1:
      pCurrentTag = pPrivate->pOutputTag;
      break;
    }
  }

  if(!pCurrentTag)
    return;

  gtk_text_buffer_insert_with_tags(pPrivate->pBuffer, &sIter, szText, -1, pCurrentTag, NULL);

  gtk_text_view_scroll_mark_onscreen(pPrivate->pTextView, gtk_text_buffer_get_insert(pPrivate->pBuffer));

  pPrivate->bFileModified = TRUE;
}

void
dasher_editor_internal_delete(DasherEditor *pSelf, int iLength, int iOffset) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if (isdirect(pPrivate->pAppSettings))
    return dasher_editor_external_delete(pSelf, iLength, iOffset);

  GtkTextIter end;

  //Dasher offset 0 = "the first character"; Gtk Text Buffer offset 0
  // = "the cursor position just before the first character" (and we want
  // the cursor position just after)
  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &end, iOffset+1);

  GtkTextIter start = end;

  gtk_text_iter_backward_chars(&start, iLength);
  //  g_bIgnoreCursorMove = true;
  gtk_text_buffer_delete(pPrivate->pBuffer, &start, &end);
  gtk_text_view_scroll_mark_onscreen(pPrivate->pTextView, gtk_text_buffer_get_insert(pPrivate->pBuffer));
  //  g_bIgnoreCursorMove = false;

  pPrivate->bFileModified = TRUE;
}

const gchar *
dasher_editor_internal_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if (isdirect(pPrivate->pAppSettings))
    return dasher_editor_external_get_context(pSelf, iOffset, iLength);

  //  g_message("Buffer lenght: %d", gtk_text_buffer_get_char_count(pPrivate->pBuffer));

  GtkTextIter start;
  GtkTextIter end; // Refers to end of context, which is start of selection!

  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &start, iOffset);
  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &end, iOffset + iLength);

  return gtk_text_buffer_get_text( pPrivate->pBuffer, &start, &end, false );
}

gint
dasher_editor_internal_get_offset(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if (isdirect(pPrivate->pAppSettings))
    return dasher_editor_external_get_offset(pSelf);

  GtkTextIter iter1,iter2;
  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &iter1, gtk_text_buffer_get_insert(pPrivate->pBuffer));
  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &iter2, gtk_text_buffer_get_selection_bound(pPrivate->pBuffer));
  return std::min(gtk_text_iter_get_offset(&iter1),gtk_text_iter_get_offset(&iter2));
}

void dasher_editor_internal_mark_changed(DasherEditorInternal *pSelf, GtkTextIter *pIter, GtkTextMark *pMark) {
  const char *szMarkName(gtk_text_mark_get_name(pMark));
  if(szMarkName && !strcmp(szMarkName,"insert")) {
    DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
    //ACL: used to emit "offset_changed" signal from buffer, which was picked up
    // by a callback registered by editor_internal, which then emitted a context_changed
    // signal from the editor_internal. So just emit the context_changed directly...
    if (!pPrivate->bInControlAction //tho not if it's the result of a control-mode edit/delete
        && !dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_GAME_MODE)) //and not in game mode
      g_signal_emit_by_name(G_OBJECT(pSelf), "context_changed", G_OBJECT(pSelf), NULL, NULL);
  }
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

GtkTextBuffer *dasher_editor_internal_game_text_buffer(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  return pPrivate->pBuffer;
}

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
dasher_editor_internal_edit_convert(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if(!(pPrivate->bConversionMode))
    return;

  GtkTextIter sStartIter;
  GtkTextIter sEndIter;
  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &sStartIter, pPrivate->iLastOffset);
  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &sEndIter, -1);
  gtk_text_buffer_apply_tag(pPrivate->pBuffer, pPrivate->pHiddenTag, &sStartIter, &sEndIter);

  pPrivate->iCurrentState = 1;
  pPrivate->iLastOffset = gtk_text_buffer_get_char_count(pPrivate->pBuffer);
}

static void
dasher_editor_internal_edit_protect(DasherEditor *pSelf) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  if(!(pPrivate->bConversionMode))
    return;

  pPrivate->iCurrentState = 0;
  pPrivate->iLastOffset = gtk_text_buffer_get_char_count(pPrivate->pBuffer);
}

static void edit_find(bool bForwards, CControlManager::EditDistance iDist, DasherEditorInternalPrivate *pPrivate, GtkTextIter *pPos) {
  if(bForwards) {
    switch(iDist) {
    case CControlManager::EDIT_CHAR:
      gtk_text_iter_forward_char(pPos);
      break;
    case CControlManager::EDIT_WORD:
      gtk_text_iter_forward_word_end(pPos);
      break;
    case CControlManager::EDIT_LINE:
      if(!gtk_text_view_forward_display_line_end(GTK_TEXT_VIEW(pPrivate->pTextView), pPos))
      {
        gtk_text_view_forward_display_line (GTK_TEXT_VIEW(pPrivate->pTextView), pPos);
        gtk_text_view_forward_display_line_end(GTK_TEXT_VIEW(pPrivate->pTextView), pPos);
      }
      break;
    case CControlManager::EDIT_FILE:
      gtk_text_iter_forward_to_end(pPos);
      break;
    }
  }
  else {
    switch(iDist) {
    case CControlManager::EDIT_CHAR:
      gtk_text_iter_backward_char(pPos);
      break;
    case CControlManager::EDIT_WORD:
      gtk_text_iter_backward_word_start(pPos);
      break;
    case CControlManager::EDIT_LINE:

      if(!gtk_text_view_backward_display_line_start(GTK_TEXT_VIEW(pPrivate->pTextView), pPos))
        gtk_text_view_backward_display_line(GTK_TEXT_VIEW(pPrivate->pTextView), pPos);
      break;
    case CControlManager::EDIT_FILE:
      gtk_text_buffer_get_start_iter(pPrivate->pBuffer, pPos);
      break;
    }
  }
}

static gint
dasher_editor_internal_ctrl_delete(DasherEditor *pSelf, bool bForwards, CControlManager::EditDistance iDist) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);

  GtkTextIter sPosStart;
  GtkTextIter sPosEnd;

  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &sPosStart, gtk_text_buffer_get_insert(pPrivate->pBuffer));

  sPosEnd = sPosStart;
  edit_find(bForwards, iDist, pPrivate, &sPosStart);
  gint iRet = gtk_text_iter_get_offset(&sPosStart);
  pPrivate->bInControlAction = true;
  gtk_text_buffer_delete(pPrivate->pBuffer, &sPosStart, &sPosEnd);
  pPrivate->bInControlAction = false;
  gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(pPrivate->pTextView), gtk_text_buffer_get_insert(pPrivate->pBuffer));
  return iRet;
}

static gint
dasher_editor_internal_ctrl_move(DasherEditor *pSelf, bool bForwards, CControlManager::EditDistance iDist) {
  DasherEditorInternalPrivate *pPrivate = DASHER_EDITOR_INTERNAL_GET_PRIVATE(pSelf);
  GtkTextIter sPos;

  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &sPos, gtk_text_buffer_get_insert(pPrivate->pBuffer));

  edit_find(bForwards, iDist, pPrivate, &sPos);
  gint iRet = gtk_text_iter_get_offset(&sPos);
  pPrivate->bInControlAction = true;
  gtk_text_buffer_place_cursor(pPrivate->pBuffer, &sPos);
  pPrivate->bInControlAction = false;
  gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(pPrivate->pTextView), gtk_text_buffer_get_insert(pPrivate->pBuffer));
  return iRet;
}


static void
dasher_editor_internal_new_buffer(DasherEditor *pSelf, const gchar *szFilename) {
  /* TODO: eventually rewrite this without references to external functions */

  if(szFilename) {
    dasher_editor_internal_open(pSelf, szFilename);
  }
  else {
    dasher_editor_internal_generate_filename(pSelf);
    dasher_editor_internal_clear(pSelf);
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

extern "C" void mark_set_handler(GtkWidget *widget, GtkTextIter *pIter, GtkTextMark *pMark, gpointer pUserData) {
  dasher_editor_internal_mark_changed(DASHER_EDITOR_INTERNAL(pUserData), pIter, pMark);
}

#include "dasher_editor_external.cpp"
