#include <gtk/gtk.h>
#include <string.h>

#include "dasher.h"

#include "dasher_editor.h"
#include "dasher_internal_buffer.h"
#include "GtkDasherControl.h"

#ifdef GNOME_SPEECH
#include "dasher_action_speech.h"
#endif 

#ifdef GNOME_A11Y
#include "dasher_action_keyboard.h"
#endif 

#ifdef WITH_MAEMO
#include "dasher_action_keyboard_maemo.h"
#else
#include "dasher_action_copy.h"
#include "dasher_action_script.h"
#endif

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
};

static void dasher_editor_class_init(DasherEditorClass *pClass);
static void dasher_editor_init(DasherEditor *pEditor);
static void dasher_editor_destroy(GObject *pObject);

void dasher_editor_select_all(DasherEditor *pSelf);
void dasher_editor_setup_actions(DasherEditor *pSelf);
void dasher_editor_add_action(DasherEditor *pSelf, DasherAction *pNewAction);
EditorAction *dasher_editor_get_action_by_id(DasherEditor *pSelf, int iID);
void dasher_editor_rebuild_action_pane(DasherEditor *pSelf);

extern "C" void action_button_callback(GtkWidget *pWidget, gpointer pUserData);

typedef struct _DasherEditorPrivate DasherEditorPrivate;

struct _DasherEditorPrivate {
  GtkTextView *pTextView;
  GtkTextBuffer *pBuffer;
  GtkVBox *pActionPane;
  GtkClipboard *pTextClipboard;
  GtkClipboard *pPrimarySelection;
  EditorAction *pActionRing;
  EditorAction *pActionIter;
  gboolean bActionIterStarted;
  gint iNextActionID;
};

GType dasher_editor_get_type() {
  static GType dasher_editor_type = 0;

  if(!dasher_editor_type) {
    static const GTypeInfo dasher_editor_info = {
      sizeof(DasherEditorClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_editor_class_init,
      NULL,
      NULL,
      sizeof(DasherEditor),
      0,
      (GInstanceInitFunc) dasher_editor_init,
      NULL
    };

    dasher_editor_type = g_type_register_static(G_TYPE_OBJECT, "DasherEditor", &dasher_editor_info, static_cast < GTypeFlags > (0));
  }

  return dasher_editor_type;
}

static void dasher_editor_class_init(DasherEditorClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_editor_destroy;
}

static void dasher_editor_init(DasherEditor *pDasherControl) {
  pDasherControl->private_data = new DasherEditorPrivate;
}

static void dasher_editor_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(((DasherEditor *)pObject)->private_data);

  EditorAction *pCurrentAction = pPrivate->pActionRing;
  bool bStarted = false;
  
  while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
    bStarted = true;
    g_object_unref(G_OBJECT(pCurrentAction->pAction));
    pCurrentAction = pCurrentAction->pNext;
  }

  delete (DasherEditorPrivate *)(((DasherEditor *)pObject)->private_data);
}

DasherEditor *dasher_editor_new(GtkTextView *pTextView, GtkVBox *pActionPane) {
  DasherEditor *pDasherControl;

  pDasherControl = (DasherEditor *)(g_object_new(dasher_editor_get_type(), NULL));

  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pDasherControl->private_data);
  pPrivate->pTextView = pTextView;
  pPrivate->pBuffer = gtk_text_view_get_buffer(pTextView);
  pPrivate->pActionPane = pActionPane;
  pPrivate->pTextClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  pPrivate->pPrimarySelection = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  pPrivate->pActionRing = NULL;
  pPrivate->iNextActionID = 0;

  dasher_editor_setup_actions(pDasherControl);

  return pDasherControl;
}

IDasherBufferSet *dasher_editor_get_buffer_set(DasherEditor *pSelf) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);
  return IDASHER_BUFFER_SET(dasher_internal_buffer_new(pPrivate->pTextView));
}


void dasher_editor_clipboard(DasherEditor *pSelf, clipboard_action act) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

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
    dasher_editor_select_all(pSelf);
    break;
  case CLIPBOARD_CLEAR:
    gtk_text_buffer_set_text(pPrivate->pBuffer, "", 0);
    break;
  }
  g_free(the_text);

  delete start;
  delete end;
}

void dasher_editor_select_all(DasherEditor *pSelf) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);
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

void dasher_editor_handle_stop(DasherEditor *pSelf) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);
  
  // See if anything is set to auto:
  EditorAction *pCurrentAction = pPrivate->pActionRing;
  bool bStarted = false;
  
  while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
    bStarted = true;
    if(pCurrentAction->bAuto)
      dasher_action_execute(pCurrentAction->pAction, dasher_editor_get_all_text(pSelf)); 
    pCurrentAction = pCurrentAction->pNext;
  }
}

