// GtkDasherSave.cc
// (c) 2002 Philip Cowans

#include "GtkDasherSave.h"
#include <iostream.h>

GtkDasherSave::GtkDasherSave()
  : FileSelection("Save As"), current_filename("")
{
  // get_ok_button()->clicked.connect(slot(this, &GtkDasherSave::file_ok_sel));
  get_cancel_button()->clicked.connect(slot(this, &GtkDasherSave::file_cancel_sel));
}

void GtkDasherSave::file_ok_sel() {
  cout << "Save here with filename " << get_filename() << endl;

  current_filename = get_filename();

  set_filename(current_filename.c_str());
  hide();
}

void GtkDasherSave::file_cancel_sel() {
  set_filename(current_filename.c_str());
  hide();
}
