#include "Common/Common.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>
#include <signal.h>

#ifdef WITH_MAEMO
#include <libosso.h>
#endif

// TODO: This shouldn't need to be here
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
#include "DasherControl.h"
#include "Gtk2/dasher_lock_dialogue.h"
#include "Gtk2/FontDialogues.h"

#ifdef WITH_GPE
#include "gpesettings_store.h"
#endif

extern int optind;
extern const gchar *filename;

DasherMain *g_pDasherMain;
DasherAppSettings *g_pDasherAppSettings;
DasherPreferencesDialogue *g_pPreferencesDialogue;

// Stuff imported from dasher.cc

#ifdef WITH_MAEMO
  osso_context_t *osso_context;
#endif



GtkWidget *vbox;
GdkPixbuf *p;                   // Hmm... descriptive names
GtkWidget *pw;
GtkStyle *style;

GtkWidget *dasher_menu_bar;
GtkWidget *open_filesel;
GtkWidget *save_filesel;
GtkWidget *save_and_quit_filesel;
GtkWidget *import_filesel;
GtkWidget *append_filesel;
GtkWidget *window;
GtkWidget *g_pHiddenWindow;
GtkWidget *file_selector;

GtkWidget *pDasherWidget = NULL;
//GtkWidget *g_pEditPane = 0;
//GtkWidget *g_pActionPane = 0;

#ifdef WITH_MAEMO
Window g_xOldIMWindow; 
#endif

//DasherAction *g_pAction = 0;

const gchar *filename = NULL;   // Filename of file currently being edited

// Apparently not obsolete, but should be sorted out

gboolean file_modified = FALSE; // Have unsaved changes been made to the current file
gint outputcharacters;

const char *g_szAccessibleContext = 0;
int g_iExpectedPosition = -1;
int g_iOldPosition = -1;

// 'Private' methods



// "member" variables for main window "class"

int g_bOnTop = true; // Whether the window should always be on top
int g_bDock = true; // Whether to dock the window
int g_iDockType; // Ignored for now - will determine how the window is docked to the side of the screen
double g_dXFraction = 0.25; // Fraction of the width of the screen to use;
double g_dYFraction = 0.25; // Fraction of the height of the screen to use;

/// ---

/// Old stuff from edit.cc

DasherEditor *g_pEditor = 0;

GtkWidget *the_text_view;
GtkTextBuffer *the_text_buffer;

KeySym *origkeymap;
int modifiedkey = 0;
int numcodes;

/// ---

void sigint_handler(int iSigNum);
void clean_up();

// TODO: reimplement command line parsing - should just be a way of
// temporarily overriding parameters

// GOption command line parsing variables
// gboolean timedata = FALSE;
// gboolean preferences = FALSE;
// gboolean textentry = FALSE;
// gboolean stdoutpipe = FALSE;


// TODO: Do we actually need this - gtk should in theory have
// functions to prevent windows from taking focus, but maybe they
// don't work which is why this is here.

// GdkFilterReturn dasher_discard_take_focus_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data) {
//   XEvent *xev = (XEvent *) xevent;
//   if(xev->xany.type == ClientMessage && (Atom) xev->xclient.data.l[0] == gdk_x11_atom_to_xatom(gdk_atom_intern("WM_TAKE_FOCUS", False))) {
//     return GDK_FILTER_REMOVE;
//   }
//   else {
//     return GDK_FILTER_CONTINUE;
//   }
// }

