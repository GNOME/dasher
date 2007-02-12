#include "../Common/Common.h"

#include "Menu.h"
#include "dasher.h"
#include "DasherTypes.h"
#include "GtkDasherControl.h"

#include <glib/gi18n.h>
#ifdef GNOME_LIBS
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#endif

#include <iostream>

//extern GladeXML *widgets;

extern GtkWidget *open_filesel;
extern GtkWidget *save_filesel;
extern GtkWidget *save_and_quit_filesel;
extern GtkWidget *import_filesel;
extern GtkWidget *append_filesel;
static GtkWidget *window = NULL; // TODO: fix
extern GtkWidget *file_selector;

static gchar *filename = "foo"; // TODO: fix

DasherEditor *g_pEditor = NULL; // TODO: fix

extern "C" void select_save_file_as(GtkWidget * widget, gpointer user_data);

// 'File' Menu

#if GTK_CHECK_VERSION(2,3,0)

// extern "C" void select_new_file(GtkWidget *widget, gpointer user_data) {
//   //FIXME - confirm this. We should check whether the user wants to lose their work.

//   // TODO: Should just call a 'new' method in DasherEditor
//   dasher_editor_generate_filename(g_pEditor);
//   dasher_editor_clear(g_pEditor, false);

//   // TODO: Reimplement

// //   // Tell the widget we have a new buffer, and set the offset to 0
// //   gtk_dasher_control_set_buffer(GTK_DASHER_CONTROL(pDasherWidget), 0);

// //   // Starting a new file indicates a new user trial in detailed logging
// //   gtk_dasher_user_log_new_trial(GTK_DASHER_CONTROL(pDasherWidget));

// }

#endif

// #if GTK_CHECK_VERSION(2,3,0)

// extern "C" void select_open_file(GtkWidget *widget, gpointer user_data) {
//   GtkWidget *filesel = gtk_file_chooser_dialog_new(_("Select File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

// #ifdef GNOME_LIBS
//   gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filesel), FALSE);
// #endif

//   if(gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
// #ifdef GNOME_LIBS
//     char *filename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filesel));
// #else
//     char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
// #endif
//     dasher_editor_open(g_pEditor, filename);
//     g_free(filename);
//   }
//   gtk_widget_destroy(filesel);
// }

// #else

// extern "C" void filesel_hide(GtkWidget *widget, gpointer user_data) {
//   // FIXME - uh. Yes. This works, but is it in any way guaranteed to?
//   // Of course, if glade let us set user_data stuff properly, this would
//   // be a lot easier
//   gtk_widget_hide(gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(widget))));
// }

// extern "C" void open_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
//   filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
//   filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
//   dasher_editor_open(g_pEditor, filename);
// }

// extern "C" void select_open_file(GtkWidget *widget, gpointer user_data) {
//   if(open_filesel == NULL) {
//     open_filesel = glade_xml_get_widget(widgets, "open_fileselector");
//     g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(open_filesel)->ok_button), "clicked", G_CALLBACK(open_file_from_filesel), (gpointer) open_filesel);
//   }
//   gtk_window_set_transient_for(GTK_WINDOW(open_filesel), GTK_WINDOW(window));
//   gtk_window_present(GTK_WINDOW(open_filesel));
// }

// #endif

extern "C" void save_file(GtkWidget *widget, gpointer user_data) {
  if(filename != NULL) {
    dasher_editor_save_as(g_pEditor, filename,FALSE); // FIXME - REIMPLEMENT
  }
  else {
    select_save_file_as(NULL, NULL);
  }
}

#if GTK_CHECK_VERSION(2,3,0)

extern "C" void select_save_file_as(GtkWidget *widget, gpointer user_data) {
  GtkWidget *filesel = gtk_file_chooser_dialog_new(_("Select File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

#ifdef GNOME_LIBS
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filesel), FALSE);
#endif

  if(gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
#ifdef GNOME_LIBS
    char *filename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filesel));
#else
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
#endif
    dasher_editor_save_as(g_pEditor, filename,FALSE);
    g_free(filename);
  }
  gtk_widget_destroy(filesel);
}

#else

extern "C" void save_file_from_filesel_and_quit(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  if(dasher_editor_save_as(g_pEditor, filename, FALSE) == false) {

    // FIXME - do we really just want to fail silently if the save operation fails?

    return;
  }
  else {
    exiting = TRUE;
    gtk_main_quit();
  }
  return;
}

extern "C" void save_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
  dasher_editor_save_as(g_pEditor, filename, FALSE);
}

extern "C" void select_save_file_as(GtkWidget *widget, gpointer user_data) {
  if(save_filesel == NULL) {
    save_filesel = glade_xml_get_widget(widgets, "save_fileselector");
    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(save_filesel)->ok_button), "clicked", G_CALLBACK(save_file_from_filesel), (gpointer) save_filesel);
  }

  if(filename != NULL)
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(save_filesel), filename);
  gtk_window_set_transient_for(GTK_WINDOW(save_filesel), GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(save_filesel));
}

#endif

#if GTK_CHECK_VERSION(2,3,0)

extern "C" void select_append_file(GtkWidget *widget, gpointer user_data) {
  GtkWidget *filesel = gtk_file_chooser_dialog_new(_("Select File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

#ifdef GNOME_LIBS
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filesel), FALSE);
#endif

  if(gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
#ifdef GNOME_LIBS
    char *filename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filesel));