void dasher_editor_handle_start(DasherEditor *pSelf) {
}

void dasher_editor_handle_control(DasherEditor *pSelf, int iNodeID) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

  if(iNodeID == Dasher::CControlManager::CTL_USER + 1)
    dasher_editor_clear(pSelf, false); // Clear node is a special case
  else {
    EditorAction *pCurrentAction = pPrivate->pActionRing;
    bool bStarted = false;
    
    while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
      bStarted = true;
      if(pCurrentAction->iControlID == iNodeID) {
	dasher_action_execute(pCurrentAction->pAction, dasher_editor_get_all_text(pSelf)); 
	dasher_editor_clear(pSelf, true); 
      }
      pCurrentAction = pCurrentAction->pNext;
    }
  }
}

void dasher_editor_add_action(DasherEditor *pSelf, DasherAction *pNewAction) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

  EditorAction *pNewEditorAction = new EditorAction;
  pNewEditorAction->pAction = pNewAction;
  pNewEditorAction->iID = pPrivate->iNextActionID;
  ++pPrivate->iNextActionID;

  // TODO: Need to get/set registry keys for these
  pNewEditorAction->bShow = true;
  pNewEditorAction->bControl = true;
  pNewEditorAction->bAuto = false;

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
}

void dasher_editor_setup_actions(DasherEditor *pSelf) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);
  
  // TODO: Activate and deactivate methods for actions
  // TODO: Clear shouldn't be a special case (include support for false in clear method)

#ifdef GNOME_SPEECH
  dasher_editor_add_action(pSelf, DASHER_ACTION(dasher_action_speech_new()));
#endif

#ifdef GNOME_A11Y
  dasher_editor_add_action(pSelf, DASHER_ACTION(dasher_action_keyboard_new()));
#endif

#ifdef WITH_MAEMO
  dasher_editor_add_action(pSelf, DASHER_ACTION(dasher_action_keyboard_maemo_new()));
#else
  dasher_editor_add_action(pSelf, DASHER_ACTION(dasher_action_copy_new(pSelf)));

  GDir *pDirectory;
  G_CONST_RETURN gchar *szFilename;

  gchar *szUserScriptDir = new gchar[strlen(dasher_app_settings_get_string(g_pDasherAppSettings, SP_USER_LOC))+9];
  strcpy(szUserScriptDir, dasher_app_settings_get_string(g_pDasherAppSettings, SP_USER_LOC));
  strcat(szUserScriptDir, "scripts/");

  pDirectory = g_dir_open(szUserScriptDir, 0, NULL);

  if(pDirectory) {
    while((szFilename = g_dir_read_name(pDirectory))) {
      dasher_editor_add_action(pSelf, DASHER_ACTION(dasher_action_script_new(szUserScriptDir, szFilename)));
    }
    
    g_dir_close(pDirectory);
  }
  
  delete[] szUserScriptDir;

  gchar *szSystemScriptDir = new gchar[strlen(dasher_app_settings_get_string(g_pDasherAppSettings, SP_SYSTEM_LOC))+9];
  strcpy(szSystemScriptDir, dasher_app_settings_get_string(g_pDasherAppSettings, SP_SYSTEM_LOC));
  strcat(szSystemScriptDir, "scripts/");

  pDirectory = g_dir_open(szSystemScriptDir, 0, NULL);

  if(pDirectory) {
    while((szFilename = g_dir_read_name(pDirectory))) {
      dasher_editor_add_action(pSelf, DASHER_ACTION(dasher_action_script_new(szSystemScriptDir, szFilename)));
    }
    
    g_dir_close(pDirectory);
  }

  delete[] szSystemScriptDir;
#endif

  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER, "Actions", -1 );
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER, Dasher::CControlManager::CTL_ROOT, -2);
  int iControlOffset(1);

  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset, "Clear", -1 );
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset, Dasher::CControlManager::CTL_USER, -2);
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), -1, Dasher::CControlManager::CTL_USER + iControlOffset, -2);
  ++iControlOffset;

  EditorAction *pCurrentAction = pPrivate->pActionRing;
  bool bStarted = false;

  while(!bStarted || (pCurrentAction != pPrivate->pActionRing)) {
    bStarted = true;
  
    gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset, dasher_action_get_name(pCurrentAction->pAction), -1 );
    gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER + iControlOffset, Dasher::CControlManager::CTL_USER, -2);
    gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), -1, Dasher::CControlManager::CTL_USER + iControlOffset, -2);
    pCurrentAction->iControlID = Dasher::CControlManager::CTL_USER + iControlOffset;
    ++iControlOffset;
    
    pCurrentAction = pCurrentAction->pNext;
  }

  dasher_editor_rebuild_action_pane(pSelf);
}

