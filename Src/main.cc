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
#endif

#include <popt.h>

static const struct poptOption options[] = {
  {"timedata", 'w', POPT_ARG_NONE, NULL, 1, "Write basic timing information to stdout", NULL},
  {"preferences", 'p', POPT_ARG_NONE, NULL, 1, "Show preferences window only", NULL},
  {"textentry", 'o', POPT_ARG_NONE, NULL, 1, "Onscreen text entry mode", NULL},
  {"pipe", 's', POPT_ARG_NONE, NULL, 1, "Pipe text to stdout", NULL},
#ifndef GNOME_LIBS
  // Gnome will add its own help
  POPT_AUTOHELP
#endif
  {NULL, '\0', 0, NULL, 0, NULL, NULL}
};


#define PREFIX "/usr/"
#define SYSCONFDIR "/usr/share/dasher/"
#define LIBDIR "/usr/lib/"
#define DATADIR "/usr/share/dasher/"

#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
//#include <getopt.h>

#include "dasher.h"
#include "edit.h"
#include "DasherControl.h"
#include "Menu.h"

#ifdef WITH_GPE
#include "gpesettings_store.h"
#endif

#ifdef GNOME_SPEECH
#include "speech.h"
#endif

#include "accessibility.h"

//extern GConfClient *the_gconf_client;

gboolean timedata = FALSE;
gboolean preferences = FALSE;
gboolean textentry = FALSE;
gboolean stdoutpipe = FALSE;
extern gboolean setup, paused;
extern int optind;
extern ControlTree *controltree;
extern const gchar *filename;

extern int oldx, oldy;

GdkFilterReturn dasher_discard_take_focus_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data) {
  XEvent *xev = (XEvent *) xevent;

  if(xev->xany.type == ClientMessage && (Atom) xev->xclient.data.l[0] == gdk_x11_atom_to_xatom(gdk_atom_intern("WM_TAKE_FOCUS", False)) && keyboardmodeon == true) {
    return GDK_FILTER_REMOVE;
  }
  else {
    return GDK_FILTER_CONTINUE;
  }
}

