#ifndef GTK_DASHER_EDIT_H
#define GTK_DASHER_EDIT_H

#include <gtk--/text.h>
#include <gtk--/box.h>
#include <gtk--/scrollbar.h>
#include "DashEdit.h"
#include "DasherTypes.h"
#include "DasherInterface.h"

#include <string.h>

using namespace Dasher;
using namespace Gtk;

class GtkDasherEdit : public Gtk::HBox, public Dasher::CDashEditbox
{
 public:
  GtkDasherEdit( CDasherInterface *_interface );
  ~GtkDasherEdit();

  void write_to_file();
  void get_new_context(std::string& str, int max);
  void unflush();
  void output(symbol Symbol);
  void flush(symbol Symbol);
  void Clear();

  void SetEncoding(Opts::FileEncodingFormats Encoding);
  void SetFont(std::string Name, long Size);
  bool SaveAs( const std::string filename, bool a );
  bool Save( bool a=false);
  bool Open( const std::string filename );

  void TimeStampNewFiles(bool Value);

  void Cut();
  void Copy();
  void CopyAll();
  void Paste();
  void SelectAll();

  gint handle_cursor_move( GdkEventButton *e );

  void kill_flush();

 protected:
  int flush_count;
  CDasherInterface *interface;
  string current_filename;
  bool filename_set;

  Gdk_Font efont;

  Gtk::Text text;
  Gtk::VScrollbar vsb;
  
};

#endif
