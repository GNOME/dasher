#include <gtk/gtk.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include <libintl.h>
#include <locale.h>

#include "libdasher.h"
#include "dasher.h"
#include "settings_store.h"
#include "canvas.h"
#include "edit.h"

//extern GConfClient *the_gconf_client;

int
main(int argc, char *argv[])
{
  gtk_init (&argc, &argv);

  GError *gconferror;

  gconf_init( argc, argv, &gconferror );

  the_gconf_client = gconf_client_get_default();

  dasher_set_get_bool_option_callback( get_bool_option_callback );
  dasher_set_get_long_option_callback( get_long_option_callback );
  dasher_set_get_string_option_callback( get_string_option_callback );

  dasher_set_set_bool_option_callback( set_bool_option_callback );
  dasher_set_set_long_option_callback( set_long_option_callback );
  dasher_set_set_string_option_callback( set_string_option_callback );

  dasher_set_string_callback( parameter_string_callback );
  dasher_set_double_callback( parameter_double_callback );
  dasher_set_int_callback( parameter_int_callback );
  dasher_set_bool_callback( parameter_bool_callback );

  dasher_set_blank_callback( blank_callback );
  dasher_set_display_callback( display_callback );
  dasher_set_draw_rectangle_callback( draw_rectangle_callback );
  dasher_set_draw_polyline_callback( draw_polyline_callback );
  dasher_set_draw_text_callback( draw_text_callback );
  dasher_set_text_size_callback( text_size_callback );
  
  dasher_set_edit_output_callback( edit_output_callback );
  dasher_set_edit_flush_callback( edit_flush_callback );
  dasher_set_edit_unflush_callback( edit_unflush_callback );

  dasher_set_clipboard_callback( clipboard_callback );

  setlocale (LC_ALL, "");

  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);

  interface_setup();

  dasher_early_initialise( 360, 360 );

  open_window ();

  dasher_late_initialise();

  gtk_main ();

  dasher_finalise();

  return 0;
}