extern "C" void delete_children_callback(GtkWidget *pWidget, gpointer pUserData) {
  gtk_widget_destroy(pWidget);
}

void dasher_editor_rebuild_action_pane(DasherEditor *pSelf) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);
  
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

const gchar *dasher_editor_get_all_text(DasherEditor *pSelf) { 
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

  GtkTextIter oStart;
  GtkTextIter oEnd;

  gtk_text_buffer_get_start_iter(pPrivate->pBuffer, &oStart);
  gtk_text_buffer_get_end_iter(pPrivate->pBuffer, &oEnd);

  return gtk_text_buffer_get_text(pPrivate->pBuffer, &oStart, &oEnd, false );
}

void dasher_editor_action_button(DasherEditor *pSelf, DasherAction *pAction) {
  if(pAction) {
    dasher_action_execute(pAction, dasher_editor_get_all_text(pSelf)); 
    dasher_editor_clear(pSelf, true);
  }
  else { // Clear button
    dasher_editor_clear(pSelf, false);
  }
}

void dasher_editor_clear(DasherEditor *pSelf, gboolean bStore) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

  GtkTextIter oStart;
  GtkTextIter oEnd;

  gtk_text_buffer_get_start_iter(pPrivate->pBuffer, &oStart);
  gtk_text_buffer_get_end_iter(pPrivate->pBuffer, &oEnd);
  gtk_text_buffer_delete(pPrivate->pBuffer, &oStart, &oEnd);
}


void dasher_editor_actions_start(DasherEditor *pSelf) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

  pPrivate->bActionIterStarted = false;
  pPrivate->pActionIter = pPrivate->pActionRing;
}

bool dasher_editor_actions_more(DasherEditor *pSelf) {
 DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

 return(!pPrivate->bActionIterStarted || (pPrivate->pActionIter != pPrivate->pActionRing));
}

void dasher_editor_actions_get_next(DasherEditor *pSelf, const gchar **szName, gint *iID, gboolean *bShow, gboolean *bControl, gboolean *bAuto) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);
  
  *szName = dasher_action_get_name(pPrivate->pActionIter->pAction);
  *iID = pPrivate->pActionIter->iID;
  *bShow = pPrivate->pActionIter->bShow; 
  *bControl = pPrivate->pActionIter->bControl;
  *bAuto = pPrivate->pActionIter->bAuto;
  
  pPrivate->pActionIter = pPrivate->pActionIter->pNext;
  pPrivate->bActionIterStarted = true;
}

void dasher_editor_action_set_show(DasherEditor *pSelf, int iActionID, bool bValue) {
  EditorAction *pAction;
  pAction = dasher_editor_get_action_by_id(pSelf, iActionID);

  if(pAction) {
    pAction->bShow = bValue;
    dasher_editor_rebuild_action_pane(pSelf);
  }
}

void dasher_editor_action_set_control(DasherEditor *pSelf, int iActionID, bool bValue) {
  // TODO: Need to actually change behaviour in resonse to these calls

  EditorAction *pAction;
  pAction = dasher_editor_get_action_by_id(pSelf, iActionID);
  
  if(pAction) {
    pAction->bControl = bValue;
    
    if(bValue)
      gtk_dasher_control_connect_node(GTK_DASHER_CONTROL(pDasherWidget), pAction->iControlID, Dasher::CControlManager::CTL_USER, -2);
    else
      gtk_dasher_control_disconnect_node(GTK_DASHER_CONTROL(pDasherWidget), pAction->iControlID, Dasher::CControlManager::CTL_USER);
  }
}

void dasher_editor_action_set_auto(DasherEditor *pSelf, int iActionID, bool bValue) { 
EditorAction *pAction;
  pAction = dasher_editor_get_action_by_id(pSelf, iActionID);

  if(pAction) {
    pAction->bAuto = bValue;
  }
}

EditorAction *dasher_editor_get_action_by_id(DasherEditor *pSelf, int iID){
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

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

extern "C" void action_button_callback(GtkWidget *pWidget, gpointer pUserData) {
  void **pPointers((void **)pUserData);
  dasher_editor_action_button((DasherEditor *)pPointers[0], (DasherAction *)pPointers[1]);
}
