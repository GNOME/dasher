#include "Common/Common.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>


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

#ifdef HAVE_POPT
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

#ifdef GNOME_SPEECH
#include "speech.h"
#endif

//#include "accessibility.h"

//extern GConfClient *the_gconf_client;

gboolean timedata = FALSE;
gboolean preferences = FALSE;
gboolean textentry = FALSE;
gboolean stdoutpipe = FALSE;
//extern gboolean setup, paused;
extern int optind;
extern ControlTree *controltree;
extern const gchar *filename;

DasherMain *g_pDasherMain;
DasherAppSettings *g_pDasherAppSettings;

//extern int oldx, oldy;

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

  bindtextdomain(PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  textdomain(PACKAGE);

#ifdef HAVE_POPT  
  poptContext sContext; 
#endif

#ifdef WITH_GPE
  gpe_application_init(&argc, &argv);
  init_xsettings();
#else
  gtk_init(&argc, &argv);
#endif

#ifdef HAVE_POPT
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
#endif // HAVE_POPT


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

  g_set_application_name (_("Dasher"));
#ifndef WITH_MAEMO
  gtk_window_set_default_icon_name ("dasher");
#endif

#ifdef WITH_GPE
  xml = glade_xml_new(PROGDATA "/dashergpe.glade", NULL, NULL);
#elif WITH_MAEMO
  xml = glade_xml_new("/var/lib/install" PROGDATA "/dashermaemo.glade", NULL, NULL);
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

#ifndef WITH_MAEMO
  gtk_widget_show(window);
#else
  appview = HILDON_APPVIEW( hildon_appview_new(NULL) );
  app = HILDON_APP( hildon_app_new() );
  hildon_app_set_appview( app, appview );
  hildon_app_set_title( app, ("Dasher" )); 
  window = glade_xml_get_widget(xml, "vpaned1");
  gtk_widget_reparent (window, GTK_WIDGET(appview));
  gtk_paned_set_position(GTK_PANED(window), 100);

  /* Do menu setup */
  GtkMenu *main_menu;
  GtkWidget *file_menu;
  GtkWidget *file_menu_item;
  GtkWidget *options_menu;
  GtkWidget *options_menu_item;
  main_menu = hildon_appview_get_menu(appview);
  file_menu = glade_xml_get_widget(xml, "menuitem4_menu");
  options_menu = glade_xml_get_widget(xml, "options1_menu");
  file_menu_item = gtk_menu_item_new_with_label ("File");
  options_menu_item = gtk_menu_item_new_with_label ("Options");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item),file_menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(options_menu_item),options_menu);
//  gtk_widget_reparent (GTK_WIDGET(main_menu), file_menu);
//  gtk_widget_reparent (GTK_WIDGET(main_menu), options_menu);
  gtk_menu_append( main_menu, file_menu_item);
  gtk_menu_append( main_menu, options_menu_item);
  gtk_widget_show_all( GTK_WIDGET( main_menu ) );

  /* And toolbar */
  GtkWidget *toolbar;
  toolbar = glade_xml_get_widget(xml, "toolbar");
  g_print("Got %p\n",toolbar);
  gtk_widget_reparent (toolbar, appview->vbox);

  gtk_widget_show_all(GTK_WIDGET(app));
#endif


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

  // Shut down the dasher_main
  // FIXME

  interface_cleanup();

#ifdef GNOME_SPEECH
  teardown_speech();
#endif

#ifdef GNOME_LIBS
  gnome_vfs_shutdown();
#endif

  // A11y sypport disabled for now
  //#ifdef GNOME_A11Y
  //  deletemenutree();
  //  SPI_exit();
  //#endif

  // ---

  return 0;
}
