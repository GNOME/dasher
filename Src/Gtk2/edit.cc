#include "edit.h"

GtkWidget *the_text_view;  
GtkTextBuffer *the_text_buffer;
GtkClipboard *the_text_clipboard;
GtkFontSelectionDialog *editfontdialog;

int flush_count;

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

void edit_output_callback(symbol Symbol)
{
  std::string label;
  label = dasher_get_edit_text( Symbol );

  gtk_text_buffer_insert_at_cursor(the_text_buffer, label.c_str(), -1);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_flush_callback(symbol Symbol)
{
  std::string label;
  label = dasher_get_edit_text( Symbol );

  gtk_text_buffer_insert_at_cursor(the_text_buffer, label.c_str(), -1);

  if (label!="") {
    ++flush_count;
  }

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_unflush_callback()
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,end,gtk_text_buffer_get_insert(the_text_buffer));

  *start=*end;  

  gtk_text_iter_backward_chars(start, flush_count);

  gtk_text_buffer_delete(the_text_buffer,start,end);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
  flush_count=0;

}

void clipboard_callback( clipboard_action act )
{
  switch( act )
    {
    case CLIPBOARD_CUT: 
      gtk_text_buffer_cut_clipboard(the_text_buffer, the_text_clipboard, TRUE);
      break;
    case CLIPBOARD_COPY:
      gtk_text_buffer_copy_clipboard(the_text_buffer, the_text_clipboard);
      break;
    case CLIPBOARD_PASTE:
      gtk_text_buffer_paste_clipboard(the_text_buffer, the_text_clipboard, NULL, TRUE);
      break;
    case CLIPBOARD_COPYALL:
      select_all();
      gtk_text_buffer_copy_clipboard(the_text_buffer, the_text_clipboard);
      break;
    case CLIPBOARD_SELECTALL:
      select_all();
      break;
    case CLIPBOARD_CLEAR:
      gtk_text_buffer_set_text (the_text_buffer,"",0);
      break;
    }
}

void select_all()
{
  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),end,-1);

  GtkTextMark *selection = gtk_text_buffer_get_mark (the_text_buffer,"selection_bound");
  GtkTextMark *cursor = gtk_text_buffer_get_mark(the_text_buffer,"insert");

  gtk_text_buffer_move_mark(the_text_buffer,selection,start);
  gtk_text_buffer_move_mark(the_text_buffer,cursor,end);
}

void clear_edit()
{
  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),end,-1); 

  gtk_text_buffer_delete(the_text_buffer,start,end);

  flush_count = 0;

}

void get_edit_font_from_dialog( GtkWidget *one, GtkWidget *two )
{
  char *font_name;
  font_name=gtk_font_selection_dialog_get_font_name(editfontdialog);
  if (font_name) {
    gtk_widget_modify_font (the_text_view,pango_font_description_from_string(font_name));
  }
  gtk_widget_destroy (GTK_WIDGET(editfontdialog));
}

void reset_edit_font()
{
  gtk_widget_modify_font (the_text_view,pango_font_description_from_string("Sans 10"));
}

void set_edit_font(gpointer data, guint action, GtkWidget *widget)
{
  editfontdialog = GTK_FONT_SELECTION_DIALOG(gtk_font_selection_dialog_new("Choose Dasher Font"));
  g_signal_connect (editfontdialog->cancel_button, "clicked", G_CALLBACK (gtk_widget_destroy), G_OBJECT (editfontdialog));
  g_signal_connect (editfontdialog->ok_button, "clicked", G_CALLBACK (get_edit_font_from_dialog), (gpointer) editfontdialog);
  gtk_widget_show(GTK_WIDGET(editfontdialog));
}
