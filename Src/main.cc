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

GError *gconferror;
GConfEngine *gconfengine;

int
main(int argc, char *argv[])
{
  gtk_init (&argc, &argv);

  gconf_init( argc, argv, &gconferror );

  /* I am a bad man and I will go straight to hell.

     But seriously.

     There's no way of determining whether a key exists through the 
     gconf_client interface, so we need a gconfengine. But if you have both,
     gconf complains at length (and, to be fair, it is a really bad idea in
     the general case). Because I'm more concerned about making the user feel
     happy than I am about making the user think I'm incompetent, we make those
     errors go to a nice place where they're happy and won't scare the user.

     The null_log_handler should probably check whether it's actually the
     error that we're worried about, and if not pass it on to the default
     handler - FIXME */

  g_log_set_handler ("GConf", G_LOG_LEVEL_WARNING, null_log_handler, NULL);

  gconfengine = gconf_engine_get_default();
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
  dasher_set_get_new_context_callback( get_new_context_callback );

  dasher_set_clipboard_callback( clipboard_callback );

  setlocale (LC_ALL, "");

  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);

  interface_setup();

  dasher_early_initialise();

  open_window ();

  dasher_late_initialise(360,360);

  choose_filename();

  gtk_main ();

  gconf_engine_unref(gconfengine);

  dasher_finalise();

  return 0;
}
