// Main.cc
// (c) 2002 Philip Cowans

// Gtk includes

#include <gtk--/main.h>

// Gtk Dasher includes

#include "GtkDasherWindow.h"

int main( int argc, char **argv )
{
  Gtk::Main gtk_main( argc, argv );

  GtkDasherWindow main_window;
  main_window.show();

  gtk_main.run();

  return( 0 );
}
