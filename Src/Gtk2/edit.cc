#include "../Common/Common.h"

#include "Output.h"
#include "edit.h"
#include "dasher.h"
#include "accessibility.h"
#include "DasherControl.h"
#include "AppSettings.h"
#include "dasher_internal_buffer.h"

#ifdef GNOME_A11Y
#include "dasher_external_buffer.h"
#endif

#include <iostream>
#include <gdk/gdkx.h>

DasherEditor *g_pEditor;

GtkWidget *the_text_view;
GtkTextBuffer *the_text_buffer;

KeySym *origkeymap;
int modifiedkey = 0;
int numcodes;

// gunichar *wideoutput;

// extern gint outputcharacters;
// extern gboolean file_modified;

/// Whether to ignore the next cursor movement event

gboolean g_bIgnoreCursorMove( false );

/// Whether to forward keyboard events

gboolean g_bForwardKeyboard(false);


// void edit_move(int iDirection, int iDist);
// void edit_delete(int iDirection, int iDist);

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




void initialise_edit(GladeXML *pGladeXML) {
  the_text_view = glade_xml_get_widget(pGladeXML, "the_text_view");

  int min, max;
  Display *dpy = gdk_x11_get_default_xdisplay();

  the_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(the_text_view));

  g_pEditor = dasher_editor_new(GTK_TEXT_VIEW(the_text_view), GTK_VBOX(glade_xml_get_widget(pGladeXML, "vbox39")));

  // We need to monitor the text buffer for mark_set in order to get
  // signals when the cursor is moved

  
  //  gtk_widget_add_events(GTK_WIDGET(the_text_view), GDK_BUTTON_RELEASE_MASK);
  g_signal_connect(G_OBJECT(the_text_view), "button-release-event", G_CALLBACK(take_real_focus), NULL);

  g_signal_connect(G_OBJECT(the_text_view), "key-press-event", G_CALLBACK(edit_key_press), NULL);
  g_signal_connect(G_OBJECT(the_text_view), "key-release-event", G_CALLBACK(edit_key_release), NULL);

#ifdef X_HAVE_UTF8_STRING
  XDisplayKeycodes(dpy, &min, &max);
  origkeymap = XGetKeyboardMapping(dpy, min, max - min + 1, &numcodes);
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
  dasher_editor_output(g_pEditor, szText);
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

extern "C" void gtk2_edit_delete_callback(GtkDasherControl *pDasherControl, const gchar *szText, gpointer user_data) {
  gint displaylength = g_utf8_strlen(szText, -1);
  dasher_editor_delete(g_pEditor, displaylength);
}

// TODO: is this ever called? (probably yes, for internal buffer)
void clear_edit() {
  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer), start, 0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer), end, -1);

  gtk_text_buffer_delete(the_text_buffer, start, end);

}

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
