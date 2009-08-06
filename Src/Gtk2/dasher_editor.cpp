// dasher_editor.cpp
//
// Copyright (c) 2008 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "dasher_action_keyboard.h"
#ifdef WITH_MAEMO
#include "dasher_action_keyboard_maemo.h"
#else
#include "dasher_action_script.h"
#endif
#ifdef GNOME_SPEECH
#include "dasher_action_speech.h"
#endif 
#include "dasher_editor.h"
#include "dasher_external_buffer.h"
#include "dasher_internal_buffer.h"
#include "dasher_lock_dialogue.h"
#include "dasher_main.h"
//#include "game_mode_helper.h"

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

typedef struct _DasherEditorPrivate DasherEditorPrivate;

struct _DasherEditorPrivate {
  DasherMain *pDasherMain;
  GtkTextView *pTextView;
  GtkTextBuffer *pBuffer;
  GtkVBox *pActionPane;
  GtkClipboard *pTextClipboard;
  GtkClipboard *pPrimarySelection;
  EditorAction *pActionRing;
  EditorAction *pActionIter;
  gboolean bActionIterStarted;
  gint iNextActionID;
  IDasherBufferSet *pBufferSet;
  IDasherBufferSet *pExternalBuffer;
  IDasherBufferSet *pInternalBuffer;
  //  GameModeHelper *pGameModeHelper;
  GtkTextMark *pNewMark;
  DasherAppSettings *pAppSettings;
  gchar *szFilename;
  gboolean bFileModified; // TODO: Make this work properly, export to main for quit etc
};

#define DASHER_EDITOR_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_EDITOR, DasherEditorPrivate))

/* Signals */
enum {
  FILENAME_CHANGED,
  BUFFER_CHANGED,
  CONTEXT_CHANGED,
  SIGNAL_NUM
};

static guint dasher_editor_signals[SIGNAL_NUM];

/* TODO: Use appropriate macros here */
/* G-object boilerplate code */
// static void dasher_editor_class_init(DasherEditorClass *pClass);
// static void dasher_editor_init(DasherEditor *pEditor);

G_DEFINE_TYPE(DasherEditor, dasher_editor, GTK_TYPE_VBOX);

static void dasher_editor_finalize(GObject *pObject);

static void 
dasher_editor_class_init(DasherEditorClass *pClass) {
  g_type_class_add_private(pClass, sizeof(DasherEditorPrivate));

  GObjectClass *pObjectClass = G_OBJECT_CLASS(pClass);
  pObjectClass->finalize = dasher_editor_finalize;

  /* Setup signals */
  dasher_editor_signals[FILENAME_CHANGED] = g_signal_new("filename-changed", G_TYPE_FROM_CLASS(pClass), 
							 static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), 
							 G_STRUCT_OFFSET(DasherEditorClass, filename_changed), 
							 NULL, NULL, g_cclosure_marshal_VOID__VOID, 
							 G_TYPE_NONE, 0);

  dasher_editor_signals[BUFFER_CHANGED] = g_signal_new("buffer-changed", G_TYPE_FROM_CLASS(pClass), 
						       static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), 
						       G_STRUCT_OFFSET(DasherEditorClass, buffer_changed), 
						       NULL, NULL, g_cclosure_marshal_VOID__VOID, 
						       G_TYPE_NONE, 0);

  dasher_editor_signals[CONTEXT_CHANGED] = g_signal_new("context-changed", G_TYPE_FROM_CLASS(pClass), 
							static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), 
							G_STRUCT_OFFSET(DasherEditorClass, context_changed), 
							NULL, NULL, g_cclosure_marshal_VOID__VOID, 
							G_TYPE_NONE, 0);

  pClass->initialise = NULL;
  pClass->command = NULL;
  pClass->action_button = NULL;
  pClass->actions_start = NULL;
  pClass->actions_more = NULL;
  pClass->actions_get_next = NULL;
  pClass->action_set_show = NULL;
  pClass->action_set_control = NULL;
  pClass->action_set_auto = NULL;
  pClass->get_all_text = NULL;
  pClass->get_new_text = NULL;
  pClass->output = NULL;
  pClass->delete_text = NULL;
  pClass->start_compose = NULL;
  pClass->end_compose = NULL;
  pClass->get_context = NULL;
  pClass->get_offset = NULL;
  pClass->handle_parameter_change = NULL;
  pClass->handle_stop = NULL;
  pClass->handle_start = NULL;
  pClass->handle_control = NULL;
  pClass->grab_focus = NULL;
  pClass->file_changed = NULL;
  pClass->get_filename = NULL;

  pClass->filename_changed = NULL;
  pClass->buffer_changed = NULL;
  pClass->context_changed = NULL;
}

static void 
dasher_editor_init(DasherEditor *pDasherControl) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pDasherControl);

  pPrivate->pBufferSet = NULL;
  pPrivate->pInternalBuffer = NULL;
  pPrivate->pExternalBuffer = NULL;
  pPrivate->szFilename = NULL;
  pPrivate->pTextClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  pPrivate->pPrimarySelection = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  pPrivate->pActionRing = NULL;
  pPrivate->iNextActionID = 0;
  //  pPrivate->pGameModeHelper = NULL;
  pPrivate->bFileModified = FALSE;
}

static void 
dasher_editor_finalize(GObject *pObject) {
  g_debug("Finalising DasherEditor");

  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pObject);

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

void
dasher_editor_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GtkBuilder *pXML, const gchar *szFullPath) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->initialise)
    DASHER_EDITOR_GET_CLASS(pSelf)->initialise(pSelf, pAppSettings, pDasherMain, pXML, szFullPath);
}

