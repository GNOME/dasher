#include "../Common/Common.h"

#include <gtk/gtk.h>

// Code for managing file selection dialogues - obsolete after GNOME 2.3

#if !GTK_CHECK_VERSION(2,3,0)

extern "C" void import_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  load_training_file(filename);
  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
}

#endif

#if !GTK_CHECK_VERSION(2,3,0)

extern "C" void open_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
  open_file(filename);
}

#endif

#if !GTK_CHECK_VERSION(2,3,0)

extern "C" void save_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
  save_file_as(filename, FALSE);
}

#endif

#if !GTK_CHECK_VERSION(2,3,0)

extern "C" void save_file_from_filesel_and_quit(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  if(save_file_as(filename, FALSE) == false) {
    return;
  }
  else {
    exiting = TRUE;
    gtk_main_quit();
  }
  return true;
}

#endif

#if !GTK_CHECK_VERSION(2,3,0)

extern "C" void append_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));

  save_file_as(filename, TRUE);

  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
}

#endif

#if !GTK_CHECK_VERSION(2,3,0)

extern "C" void filesel_hide(GtkWidget *widget, gpointer user_data) {
  // FIXME - uh. Yes. This works, but is it in any way guaranteed to?
  // Of course, if glade let us set user_data stuff properly, this would
  // be a lot easier
  gtk_widget_hide(gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(widget))));
}
#endif
