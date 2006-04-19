#include <gtk/gtk.h>
#include <string.h>

#include "dasher.h"
#ifndef WITH_MAEMO
#include "dasher_action_copy.h"
#endif
#ifdef GNOME_A11Y
#include "dasher_action_keyboard.h"
#endif 
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
#include "fileops.h"
#include "FontDialogues.h"
#include "GtkDasherControl.h"
#include "Preferences.h"

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
void dasher_editor_display_message(DasherEditor *pSelf, DasherMessageInfo *pMessageInfo);

// Private methods not in class
extern "C" void action_button_callback(GtkWidget *pWidget, gpointer pUserData);
extern "C" void context_changed_handler(GObject *pSource, gpointer pUserData);

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
  IDasherBufferSet *pBufferSet;
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

  ((DasherEditorPrivate *)(pDasherControl->private_data))->pBufferSet = 0;
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

  if(pPrivate->pBufferSet)
    g_object_unref(G_OBJECT(pPrivate->pBufferSet));

  delete (DasherEditorPrivate *)(((DasherEditor *)pObject)->private_data);
}

DasherEditor *dasher_editor_new(int argc, char **argv) {
  DasherEditor *pDasherControl;
  pDasherControl = (DasherEditor *)(g_object_new(dasher_editor_get_type(), NULL));
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pDasherControl->private_data);

  g_pDasherMain = dasher_main_new();

  GladeXML *pGladeXML = dasher_main_get_glade(g_pDasherMain);
  pDasherWidget = glade_xml_get_widget(pGladeXML, "DasherControl");
  
  g_pDasherAppSettings = dasher_app_settings_new(argc, argv);
  dasher_main_set_app_settings(g_pDasherMain, g_pDasherAppSettings);

  // TODO: Make lock diaogue a full method
  dasher_lock_dialogue_new(pGladeXML, GTK_WINDOW(dasher_main_get_window(g_pDasherMain)));
  // TODO: Bring into object framework
  InitialiseFontDialogues(pGladeXML);
  
  g_pPreferencesDialogue = dasher_preferences_dialogue_new(pGladeXML, pDasherControl);

  GtkTextView *pTextView = GTK_TEXT_VIEW(glade_xml_get_widget(pGladeXML, "the_text_view"));
  GtkVBox *pActionPane = GTK_VBOX(glade_xml_get_widget(pGladeXML, "vbox39"));

  pPrivate->pTextView = pTextView;
  pPrivate->pBuffer = gtk_text_view_get_buffer(pTextView);
  pPrivate->pActionPane = pActionPane;
  pPrivate->pTextClipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  pPrivate->pPrimarySelection = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  pPrivate->pActionRing = NULL;
  pPrivate->iNextActionID = 0;

  dasher_editor_setup_actions(pDasherControl);
  dasher_preferences_dialogue_populate_actions(g_pPreferencesDialogue);

  dasher_editor_create_buffer(pDasherControl);

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
  // The edit box keeps track of where we started 

  // TODO: This should be filtered through the buffer, rather than directly to the edit box
  //  set_mark();
}

void dasher_editor_handle_control(DasherEditor *pSelf, int iNodeID) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

  if(iNodeID == Dasher::CControlManager::CTL_USER + 1)
    dasher_editor_clear(pSelf, false); // Clear node is a special case (it shouldn't be)
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

  for(int i(0); i < sizeof(sMap)/sizeof(struct SControlMap); ++i) {
    if(sMap[i].iEvent == iNodeID) {
      if(sMap[i].bDelete) 
	idasher_buffer_set_edit_delete(pPrivate->pBufferSet, sMap[i].iDir, sMap[i].iDist);
      else
	idasher_buffer_set_edit_move(pPrivate->pBufferSet, sMap[i].iDir, sMap[i].iDist);	
    }
  }
}

void dasher_editor_handle_parameter_change(DasherEditor *pSelf, int iParameter) {
  switch(iParameter) {
  case APP_LP_STYLE:
    dasher_editor_create_buffer(pSelf);
    break;
  }

  dasher_preferences_dialogue_handle_parameter_change(g_pPreferencesDialogue, iParameter);
  dasher_main_handle_parameter_change(g_pDasherMain, iParameter);
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

// TODO: We shouldn't need to know about the buffer here - make this a method of the buffer set
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

  if(IS_DASHER_INTERNAL_BUFFER(pPrivate->pBufferSet))
    dasher_internal_buffer_clear(DASHER_INTERNAL_BUFFER(pPrivate->pBufferSet));
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

void dasher_editor_output(DasherEditor *pSelf, const gchar *szText) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);
  idasher_buffer_set_insert(pPrivate->pBufferSet, szText);
}

void dasher_editor_delete(DasherEditor *pSelf, int iLength) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);
  idasher_buffer_set_delete(pPrivate->pBufferSet, iLength);
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

void dasher_editor_create_buffer(DasherEditor *pSelf) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

  if(pPrivate->pBufferSet != 0) {
    g_object_unref(G_OBJECT(pPrivate->pBufferSet));
  }
  
  if(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 2)
#ifdef GNOME_A11Y
    pPrivate->pBufferSet = IDASHER_BUFFER_SET(dasher_external_buffer_new());
