// GtkDasherSave.h
// (c) 2002 Philip Cowans

#ifndef GTKDASHERSAVE_H
#define GTKDASHERSAVE_H

#include <gtk--/fileselection.h>
#include <string.h>

class GtkDasherSave : public Gtk::FileSelection
{
 public:
  GtkDasherSave();

  string current_filename;
 protected:
  void file_ok_sel();
  void file_cancel_sel();
  

};

#endif
