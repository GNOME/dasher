#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>

#ifndef WITH_GPE
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#endif

#if (defined GNOME_SPEECH || defined GNOME_A11Y)
//#include <gnome.h>
#include <libbonobo.h>
#endif

#ifdef WITH_GPE
#include <gpe/init.h>
#endif

#ifdef GNOME_LIBS
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <popt.h>

static const struct poptOption options [] =
{
  {NULL, '\0', 0, NULL, 0}
};
#endif

#define PREFIX "/usr/"
#define SYSCONFDIR "/usr/share/dasher/"
#define LIBDIR "/usr/lib/"
#define DATADIR "/usr/share/dasher/"

#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
//#include <getopt.h>

#include "libdasher.h"
#include "dasher.h"
#include "canvas.h"
#include "edit.h"

#ifdef WITH_GPE
#include "gpesettings_store.h"
#else
#include "settings_store.h"
#endif

#ifdef GNOME_SPEECH
#include "speech.h"
#endif

#include "accessibility.h"
#include "mouse_input.h"

//extern GConfClient *the_gconf_client;

GError *gconferror;
gboolean timedata=FALSE;
gboolean preferences=FALSE;
gboolean textentry=FALSE;
gboolean stdoutpipe=FALSE;
extern gboolean setup,paused;
extern int optind;
extern ControlTree *controltree;
extern const gchar* filename;

extern int oldx, oldy;

// Declare our global file logging object
#include "DasherCore/FileLogger.h"
#ifdef _DEBUG
	const eLogLevel gLogLevel   = logDEBUG;
    const int       gLogOptions = logTimeStamp | logDateStamp | logDeleteOldFile;    
#else
	const eLogLevel gLogLevel = logNORMAL;
    const int       gLogOptions = logTimeStamp | logDateStamp;
#endif
CFileLogger* gLogger = NULL;

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

  // Global logging object we can use from anywhere
  gLogger = new CFileLogger("dasher.log",
							gLogLevel,		
                            gLogOptions);

  int c;
  XWMHints wm_hints;
  Atom wm_window_protocols[3];

  bindtextdomain (PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);

#ifdef GNOME_LIBS
  GnomeProgram *program=0;
  program = gnome_program_init("Dasher", PACKAGE_VERSION, LIBGNOMEUI_MODULE, argc, argv, GNOME_PARAM_POPT_TABLE, options, GNOME_PROGRAM_STANDARD_PROPERTIES, GNOME_PARAM_HUMAN_READABLE_NAME, _("Dasher Text Entry"), NULL);

  gnome_vfs_init();

#endif

#ifdef WITH_GPE
  gpe_application_init (&argc, &argv);
  init_xsettings();
#else
  gtk_init (&argc, &argv);
  gconf_init( argc, argv, &gconferror );
#endif

  // We need thread support for updating the splash window while
  // training...

#ifndef GNOME_LIBS
  g_thread_init(NULL);
#endif

  
#ifdef WITH_GPE
  xml = glade_xml_new(PROGDATA"/dashergpe.glade", NULL, NULL);
#else
  xml = glade_xml_new(PROGDATA"/dasher.glade", NULL, NULL);
  the_gconf_client = gconf_client_get_default();
#endif


#if (defined GNOME_SPEECH || defined GNOME_A11Y)
    if (!bonobo_init (&argc, argv))
      {
        g_error ("Can't initialize Bonobo...\n");
      }
    bonobo_activate();
#endif

  while (1) {
    c=getopt( argc, argv, "wpos" );

    if (c == -1)
      break;

    switch (c) {
    case 'w':
      // Print number of characters produced per second
      timedata=TRUE;
      break;
    case 'p':
      // Only show the preferences window
      preferences=TRUE;
      break;
    case 'o':
      // Onscreen text entry mode
      textentry=TRUE;
      break;
    case 's':
      // Pipe stuff to stdout
      stdoutpipe=TRUE;
      break;
    }
  }

  

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
  dasher_set_draw_colour_polyline_callback( draw_colour_polyline_callback );
  dasher_set_draw_colour_polygon_callback( draw_colour_polygon_callback );
  dasher_set_draw_text_callback( draw_text_callback );
  dasher_set_draw_text_string_callback( draw_text_string_callback );
  dasher_set_text_size_callback( text_size_callback );
  dasher_set_send_marker_callback( send_marker_callback );


  dasher_set_edit_output_callback( gtk2_edit_output_callback );
  dasher_set_edit_outputcontrol_callback( gtk2_edit_outputcontrol_callback );

  dasher_set_edit_delete_callback( gtk2_edit_delete_callback );
  dasher_set_get_new_context_callback( gtk2_get_new_context_callback );
  dasher_set_clipboard_callback( gtk2_clipboard_callback );


  oldx = -1;
  oldy = -1;

#ifdef GNOME_A11Y
  SPI_init ();
#endif


  interface_setup(xml);

  dasher_early_initialise();

  paused=true;

  glade_xml_signal_autoconnect(xml);

  if (preferences==TRUE) {
    window = glade_xml_get_widget(xml, "preferences");
  } else {
    window = glade_xml_get_widget(xml, "window");
  }

  open_window(xml);

  dasher_late_initialise(360,360);

  interface_late_setup();

  pMouseInput = new CDasherMouseInput;

  dasher_set_input( pMouseInput );


#ifdef WITH_GPE
  gtk_window_set_decorated(GTK_WINDOW(window),false);
#endif

  gtk_widget_show(window);

  //  if (preferences!=TRUE) {
    setup=TRUE;
    //  }

  // We support advanced colour mode
  dasher_set_parameter_bool( BOOL_COLOURMODE, true);

  
  wm_window_protocols[0] = gdk_x11_get_xatom_by_name("WM_DELETE_WINDOW");
  wm_window_protocols[1] = gdk_x11_get_xatom_by_name("_NET_WM_PING");
  wm_window_protocols[2] = gdk_x11_get_xatom_by_name("WM_TAKE_FOCUS");
  
  wm_hints.flags = InputHint;
  wm_hints.input = False;

  XSetWMHints (GDK_WINDOW_XDISPLAY (window->window), GDK_WINDOW_XWINDOW (window->window), &wm_hints);
  XSetWMProtocols (GDK_WINDOW_XDISPLAY (window->window),GDK_WINDOW_XWINDOW (window->window), wm_window_protocols, 3);
  gdk_window_add_filter (window->window, dasher_discard_take_focus_filter, NULL);
  
  dasher_pause(0,0); // we start paused

#ifdef GNOME_SPEECH
  setup_speech();
#endif

  setupa11y();

  controltree=gettree();
  add_control_tree(controltree);

  if (optind<argc) {
    if (!g_path_is_absolute(argv[optind])) {
      char *cwd;
      cwd=(char *)malloc(1024*sizeof(char));
      getcwd(cwd,1024);
      filename=g_build_path("/",cwd,argv[optind],NULL);
      open_file(filename);
    } else {
      filename=argv[optind];
      open_file(filename);
    }
  } else {
    choose_filename();
  }

  dasher_user_log_init_is_done();

  gtk_main ();

  interface_cleanup();

#ifndef WITH_GPE
  g_object_unref(the_gconf_client);
#endif

#ifdef GNOME_SPEECH
  teardown_speech();
#endif

#ifdef GNOME_LIBS
  gnome_vfs_shutdown();
#endif
  
#ifdef GNOME_A11Y
  deletemenutree();
  SPI_exit();
#endif

  dasher_finalise();
  
  if (gLogger != NULL)
  {
    delete gLogger;
	gLogger  = NULL;
  }

  return 0;
}