int main(int argc, char *argv[]) {
  GladeXML *xml;

  int c;
  XWMHints wm_hints;
  Atom wm_window_protocols[3];

  bindtextdomain(PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  textdomain(PACKAGE);
  
  poptContext sContext; 

#ifdef GNOME_LIBS
  GnomeProgram *program = 0;

  program = gnome_program_init("Dasher", PACKAGE_VERSION, LIBGNOMEUI_MODULE, argc, argv, GNOME_PARAM_POPT_TABLE, options, GNOME_PROGRAM_STANDARD_PROPERTIES, GNOME_PARAM_HUMAN_READABLE_NAME, _("Dasher Text Entry"), NULL);

  g_object_get(G_OBJECT(program), GNOME_PARAM_POPT_CONTEXT, &sContext, NULL);
  poptResetContext(sContext);
  
  gnome_vfs_init();
#else
  sContext = poptGetContext(NULL, argc, (const char**)argv, options, 0);
#endif

  while(1) {
    int iNextOption;

    iNextOption = poptGetNextOpt(sContext);

    if(iNextOption == -1)
      break;

    switch(iNextOption) {
    case 1:
      // Print number of characters produced per second
      timedata = TRUE;
      break;
    case 2:
      // Only show the preferences window
      preferences = TRUE;
      break;
    case 3:
      // Onscreen text entry mode
      textentry = TRUE;
      break;
    case 4:
      // Pipe stuff to stdout
      stdoutpipe = TRUE;
      break;
    }
  }

  // Set up the app GConf client

  GError *pGConfError;

  if(!gconf_init(argc, argv, &pGConfError)) {
    std::cerr << "Failed to initialise gconf: " << pGConfError->message << std::endl;
    exit(1);
  }

  g_pGConfClient = gconf_client_get_default();

  // ---

#ifdef WITH_GPE
  gpe_application_init(&argc, &argv);
  init_xsettings();
#else
  gtk_init(&argc, &argv);

#endif

  // We need thread support for updating the splash window while
  // training...

#ifndef GNOME_LIBS
  if (!g_thread_supported()) 
    g_thread_init(NULL);
#endif

  g_type_class_ref(dasher_gtk_text_view_get_type());

#ifdef WITH_GPE
  xml = glade_xml_new(PROGDATA "/dashergpe.glade", NULL, NULL);
#else
  xml = glade_xml_new(PROGDATA "/dasher.glade", NULL, NULL);
#endif

#if (defined GNOME_SPEECH || defined GNOME_A11Y)
  if(!bonobo_init(&argc, argv)) {
    g_error("Can't initialize Bonobo...\n");
  }
  bonobo_activate();
#endif

//   while(1) {
//     c = getopt(argc, argv, "wpos");

//     if(c == -1)
//       break;

//     switch (c) {
//     case 'w':
//       // Print number of characters produced per second
//       timedata = TRUE;
//       break;
//     case 'p':
//       // Only show the preferences window
//       preferences = TRUE;
//       break;
//     case 'o':
//       // Onscreen text entry mode
//       textentry = TRUE;
//       break;
//     case 's':
//       // Pipe stuff to stdout
//       stdoutpipe = TRUE;
//       break;
//     }
//   }

//   dasher_set_string_callback( parameter_string_callback );
//   dasher_set_double_callback( parameter_double_callback );
//   dasher_set_int_callback( parameter_int_callback );
//   dasher_set_bool_callback( parameter_bool_callback );

//   dasher_set_edit_output_callback( gtk2_edit_output_callback );
//   dasher_set_edit_outputcontrol_callback( gtk2_edit_outputcontrol_callback );

//   dasher_set_edit_delete_callback( gtk2_edit_delete_callback );
//   dasher_set_get_new_context_callback( gtk2_get_new_context_callback );
//   dasher_set_clipboard_callback( gtk2_clipboard_callback );

  oldx = -1;
  oldy = -1;

#ifdef GNOME_A11Y
  SPI_init();
#endif

  glade_xml_signal_autoconnect(xml);

  if(preferences == TRUE) {
    window = glade_xml_get_widget(xml, "preferences");
  }
  else {
    window = glade_xml_get_widget(xml, "window");
  }

  // Initialise the main window and show it

  InitialiseMainWindow(argc, argv, xml);

#ifdef WITH_GPE
  gtk_window_set_decorated(GTK_WINDOW(window), false);
#endif

  gtk_widget_show(window);

  //  if (preferences!=TRUE) {
  setup = TRUE;
  //  }

  wm_window_protocols[0] = gdk_x11_get_xatom_by_name("WM_DELETE_WINDOW");
  wm_window_protocols[1] = gdk_x11_get_xatom_by_name("_NET_WM_PING");
  wm_window_protocols[2] = gdk_x11_get_xatom_by_name("WM_TAKE_FOCUS");

  wm_hints.flags = InputHint;
  wm_hints.input = False;

  XSetWMHints(GDK_WINDOW_XDISPLAY(window->window), GDK_WINDOW_XWINDOW(window->window), &wm_hints);
  XSetWMProtocols(GDK_WINDOW_XDISPLAY(window->window), GDK_WINDOW_XWINDOW(window->window), wm_window_protocols, 3);
  gdk_window_add_filter(window->window, dasher_discard_take_focus_filter, NULL);

#ifdef GNOME_SPEECH
  setup_speech();
#endif

  setupa11y();

  // FIXME - REIMPLEMENT
//   controltree=gettree();
//   add_control_tree(controltree);

  if(optind < argc) {
    if(!g_path_is_absolute(argv[optind])) {
      char *cwd;
      cwd = (char *)malloc(1024 * sizeof(char));
      getcwd(cwd, 1024);
      filename = g_build_path("/", cwd, argv[optind], NULL);
      open_file(filename);
    }
    else {
      filename = argv[optind];
      open_file(filename);
    }
  }
  else {
    choose_filename();
  }

  gtk_main();

  interface_cleanup();

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

  // Take down GConf client

  g_object_unref(g_pGConfClient);

  // ---

  return 0;
}
