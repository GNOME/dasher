// Main.cc
// (c) 2002 Philip Cowans

#include <libintl.h>
#include <locale.h>

// Gtk includes

#include <gtk--/main.h>

// Gtk Dasher includes

#include "GtkDasherWindow.h"

int main( int argc, char **argv )
{
  // Do gettext stuff

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  Gtk::Main gtk_main( argc, argv );

  GtkDasherWindow main_window;
  main_window.show();

  gtk_main.run();

  return( 0 );
}
