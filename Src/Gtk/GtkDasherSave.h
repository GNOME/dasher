#ifndef GTKDASHERSAVE_H
#define GTKDASHERSAVE_H

#include <gtk--/fileselection.h>
#include <string.h>

class GtkDasherSave : public Gtk::FileSelection
{
 public:
  GtkDasherSave();
 protected:
  string current_filename;

  void file_ok_sel();
  void file_cancel_sel();
  

};

#endif