#else
  pPrivate->pBufferSet = 0;
#endif
  else
    pPrivate->pBufferSet = dasher_editor_get_buffer_set(pSelf);
  
  g_signal_connect(G_OBJECT(pPrivate->pBufferSet), "context_changed", G_CALLBACK(context_changed_handler), NULL);
}


void dasher_editor_refresh_context(DasherEditor *pSelf, int iMaxLength) {
  DasherEditorPrivate *pPrivate = (DasherEditorPrivate *)(pSelf->private_data);

  gchar *szContext = idasher_buffer_set_get_context(pPrivate->pBufferSet, iMaxLength);
  
  if(szContext && (strlen(szContext) > 0))
    gtk_dasher_control_set_context( GTK_DASHER_CONTROL(pDasherWidget), szContext );
}

void dasher_editor_display_message(DasherEditor *pSelf, DasherMessageInfo *pMessageInfo) {
  GtkMessageDialog *pDialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, pMessageInfo->szMessage));
  gtk_dialog_run(GTK_DIALOG(pDialog));
  gtk_widget_destroy(GTK_WIDGET(pDialog));
}

void dasher_editor_generate_filename(DasherEditor *pSelf) {
  if( dasher_app_settings_get_bool(g_pDasherAppSettings,  APP_BP_TIME_STAMP )) {
    // Build a filename based on the current time and date
    tm *t_struct;
    time_t ctime;
    char cwd[1000];
    char tbuffer[200];

    ctime = time(NULL);

    t_struct = localtime(&ctime);

    if(filename) {
      g_free((void *)filename);
    }

    getcwd(cwd, 1000);
    snprintf(tbuffer, 200, "dasher-%04d%02d%02d-%02d%02d.txt", (t_struct->tm_year + 1900), (t_struct->tm_mon + 1), t_struct->tm_mday, t_struct->tm_hour, t_struct->tm_min);

    filename = g_build_path("/", cwd, tbuffer, NULL);
  }
  else {
    if(filename) {
      g_free((void *)filename);
    }
    filename = NULL;
  }

  // TODO: Rationalise this - should probably be in 'new' function rather than here
  dasher_main_set_filename(g_pDasherMain, filename);
}

// TODO: Rationalise this
void dasher_editor_open(DasherEditor *pSelf, const gchar *szFilename) {
  open_file(szFilename);
}

// TODO: Rationalise this
bool dasher_editor_save_as(DasherEditor *pSelf, const gchar *szFilename, bool bAppend) {
  return save_file_as(szFilename, bAppend);
}

// Callbacks

extern "C" void action_button_callback(GtkWidget *pWidget, gpointer pUserData) { 
  void **pPointers((void **)pUserData);
  dasher_editor_action_button((DasherEditor *)pPointers[0], (DasherAction *)pPointers[1]);
}

extern "C" void parameter_notification(GtkDasherControl *pDasherControl, gint iParameter, gpointer data) { 
  if(g_pEditor)
    dasher_editor_handle_parameter_change(g_pEditor, iParameter);
}

extern "C" void handle_start_event(GtkDasherControl *pDasherControl, gpointer data) { 
  if(g_pEditor)
    dasher_editor_handle_start(g_pEditor);
}

extern "C" void handle_stop_event(GtkDasherControl *pDasherControl, gpointer data) {
  if(g_pEditor)
    dasher_editor_handle_stop(g_pEditor);
}

extern "C" void handle_context_request(GtkDasherControl * pDasherControl, gint iMaxLength, gpointer data) { 
  if(g_pEditor)
    dasher_editor_refresh_context(g_pEditor, iMaxLength);
}

extern "C" void handle_control_event(GtkDasherControl *pDasherControl, gint iEvent, gpointer data) { 
  if(g_pEditor)
    dasher_editor_handle_control(g_pEditor, iEvent);
}

extern "C" void on_message(GtkDasherControl *pDasherControl, gpointer pMessageInfo, gpointer pUserData) {
 if(g_pEditor)
   dasher_editor_display_message(g_pEditor, (DasherMessageInfo *)pMessageInfo);
}

// TODO: The following two should probably be made the same
extern "C" void handle_request_settings(GtkDasherControl * pDasherControl, gpointer data) {
  dasher_preferences_dialogue_show(g_pPreferencesDialogue);
}

extern "C" void preferences_display(GtkWidget *widget, gpointer user_data) {
  dasher_preferences_dialogue_show(g_pPreferencesDialogue);
}

extern "C" void gtk2_edit_delete_callback(GtkDasherControl *pDasherControl, const gchar *szText, gpointer user_data) {
  gint displaylength = g_utf8_strlen(szText, -1);
  dasher_editor_delete(g_pEditor, displaylength);
}

extern "C" void gtk2_edit_output_callback(GtkDasherControl *pDasherControl, const gchar *szText, gpointer user_data) {
  dasher_editor_output(g_pEditor, szText);
}

// TODO: This should call back into editor, not directly into Dasher control
extern "C" void context_changed_handler(GObject *pSource, gpointer pUserData) {
  gtk_dasher_control_invalidate_context(GTK_DASHER_CONTROL(pDasherWidget));
}