void 
dasher_editor_handle_stop(DasherEditor *pSelf) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->handle_stop)
    DASHER_EDITOR_GET_CLASS(pSelf)->handle_stop(pSelf);
}

void 
dasher_editor_handle_start(DasherEditor *pSelf) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->handle_start)
    DASHER_EDITOR_GET_CLASS(pSelf)->handle_start(pSelf);
}

/* TODO: This is obsolete - sort this out when commands are reconsidered */
void 
dasher_editor_handle_control(DasherEditor *pSelf, int iNodeID) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->handle_control)
    DASHER_EDITOR_GET_CLASS(pSelf)->handle_control(pSelf, iNodeID);
}

void 
dasher_editor_action_button(DasherEditor *pSelf, DasherAction *pAction) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->action_button)
    DASHER_EDITOR_GET_CLASS(pSelf)->action_button(pSelf, pAction);
}

void 
dasher_editor_actions_start(DasherEditor *pSelf) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->actions_start)
    DASHER_EDITOR_GET_CLASS(pSelf)->actions_start(pSelf);
}

bool 
dasher_editor_actions_more(DasherEditor *pSelf) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->actions_more)
    return DASHER_EDITOR_GET_CLASS(pSelf)->actions_more(pSelf);
  else
    return false;
}

void 
dasher_editor_actions_get_next(DasherEditor *pSelf, const gchar **szName, gint *iID, gboolean *bShow, gboolean *bControl, gboolean *bAuto) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->actions_get_next)
    DASHER_EDITOR_GET_CLASS(pSelf)->actions_get_next(pSelf, szName, iID, bShow, bControl, bAuto);
}

void 
dasher_editor_action_set_show(DasherEditor *pSelf, int iActionID, bool bValue) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->action_set_show)
    DASHER_EDITOR_GET_CLASS(pSelf)->action_set_show(pSelf, iActionID, bValue);
}

void 
dasher_editor_action_set_control(DasherEditor *pSelf, int iActionID, bool bValue) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->action_set_control)
    DASHER_EDITOR_GET_CLASS(pSelf)->action_set_control(pSelf, iActionID, bValue);
}

void 
dasher_editor_action_set_auto(DasherEditor *pSelf, int iActionID, bool bValue) { 
  if(DASHER_EDITOR_GET_CLASS(pSelf)->action_set_auto)
    DASHER_EDITOR_GET_CLASS(pSelf)->action_set_auto(pSelf, iActionID, bValue);
}

void 
dasher_editor_grab_focus(DasherEditor *pSelf) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->grab_focus)
    DASHER_EDITOR_GET_CLASS(pSelf)->grab_focus(pSelf);
}

void 
dasher_editor_output(DasherEditor *pSelf, const gchar *szText, int iOffset) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->output)
    DASHER_EDITOR_GET_CLASS(pSelf)->output(pSelf, szText, iOffset);
}

void 
dasher_editor_delete(DasherEditor *pSelf, int iLength, int iOffset) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->delete_text)
    DASHER_EDITOR_GET_CLASS(pSelf)->delete_text(pSelf, iLength, iOffset);
}

void 
dasher_editor_start_compose(DasherEditor *pSelf) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->start_compose)
    DASHER_EDITOR_GET_CLASS(pSelf)->start_compose(pSelf);
}

void 
dasher_editor_end_compose(DasherEditor *pSelf, bool bKeep) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->end_compose)
    DASHER_EDITOR_GET_CLASS(pSelf)->end_compose(pSelf, bKeep);
}


const gchar *
dasher_editor_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->get_context)
    return DASHER_EDITOR_GET_CLASS(pSelf)->get_context(pSelf, iOffset, iLength);
  else
    return NULL;
}

gint 
dasher_editor_get_offset(DasherEditor *pSelf) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->get_offset)
    return DASHER_EDITOR_GET_CLASS(pSelf)->get_offset(pSelf);
  else
    return 0;
}

gboolean 
dasher_editor_command(DasherEditor *pSelf, const gchar *szCommand) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->command)
    return DASHER_EDITOR_GET_CLASS(pSelf)->command(pSelf, szCommand);
  else
    return false;
}

gboolean 
dasher_editor_file_changed(DasherEditor *pSelf) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->file_changed)
    return DASHER_EDITOR_GET_CLASS(pSelf)->file_changed(pSelf);
  else
    return false;
}

const gchar *
dasher_editor_get_filename(DasherEditor *pSelf) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->get_filename)
    return DASHER_EDITOR_GET_CLASS(pSelf)->get_filename(pSelf);
  else
    return NULL;
}

const gchar *
dasher_editor_get_all_text(DasherEditor *pSelf) { 
  if(DASHER_EDITOR_GET_CLASS(pSelf)->get_all_text)
    return DASHER_EDITOR_GET_CLASS(pSelf)->get_all_text(pSelf);
  else
    return NULL;
}

const gchar *
dasher_editor_get_new_text(DasherEditor *pSelf) { 
  if(DASHER_EDITOR_GET_CLASS(pSelf)->get_new_text)
    return DASHER_EDITOR_GET_CLASS(pSelf)->get_new_text(pSelf);
  else
    return NULL;
}

void 
dasher_editor_handle_parameter_change(DasherEditor *pSelf, gint iParameter) {
  if(DASHER_EDITOR_GET_CLASS(pSelf)->handle_parameter_change)
    DASHER_EDITOR_GET_CLASS(pSelf)->handle_parameter_change(pSelf, iParameter);
}