int main(int argc, char *argv[]) {
  signal(2, sigint_handler);

  bindtextdomain(PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  textdomain(PACKAGE);

  gchar *szOptionAppstyle = NULL;

  // TODO: It would be nice to have command line parsing in version prior to goption (eg in Solaris 10)...
#if GLIB_CHECK_VERSION(2,6,0)
  static const GOptionEntry options[] = {
    //   {"timedata", 'w', 0, G_OPTION_ARG_NONE, &timedata, "Write basic timing information to stdout", NULL},
    //   {"preferences", 'p', 0, G_OPTION_ARG_NONE, &preferences, "Show preferences window only", NULL},
    //   {"textentry", 'o', 0, G_OPTION_ARG_NONE, &textentry, "Onscreen text entry mode", NULL},
    //   {"pipe", 's', 0, G_OPTION_ARG_NONE, &stdoutpipe, "Pipe text to stdout", NULL},
    {"appstyle", 'a', 0, G_OPTION_ARG_STRING, &szOptionAppstyle, "Application style (traditional, direct, compose or fullscreen)", "traditional"},
    {NULL}
  };

  //parse command line options
  GOptionContext *goptcontext;
  goptcontext = g_option_context_new(("- A text input application honouring accessibility"));
  g_option_context_add_main_entries(goptcontext, options, "Dasher");
  g_option_context_parse(goptcontext, &argc, &argv, NULL);
  //later GnomeProgram will call g_option_context_free() when we unref it
#endif 

#ifdef WITH_GPE
  gpe_application_init(&argc, &argv);
  init_xsettings();
#else
  gtk_init(&argc, &argv);
#endif

#ifdef WITH_MAEMO
  osso_context = osso_initialize("dasher", PACKAGE_VERSION, TRUE, NULL);
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

  gnome_vfs_init();
#endif

#if (defined GNOME_SPEECH || defined GNOME_A11Y)
  if(!bonobo_is_initialized()) {
    if(!bonobo_init(&argc, argv)) {
      g_error("Can't initialize Bonobo...\n");
    }
    bonobo_activate();
  }
#endif

  g_set_application_name("Dasher");
#if (!defined WITH_MAEMO) && GTK_CHECK_VERSION(2,6,0)
  gtk_window_set_default_icon_name("dasher");
#endif


  g_pDasherMain = dasher_main_new();

  // Stuff which will eventually be in init of main class

  // 1.
  g_pDasherAppSettings = dasher_app_settings_new(argc, argv);
  dasher_main_set_app_settings(g_pDasherMain, g_pDasherAppSettings);

  // 2.
  if(szOptionAppstyle) {
    if(!strcmp(szOptionAppstyle, "traditional")) {
      dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_STYLE, 0);
    }
    else if(!strcmp(szOptionAppstyle, "compose")) {
      dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_STYLE, 1);
    }
    else if(!strcmp(szOptionAppstyle, "direct")) {
      dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_STYLE, 2);
    }
    else if(!strcmp(szOptionAppstyle, "fullscreen")) {
      dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_STYLE, 3);
    }
    else {
      g_error("Application style %s is not supported", szOptionAppstyle);
    }
  }
  else { 
    dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_STYLE, 0);
  }

  // 3.
  dasher_main_load_interface(g_pDasherMain);

  GladeXML *pGladeXML = dasher_main_get_glade(g_pDasherMain);

  window = glade_xml_get_widget(pGladeXML, "window");
  vbox = glade_xml_get_widget(pGladeXML, "vbox1");
  the_text_view = glade_xml_get_widget(pGladeXML, "the_text_view");
  the_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(the_text_view));

  // 4.
  dasher_app_settings_set_widget(g_pDasherAppSettings, GTK_DASHER_CONTROL(pDasherWidget));
  
  // 5.
  // Create a main app object
  g_pEditor = dasher_editor_new(argc, argv);
  dasher_editor_initialise(g_pEditor);

  // TODO: Make this part of editor initialisation.
  // Subclass editors depending on required functionality.
  if(optind < argc) {
    if(!g_path_is_absolute(argv[optind])) {
      char *cwd;
      cwd = (char *)malloc(1024 * sizeof(char));
      getcwd(cwd, 1024);
      filename = g_build_path("/", cwd, argv[optind], NULL);
    }
    else {
      filename = argv[optind];
    }
    dasher_editor_open(g_pEditor, filename);
  }
  else {
    // TODO: Call new routine, make generate_filename private
    dasher_editor_generate_filename(g_pEditor);
  }

  // 6.
  g_pPreferencesDialogue = dasher_preferences_dialogue_new(pGladeXML, g_pEditor);
  dasher_preferences_dialogue_populate_actions(g_pPreferencesDialogue);
  // TODO: Make lock diaogue a full method
#ifndef WITH_MAEMO
  dasher_lock_dialogue_new(pGladeXML, GTK_WINDOW(dasher_main_get_window(g_pDasherMain)));
#else
  dasher_lock_dialogue_new(pGladeXML, 0);
#endif
  // TODO: Bring into object framework
  InitialiseFontDialogues(pGladeXML);
  
  // 7. 
  dasher_main_populate_controls(g_pDasherMain);

  // 8.
  dasher_main_setup_window(g_pDasherMain);

  // 9.
  dasher_main_show(g_pDasherMain);

  // 10.
  gtk_main();

  // 11.
  clean_up();

  return 0;
}

void clean_up() {
#ifdef WITH_MAEMO
  osso_deinitialize(osso_context);
#endif

  // TODO: Really need a sensible object takedown chain (preferences dialogue etc.)
  
  if(g_pEditor)
    g_object_unref(G_OBJECT(g_pEditor));
  
  if(g_pDasherMain)
    g_object_unref(G_OBJECT(g_pDasherMain));

#ifdef GNOME_LIBS
  gnome_vfs_shutdown();
#endif
}

void sigint_handler(int iSigNum) { 
  if(gtk_main_level() > 0) {
    gtk_main_quit();  
  }
  else {
    // This implies that we haven't got as far as setting up the main loop yet
    clean_up();
    exit(0);
  }
}
