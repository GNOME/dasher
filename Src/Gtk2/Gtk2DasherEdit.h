#ifndef GTK2_DASHER_EDIT_H
#define GTK2_DASHER_EDIT_H

#include <gtk/gtk.h>

#include "DashEdit.h"
#include "DasherTypes.h"
#include "DasherInterface.h"

using namespace Dasher;

class Gtk2DasherEdit : public Dasher::CDashEditbox
{
 public:
  Gtk2DasherEdit(CDasherInterface *interface );
  ~Gtk2DasherEdit();

  void write_to_file();
  void get_new_context(std::string& str, int max);
  void unflush();
  void output(symbol Symbol);
  void flush(symbol Symbol);
  void Clear();
  void Cut();
  void Copy();
  void CopyAll();
  void Paste();
  void SelectAll();

  void SetEncoding(Opts::FileEncodingFormats Encoding);
  void SetFont(std::string Name, long Size);
  void TimeStampNewFiles(bool Value);

  GtkWidget *text_view;  
  GtkTextBuffer *text_buffer;
  GtkClipboard *text_clipboard;
  CDasherInterface *interface;

  static gboolean handle_cursor_move(GtkWidget *widget, GdkEventButton *event, gpointer callback_data);
  int flush_count;
  bool timestamp;
  Opts::FileEncodingFormats file_encoding;
};

#endif


