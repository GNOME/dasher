#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#if (defined GNOME_SPEECH || defined GNOME_A11Y)
//#include <gnome.h>
#include <libbonobo.h>
#endif

#define PREFIX "/usr/"
#define SYSCONFDIR "/usr/share/dasher/"
#define LIBDIR "/usr/lib/"
#define DATADIR "/usr/share/dasher/"

#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "libdasher.h"
#include "dasher.h"
#include "settings_store.h"
#include "canvas.h"
#include "edit.h"

#ifdef GNOME_SPEECH
#include "speech.h"
#endif

#include "accessibility.h"

//extern GConfClient *the_gconf_client;

GError *gconferror;
GConfEngine *gconfengine;
gboolean timedata;

GdkFilterReturn dasher_discard_take_focus_filter (GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
  XEvent *xev = (XEvent *)xevent;
 
  if (xev->xany.type == ClientMessage &&
      (Atom) xev->xclient.data.l[0] == gdk_x11_atom_to_xatom (
							      gdk_atom_intern ("WM_TAKE_FOCUS", False)) && keyboardmodeon==true)
    {
      return GDK_FILTER_REMOVE;
    }
  else
    {
      return GDK_FILTER_CONTINUE;
    }
}


int
main(int argc, char *argv[])
{
  GladeXML *xml;
  GtkWidget *window;

  int c;
  XWMHints wm_hints;
  Atom wm_window_protocols[3];

  while (1) {
    c=getopt( argc, argv, "w" );

    if (c == -1)
      break;

    switch (c) {
    case 'w':
      timedata=TRUE;
      break;
    }
  }
    

  gtk_init (&argc, &argv);

  gconf_init( argc, argv, &gconferror );

  xml = glade_xml_new(PROGDATA"/dasher.glade", NULL, NULL);

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

#if (defined GNOME_SPEECH || defined GNOME_A11Y)
    if (!bonobo_init (&argc, argv))
      {
        g_error ("Can't initialize Bonobo...\n");
      }
#endif

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
  dasher_set_colour_scheme_callback( receive_colour_scheme_callback );
  dasher_set_draw_rectangle_callback( draw_rectangle_callback );
  dasher_set_draw_polyline_callback( draw_polyline_callback );
  dasher_set_draw_text_callback( draw_text_callback );
  dasher_set_draw_text_string_callback( draw_text_string_callback );
  dasher_set_text_size_callback( text_size_callback );
  
  dasher_set_edit_output_callback( edit_output_callback );
  dasher_set_edit_outputcontrol_callback( edit_outputcontrol_callback );
  dasher_set_edit_delete_callback( edit_delete_callback );
  dasher_set_get_new_context_callback( get_new_context_callback );

  dasher_set_clipboard_callback( clipboard_callback );

  setlocale (LC_ALL, "");

  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);

#ifdef GNOME_A11Y
  SPI_init ();
#endif


  interface_setup(xml);

  dasher_early_initialise();

  open_window(xml);


  // We support advanced colour mode
  window = glade_xml_get_widget(xml, "window");
  glade_xml_signal_autoconnect(xml);


  wm_window_protocols[0] = gdk_x11_get_xatom_by_name("WM_DELETE_WINDOW");
  wm_window_protocols[1] = gdk_x11_get_xatom_by_name("_NET_WM_PING");
  wm_window_protocols[2] = gdk_x11_get_xatom_by_name("WM_TAKE_FOCUS");
  
  wm_hints.flags = InputHint;
  wm_hints.input = False;

  XSetWMHints (GDK_WINDOW_XDISPLAY (window->window), GDK_WINDOW_XWINDOW (window->window), &wm_hints);
  XSetWMProtocols (GDK_WINDOW_XDISPLAY (window->window),GDK_WINDOW_XWINDOW (window->window), wm_window_protocols, 3);
  gdk_window_add_filter (window->window, dasher_discard_take_focus_filter, NULL);

  dasher_late_initialise(360,360);

  dasher_set_parameter_bool( BOOL_COLOURMODE, true);
  
  choose_filename();

  dasher_pause(0,0); // we start paused

#ifdef GNOME_SPEECH
  setup_speech();
#endif

  add_control_tree(gettree());
  
  gtk_main ();

#ifdef GNOME_SPEECH
  teardown_speech();
#endif
  
  gconf_engine_unref(gconfengine);

  dasher_finalise();

  return 0;
}
