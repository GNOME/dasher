#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <gtk--/window.h>

#include "GtkDasherPane.h"

class GtkDasherWindow : public Gtk::Window
{
public:
  GtkDasherWindow();
  ~GtkDasherWindow();
  int destroy_event_impl(GdkEventAny *event);
  int delete_event_impl(GdkEventAny *event);

protected:
  GtkDasherPane *dasher_pane;
};

#endif







