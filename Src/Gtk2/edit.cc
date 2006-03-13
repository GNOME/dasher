#include "../Common/Common.h"

#include "Output.h"
#include "edit.h"
#include "dasher.h"
#include "accessibility.h"
#include "DasherControl.h"
#include "AppSettings.h"
#include "dasher_internal_buffer.h"
#include "dasher_external_buffer.h"

#include <iostream>
#include <gdk/gdkx.h>

#ifdef GNOME_SPEECH
#include "speech.h"
#endif

IDasherBufferSet *g_pBufferSet = 0;
DasherEditor *g_pEditor;

GtkWidget *the_text_view;
GtkTextBuffer *the_text_buffer;
std::string last_said;
std::string say;
std::string pipetext;
std::string outputtext;
KeySym *origkeymap;
int modifiedkey = 0;
int numcodes;

#ifdef GNOME_A11Y
AccessibleText *textbox = NULL;
AccessibleEditableText *edittextbox = NULL;
#endif

//GtkWidget *text_view;
GtkWidget *text_scrolled_window;

gunichar *wideoutput;

extern gint outputcharacters;
extern gboolean file_modified;

/// Whether to ignore the next cursor movement event

gboolean g_bIgnoreCursorMove( false );

/// Whether to forward keyboard events

gboolean g_bForwardKeyboard(false);


void edit_move(int iDirection, int iDist);
void edit_delete(int iDirection, int iDist);

// Old stuff (but quite probably still needed)

extern "C" gboolean take_real_focus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
extern "C" gboolean edit_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
extern "C" gboolean edit_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

extern "C" void context_changed_handler(GObject *pSource, gpointer pUserData);


extern "C" void choose_filename() {
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
    gtk_window_set_title(GTK_WINDOW(window), filename);
  }
  else {
    gtk_window_set_title(GTK_WINDOW(window), "Dasher");
    if(filename) {
      g_free((void *)filename);
    }
    filename = NULL;
  }
}

// extern "C" gboolean edit_button_release_event(GtkWidget *widget, GtkTextIter *pIter, GtkTextMark *pMark, gpointer data) {

//   // FIXME - this probably gets called a little too often...  (not a
//   // problem as we ignore requests which don't actually change the
//   // context, but probably should be fixed for efficientcy reasons).

//   if( g_bIgnoreCursorMove ) {
//     // If we're expecting a callback as a result of our own action, ignore it
//     return false;
//   }

// //   GtkTextIter start;
// //   GtkTextIter end; // Refers to end of context, which is start of selection!

// //   gtk_text_buffer_get_selection_bounds( the_text_buffer, &end, NULL );
// //   start = end;

// //   gtk_text_iter_backward_chars( &start, 10 );

// //   gchar *szContext( gtk_text_buffer_get_text( the_text_buffer, &start, &end, false ));

// //   if( gtk_text_iter_is_start( &start )) {

// //     // Urgh - I hate C style strings

// //     gchar *szContextNew = new gchar[strlen( szContext ) + 3];

// //     strcpy( szContextNew, ". " );
// //     strcat( szContextNew, szContext );

// //     gtk_dasher_control_set_context( GTK_DASHER_CONTROL(pDasherWidget), szContextNew );
    
// //     delete[] szContextNew;
    
// //   }
// //   else {
// //     gtk_dasher_control_set_context( GTK_DASHER_CONTROL(pDasherWidget), szContext );
// //   }
  


// //   g_free( szContext );


// //  gtk_dasher_control_invalidate_context(GTK_DASHER_CONTROL(pDasherWidget));

//   return FALSE;
// }




void RefreshContext(int iMaxLength) {
  gchar *szContext = idasher_buffer_set_get_context(g_pBufferSet, iMaxLength);
  
  if(szContext && (strlen(szContext) > 0))
    gtk_dasher_control_set_context( GTK_DASHER_CONTROL(pDasherWidget), szContext );
}


