#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <gtk--/window.h>
#include <gtk--/toolbar.h>
#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/menubar.h>

#include "GtkDasherPane.h"

class GtkDasherWindow : public Gtk::Window
{
public:
  GtkDasherWindow();
  ~GtkDasherWindow();
  int destroy_event_impl(GdkEventAny *event);
  int delete_event_impl(GdkEventAny *event);

protected:
  GtkDasherPane dasher_pane;
  Gtk::Toolbar toolbar;
  Gtk::VBox main_vbox; 
  Gtk::MenuBar menu;

  void toolbar_button_cb(char *);
};

#endif