#else
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
#endif
    //    dasher_editor_save_as(g_pEditor, filename,TRUE);// FIXME - REIMPLEMENT
    g_free(filename);
  }
  gtk_widget_destroy(filesel);
}

#else

extern "C" void append_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));

  dasher_editor_save_as(g_pEditor, filename, TRUE);

  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
}

extern "C" void select_append_file(GtkWidget *widget, gpointer user_data) {
  if(append_filesel == NULL) {
    append_filesel = glade_xml_get_widget(widgets, "append_fileselector");
    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(append_filesel)->ok_button), "clicked", G_CALLBACK(append_file_from_filesel), (gpointer) append_filesel);
  }

  gtk_window_set_transient_for(GTK_WINDOW(append_filesel), GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(append_filesel));
}

#endif

#if GTK_CHECK_VERSION(2,3,0)

extern "C" void select_import_file(GtkWidget *widget, gpointer user_data) {
  // Moved elsewhere
}

#else

// Minimu GTK 2.4 now required

// extern "C" void import_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
//   filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
//   // TODO: Reimplement
//   //  gtk_dasher_control_train(GTK_DASHER_CONTROL(pDasherWidget), filename);
//   filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
// }

// extern "C" void select_import_file(GtkWidget *widget, gpointer user_data) {
//   if(import_filesel == NULL) {
//     import_filesel = glade_xml_get_widget(widgets, "import_fileselector");

//     g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(import_filesel)->ok_button), "clicked", G_CALLBACK(import_file_from_filesel), (gpointer) import_filesel);
//   }

//   gtk_window_set_transient_for(GTK_WINDOW(import_filesel), GTK_WINDOW(window));
//   gtk_window_present(GTK_WINDOW(import_filesel));
// }

#endif

#if !GTK_CHECK_VERSION(2,3,0)

// FIXME - I believe that this doesn't ever get called

extern "C" void select_save_file_as_and_quit(GtkWidget *widget, gpointer user_data) {
  if(save_and_quit_filesel == NULL) {
    save_and_quit_filesel = glade_xml_get_widget(widgets, "save_and_quit_fileselector");
    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(save_and_quit_filesel)->ok_button), "clicked", G_CALLBACK(save_file_from_filesel_and_quit), (gpointer) save_and_quit_filesel);
  }

  if(filename != NULL)
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(save_and_quit_filesel), filename);
  gtk_window_set_transient_for(GTK_WINDOW(save_and_quit_filesel), GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(save_and_quit_filesel));
}

#endif

extern "C" void save_file_and_quit(GtkWidget *widget, gpointer user_data) {
  if(filename != NULL) {
    if (dasher_editor_save_as(g_pEditor, filename,FALSE)==true) {
    //  exiting=TRUE;
    gtk_main_quit();
    } else {
      return;
    }
  }
  else {
    select_save_file_as(NULL, NULL);
    gtk_main_quit();
  }
}

extern "C" bool ask_save_before_exit(GtkWidget *widget, gpointer data) {
  // Moved elsewhere
}

// 'Edit' menu

// extern "C" void clipboard_cut(void) {
//   dasher_editor_clipboard(g_pEditor, CLIPBOARD_CUT);       // FIXME - give this a better name
// }

// extern "C" void clipboard_copy(void) {
//   dasher_editor_clipboard(g_pEditor, CLIPBOARD_COPY);
// }

// extern "C" void clipboard_paste(void) {
//   dasher_editor_clipboard(g_pEditor, CLIPBOARD_PASTE);
// }

// extern "C" void clipboard_copy_all(void) {
//   dasher_editor_clipboard(g_pEditor, CLIPBOARD_COPYALL);
// }

extern "C" void on_menututorial_activate() {
  dasher_editor_start_tutorial(g_pEditor);
}

// Not actually on the menu, but should be?

// extern "C" void clipboard_select_all(void) {
//   dasher_editor_clipboard(g_pEditor, CLIPBOARD_SELECTALL);
// }

// 'Options' Menu

// 'Preferences' item is in Preferences.cpp

// Font selector options will be in their own file

// extern "C" void set_dasher_fontsize(GtkWidget *widget, gpointer user_data) {
// //   if(GTK_TOGGLE_BUTTON(widget)->active == TRUE) {
// //     if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "fontsizenormal")) {
// //       gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_DASHER_FONTSIZE, Dasher::Opts::Normal);
// //     }
// //     else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "fontsizelarge")) {
// //       gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_DASHER_FONTSIZE, Dasher::Opts::Big);
// //     }
// //     else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "fontsizevlarge")) {
// //       gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_DASHER_FONTSIZE, Dasher::Opts::VBig);
// //     }
// //   }
// }

// extern "C" void reset_fonts(GtkWidget *widget, gpointer user_data) {
//   dasher_app_settings_reset(g_pDasherAppSettings, SP_DASHER_FONT);
//   dasher_app_settings_reset(g_pDasherAppSettings, APP_SP_EDIT_FONT);
// }

// 'Help' Menu

// extern "C" void about_dasher(GtkWidget *widget, gpointer user_data) {
//   // Moved elsewhere
// }

// // TODO: Need to hide menu item if GNOME_LIBS not defined
// extern "C" void show_help(GtkWidget *widget, gpointer user_data) {
//   // Moved elsewhere
// }