void initialise_edit(GladeXML *pGladeXML) {
  text_scrolled_window = glade_xml_get_widget(pGladeXML, "text_scrolled_window");
  the_text_view = glade_xml_get_widget(pGladeXML, "the_text_view");

  int min, max;
  Display *dpy = gdk_x11_get_default_xdisplay();

  the_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(the_text_view));

  g_pEditor = dasher_editor_new(GTK_TEXT_VIEW(the_text_view), GTK_VBOX(glade_xml_get_widget(pGladeXML, "vbox39")));

  create_buffer();


  // We need to monitor the text buffer for mark_set in order to get
  // signals when the cursor is moved

  
  //  gtk_widget_add_events(GTK_WIDGET(the_text_view), GDK_BUTTON_RELEASE_MASK);
  g_signal_connect(G_OBJECT(the_text_view), "button-release-event", G_CALLBACK(take_real_focus), NULL);

  g_signal_connect(G_OBJECT(the_text_view), "key-press-event", G_CALLBACK(edit_key_press), NULL);
  g_signal_connect(G_OBJECT(the_text_view), "key-release-event", G_CALLBACK(edit_key_release), NULL);


  set_editbox_font(dasher_app_settings_get_string(g_pDasherAppSettings, APP_SP_EDIT_FONT));


#ifdef X_HAVE_UTF8_STRING
  XDisplayKeycodes(dpy, &min, &max);
  origkeymap = XGetKeyboardMapping(dpy, min, max - min + 1, &numcodes);
#endif
}

void create_buffer() {
  // TODO: need to delete old buffer here

   if(g_pBufferSet != 0) {
     g_object_unref(G_OBJECT(g_pBufferSet));
   }

  // GRR - how the hell do I do this? Why isn't all this C++ as would be sensible?

  if(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 2)
    g_pBufferSet = IDASHER_BUFFER_SET(dasher_external_buffer_new());
  else
    g_pBufferSet = dasher_editor_get_buffer_set(g_pEditor);

  g_signal_connect(G_OBJECT(g_pBufferSet), "context_changed", G_CALLBACK(context_changed_handler), NULL);
}

void cleanup_edit() {
#ifdef X_HAVE_UTF8_STRING
  // We want to set the keymap back to whatever it was before,
  // if that's possible
  int min, max;
  Display *dpy = gdk_x11_get_default_xdisplay();
  XDisplayKeycodes(dpy, &min, &max);
  XChangeKeyboardMapping(dpy, min, numcodes, origkeymap, (max - min));
#endif
}

void set_mark() {
  GtkTextIter oBufferEnd;
  GtkTextIter oBufferStart;
  gtk_text_buffer_get_bounds( the_text_buffer, &oBufferStart, &oBufferEnd);
  gtk_text_buffer_create_mark(the_text_buffer, "new_start", &oBufferEnd, true);
}

const gchar *get_new_text() {
  GtkTextIter oNewStart;
  GtkTextIter oNewEnd;
  GtkTextIter oDummy;

  gtk_text_buffer_get_bounds( the_text_buffer, &oDummy, &oNewEnd);
  gtk_text_buffer_get_iter_at_mark( the_text_buffer, &oNewStart, gtk_text_buffer_get_mark(the_text_buffer, "new_start"));

  return gtk_text_buffer_get_text( the_text_buffer, &oNewStart, &oNewEnd, false );
  
}

const gchar *get_all_text() {
  GtkTextIter oStart;
  GtkTextIter oEnd;

  gtk_text_buffer_get_start_iter(the_text_buffer, &oStart);
  gtk_text_buffer_get_end_iter(the_text_buffer, &oEnd);

  return gtk_text_buffer_get_text( the_text_buffer, &oStart, &oEnd, false );
}

// void handle_cursor_move(GtkTextView *textview, GtkMovementStep arg1, gint arg2, gboolean arg3, gpointer data) {

//   // FIXME - REIMPLEMENT

//   // Let the core get new context and redraw it if the cursor is moved
// //   dasher_start();
// //   dasher_redraw();
// }

extern "C" void gtk2_edit_output_callback(GtkDasherControl *pDasherControl, const gchar *szText, gpointer user_data) {
  idasher_buffer_set_insert(g_pBufferSet, szText);
}

void write_to_file() {
  // FIXME - REIMPLEMENT

  // Add the text from the edit box to the user training file so we
  // can learn from them
//   std::string filename=dasher_get_training_file();
//   int fd=open(filename.c_str(),O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
//   write(fd,outputtext.c_str(),outputtext.length());
//   close(fd);
//   outputtext="";
}

bool edit_handle_control_event(gint iEvent) {

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
    if(sMap[i].iEvent == iEvent) {
      if(sMap[i].bDelete) 
	idasher_buffer_set_edit_delete(g_pBufferSet, sMap[i].iDir, sMap[i].iDist);
      else
	idasher_buffer_set_edit_move(g_pBufferSet, sMap[i].iDir, sMap[i].iDist);	
      return true;
    }
  }

  return false;
}

