#ifndef GTK_DASHER_EDIT_H
#define GTK_DASHER_EDIT_H

#include <gtk--/text.h>
#include "DashEdit.h"
#include "DasherTypes.h"

using namespace Dasher;
using namespace Gtk;

class GtkDasherEdit : public Gtk::Text, public Dasher::CDashEditbox
{
 public:
  GtkDasherEdit();
  ~GtkDasherEdit();

  void write_to_file();
  void get_new_context(std::string& str, int max);
  void unflush();
  void output(symbol Symbol);
  void flush(symbol Symbol);
  void Clear();

  void SetEncoding(Opts::FileEncodingFormats Encoding);
  void SetFont(std::string Name, long Size);
};

#endif
