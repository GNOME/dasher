#include "GtkDasherWindow.h"
#include "GtkDasherPane.h"

#include <gtk--/window.h>
#include <gtk--/main.h>

GtkDasherWindow::GtkDasherWindow()
{
  dasher_pane = new GtkDasherPane;

  add( *dasher_pane );

  show_all();
  
  dasher_pane->clear();

}

GtkDasherWindow::~GtkDasherWindow()
{
  delete( dasher_pane );
}

int GtkDasherWindow::destroy_event_impl(GdkEventAny *event)
{
  Gtk::Main::quit();
}

int GtkDasherWindow::delete_event_impl(GdkEventAny *event)
{
  Gtk::Main::quit();
  return( false );
}