extern "C" void gtk2_edit_delete_callback(GtkDasherControl *pDasherControl, const gchar *szText, gpointer user_data) {
  gint displaylength = g_utf8_strlen(szText, -1);
  idasher_buffer_set_delete(g_pBufferSet, displaylength);
}

void clear_edit() {
  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer), start, 0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer), end, -1);

  gtk_text_buffer_delete(the_text_buffer, start, end);

}

// void gtk2_get_new_context_callback(std::string &str, int max) {
//   GtkTextIter *start = new GtkTextIter;
//   GtkTextIter *end = new GtkTextIter;

//   gtk_text_buffer_get_selection_bounds(the_text_buffer, start, end);

//   if(gtk_text_iter_get_offset(start) == gtk_text_iter_get_offset(end)) {
//     // if there's no slection, just get the context from the cursor
//     gtk_text_buffer_get_iter_at_mark(the_text_buffer, end, gtk_text_buffer_get_insert(the_text_buffer));
//     *start = *end;
//   }
//   else {
//     // otherwise, we want to get the context from the left hand edge of
//     // the selection rather than the right hand edge (which is where the 
//     // cursor is)
//     *end = *start;
//   }

//   gtk_text_iter_backward_chars(start, max);

//   str = std::string(gtk_text_buffer_get_text(the_text_buffer, start, end, FALSE));

//   delete start;
//   delete end;
// }

void outputpipe() {
  printf("%s", pipetext.c_str());
  fflush(stdout);
  pipetext = "";
}

#ifdef GNOME_SPEECH

// void speak() {
//   if(say.length() > 0) {
//     while(say.find("\"") != std::string::npos) {
//       say.replace(say.find("\""), 1, "");
//     }
//     SPEAK_DAMN_YOU(say.c_str());
//     last_said = say;
//   }
//   say = "";
// }

// void speak_last() {
//   // Repeat whatever it was that we last spoke
//   if(last_said.length() > 0) {
//     // Festival seems unhappy about quotes
//     while(last_said.find("\"") != std::string::npos) {
//       last_said.replace(last_said.find("\""), 1, "");
//     }
//     SPEAK_DAMN_YOU(last_said.c_str());
//   }
// }

// void speak_buffer() {
//   std::string buffer;
//   GtkTextIter *speak_start, *speak_end;

//   speak_start = new GtkTextIter;
//   speak_end = new GtkTextIter;

//   gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer), speak_start, 0);
//   gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer), speak_end, -1);

//   buffer = gtk_text_iter_get_slice(speak_start, speak_end);

//   if(buffer.length() > 0) {
//     while(buffer.find("\"") != std::string::npos) {
//       buffer.replace(buffer.find("\""), 1, "");
//     }
//     SPEAK_DAMN_YOU(buffer.c_str());
//     last_said = buffer;
//   }
// }
#endif

void set_editbox_font(std::string FontName) {
  if(FontName != "") {
    gtk_widget_modify_font(the_text_view, pango_font_description_from_string(FontName.c_str()));
  }
}

#ifdef GNOME_A11Y
void set_textbox(Accessible *newtextbox) {
  AccessibleText_unref(textbox);
  AccessibleEditableText_unref(textbox);
  if(newtextbox == NULL) {
    textbox = NULL;
    edittextbox = NULL;
  }
  else {
    AccessibleComponent *component;
    component = Accessible_getComponent(newtextbox);
    //    AccessibleComponent_grabFocus(component);
    textbox = Accessible_getText(newtextbox);
    edittextbox = Accessible_getEditableText(newtextbox);
  }
}
#endif

gboolean grab_focus() {
  gtk_widget_grab_focus(the_text_view);
  g_bForwardKeyboard = true;
  return true;
}

extern "C" gboolean take_real_focus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
  g_bForwardKeyboard = false;
  return false;
}

extern "C" gboolean edit_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
  if(g_bForwardKeyboard) {
    gboolean *returnType;
    g_signal_emit_by_name(GTK_OBJECT(pDasherWidget), "key_press_event", event, &returnType);
    return true;
  }
  else {
    return false;
  }
}

extern "C" gboolean edit_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data) { 
  if(g_bForwardKeyboard) {
    gboolean *returnType;
    g_signal_emit_by_name(GTK_OBJECT(pDasherWidget), "key_release_event", event, &returnType);
    return true;
  }
  else {
    return false;
  }
}

extern "C" void context_changed_handler(GObject *pSource, gpointer pUserData) {
  gtk_dasher_control_invalidate_context(GTK_DASHER_CONTROL(pDasherWidget));
}
