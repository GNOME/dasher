#include "edit.h"

GtkWidget *the_text_view;  
GtkTextBuffer *the_text_buffer;
GtkClipboard *the_text_clipboard;

void initialise_edit()
{
  flush_count = 0;

  the_text_clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  the_text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (the_text_view), TRUE);
  the_text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (the_text_view));

  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (the_text_view), GTK_WRAP_WORD);


  // FIXME - need to make this work
  //  g_signal_connect(G_OBJECT(the_text_view), "button_press_event", G_CALLBACK(handle_cursor_move), (gpointer) this);
}
