#include <iostream>
#include <string>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "Gtk2DasherEdit.h"

Gtk2DasherEdit::Gtk2DasherEdit(CDasherInterface *interface ) : Dasher::CDashEditbox()
{
  flush_count = 0;
  this->interface = interface;

  text_clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), TRUE);
  text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);

  g_signal_connect(G_OBJECT(text_view), "button_press_event", G_CALLBACK(handle_cursor_move), (gpointer) this);
  


}

Gtk2DasherEdit::~Gtk2DasherEdit()
{
}

void Gtk2DasherEdit::get_new_context(std::string& str, int max)
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(text_buffer,end,gtk_text_buffer_get_insert(text_buffer));

  *start=*end;  

  gtk_text_iter_backward_chars(start, max);

  str = gtk_text_buffer_get_text (text_buffer,start,end,FALSE);

}

void Gtk2DasherEdit::write_to_file()
{
}

void Gtk2DasherEdit::unflush()
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(text_buffer,end,gtk_text_buffer_get_insert(text_buffer));

  *start=*end;  

  gtk_text_iter_backward_chars(start, flush_count);

  gtk_text_buffer_delete(text_buffer,start,end);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(text_view),gtk_text_buffer_get_insert(text_buffer));
  flush_count=0;

}

void Gtk2DasherEdit::output(symbol Symbol)
{
  std::string label;
  label = interface->GetEditText( Symbol );

  gtk_text_buffer_insert_at_cursor(text_buffer, label.c_str(), -1);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(text_view),gtk_text_buffer_get_insert(text_buffer));
}

void Gtk2DasherEdit::flush(symbol Symbol)
{
  std::string label;
  label = interface->GetEditText( Symbol );
  gtk_text_buffer_insert_at_cursor(text_buffer, label.c_str(), -1);

  if (label!="") {
    flush_count++;
  }

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(text_view),gtk_text_buffer_get_insert(text_buffer));
}
  
void Gtk2DasherEdit::Clear()
{
  gtk_text_buffer_set_text (text_buffer,"",0);
}

void Gtk2DasherEdit::SetEncoding(Opts::FileEncodingFormats Encoding)
{
}

void Gtk2DasherEdit::SetFont(std::string Name, long Size)
{
}

gboolean Gtk2DasherEdit::handle_cursor_move(GtkWidget *widget, GdkEventButton *event, gpointer this2 ) 
{
  ((Gtk2DasherEdit*)this2)->flush_count=0;
  ((Gtk2DasherEdit*)this2)->interface->Start();
  ((Gtk2DasherEdit*)this2)->interface->Redraw();

  return( false );

}

void Gtk2DasherEdit::Cut()
{
  gtk_text_buffer_cut_clipboard(text_buffer, text_clipboard, TRUE);
}

void Gtk2DasherEdit::Copy()
{
  gtk_text_buffer_copy_clipboard(text_buffer, text_clipboard);
}

void Gtk2DasherEdit::CopyAll()
{
}

void Gtk2DasherEdit::Paste()
{
  gtk_text_buffer_paste_clipboard(text_buffer, text_clipboard, NULL, TRUE);
}

void Gtk2DasherEdit::SelectAll()
{
}


