#include "Common/Common.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>
#include <signal.h>

#ifdef WITH_MAEMO
#include <hildon-lgpl/hildon-widgets/hildon-app.h>
#include <hildon-lgpl/hildon-widgets/hildon-appview.h>
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
#include "accessibility.h"

#ifdef WITH_GPE
#include "gpesettings_store.h"
#endif

// gboolean timedata = FALSE;
// gboolean preferences = FALSE;
// gboolean textentry = FALSE;
// gboolean stdoutpipe = FALSE;
extern int optind;
extern const gchar *filename;

DasherMain *g_pDasherMain;
DasherAppSettings *g_pDasherAppSettings;

void sigint_handler(int iSigNum);

// GOption command line parsing variables
gboolean timedata = FALSE;
gboolean preferences = FALSE;
gboolean textentry = FALSE;
gboolean stdoutpipe = FALSE;

static const GOptionEntry options[] = {
  {"timedata", 'w', 0, G_OPTION_ARG_NONE, &timedata, "Write basic timing information to stdout", NULL},
  {"preferences", 'p', 0, G_OPTION_ARG_NONE, &preferences, "Show preferences window only", NULL},
  {"textentry", 'o', 0, G_OPTION_ARG_NONE, &textentry, "Onscreen text entry mode", NULL},
  {"pipe", 's', 0, G_OPTION_ARG_NONE, &stdoutpipe, "Pipe text to stdout", NULL},
  {NULL}
};


GdkFilterReturn dasher_discard_take_focus_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data) {
  XEvent *xev = (XEvent *) xevent;

  //  if(xev->xany.type == ClientMessage && (Atom) xev->xclient.data.l[0] == gdk_x11_atom_to_xatom(gdk_atom_intern("WM_TAKE_FOCUS", False)) && keyboardmodeon == true) {

if(xev->xany.type == ClientMessage && (Atom) xev->xclient.data.l[0] == gdk_x11_atom_to_xatom(gdk_atom_intern("WM_TAKE_FOCUS", False))) {

    return GDK_FILTER_REMOVE;
  }
  else {
    return GDK_FILTER_CONTINUE;
  }
}

int main(int argc, char *argv[]) {
  GladeXML *xml;
#ifdef WITH_MAEMO
  HildonApp *app;
  HildonAppView *appview;
#endif

  signal(2, sigint_handler);

  bindtextdomain(PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  textdomain(PACKAGE);

  //parse command line options
  GOptionContext *goptcontext;
  goptcontext = g_option_context_new(_("- A text input application honouring accessibility"));
  g_option_context_add_main_entries(goptcontext, options, "Dasher");
  g_option_context_parse(goptcontext, &argc, &argv, NULL);
  //later GnomeProgram will call g_option_context_free() when we unref it

#ifdef WITH_GPE
  gpe_application_init(&argc, &argv);
  init_xsettings();
#else
  gtk_init(&argc, &argv);
#endif

#ifdef GNOME_LIBS
  GnomeProgram *program = 0;
#if GLIB_CHECK_VERSION(2,14,0)
  program = gnome_program_init
    (argv[0], PACKAGE_VERSION, LIBGNOMEUI_MODULE,
     argc, argv,
     GNOME_PARAM_GOPTION_CONTEXT, goptcontext, GNOME_PARAM_NONE);
#else
  program = gnome_program_init
    (argv[0], PACKAGE_VERSION, LIBGNOMEUI_MODULE,
     argc, argv,
     GNOME_PARAM_NONE);
#endif
#endif
  
  gnome_vfs_init();



  // We need thread support for updating the splash window while
  // training...

#ifndef GNOME_LIBS
  if (!g_thread_supported()) 
    g_thread_init(NULL);
#endif


  // Set up the dasher_main

  g_pDasherMain = dasher_main_new(argc, argv);
  g_pDasherAppSettings = dasher_app_settings_new(argc, argv);


  //  g_type_class_ref(dasher_gtk_text_view_get_type());

  g_set_application_name ("Dasher");
#ifndef WITH_MAEMO
  gtk_window_set_default_icon_name ("dasher");
#endif

#ifdef WITH_GPE
  xml = glade_xml_new(PROGDATA "/dashergpe.glade", NULL, NULL);
#elif WITH_MAEMO
  //  xml = glade_xml_new("/var/lib/install" PROGDATA "/dashermaemo.glade", NULL, NULL);
  xml = glade_xml_new(PROGDATA "/dashermaemo.glade", NULL, NULL);
#else
  xml = glade_xml_new(PROGDATA "/dasher.glade", NULL, NULL);
#endif
  if (!xml) {
    g_error("Can't find dasher.glade. Probably not installed properly ...\n");
  }

#if (defined GNOME_SPEECH || defined GNOME_A11Y)
  if(!bonobo_init(&argc, argv)) {
    g_error("Can't initialize Bonobo...\n");
  }
  bonobo_activate();
#endif

//   oldx = -1;
//   oldy = -1;

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

  //#ifndef WITH_MAEMO
  gtk_widget_show(window);
// #else
//   appview = HILDON_APPVIEW( hildon_appview_new(NULL) );
//   app = HILDON_APP( hildon_app_new() );
//   hildon_app_set_appview( app, appview );
//   hildon_app_set_title( app, ("Dasher" )); 
//   window = glade_xml_get_widget(xml, "vpaned1");
//   gtk_widget_reparent (window, GTK_WIDGET(appview));
//   gtk_paned_set_position(GTK_PANED(window), 100);

//   /* Do menu setup */
//   GtkMenu *main_menu;
//   GtkWidget *file_menu;
//   GtkWidget *file_menu_item;
//   GtkWidget *options_menu;
//   GtkWidget *options_menu_item;
//   main_menu = hildon_appview_get_menu(appview);
//   file_menu = glade_xml_get_widget(xml, "menuitem4_menu");
//   options_menu = glade_xml_get_widget(xml, "options1_menu");
//   file_menu_item = gtk_menu_item_new_with_label ("File");
//   options_menu_item = gtk_menu_item_new_with_label ("Options");
//   gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item),file_menu);
//   gtk_menu_item_set_submenu(GTK_MENU_ITEM(options_menu_item),options_menu);
// //  gtk_widget_reparent (GTK_WIDGET(main_menu), file_menu);
// //  gtk_widget_reparent (GTK_WIDGET(main_menu), options_menu);
//   gtk_menu_append( main_menu, file_menu_item);
//   gtk_menu_append( main_menu, options_menu_item);
//   gtk_widget_show_all( GTK_WIDGET( main_menu ) );

//   /* And toolbar */
//   GtkWidget *toolbar;
//   toolbar = glade_xml_get_widget(xml, "toolbar");
//   g_print("Got %p\n",toolbar);
//   gtk_widget_reparent (toolbar, appview->vbox);

//   gtk_widget_show_all(GTK_WIDGET(app));
// #endif


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

#ifdef GNOME_LIBS
  gnome_vfs_shutdown();
#endif

  return 0;
}

void sigint_handler(int iSigNum) {
  g_message("Trapped SIGINT - attempting shutdown...");
  gtk_main_quit();
}
