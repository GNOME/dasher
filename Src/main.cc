#include "Common/Common.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>
#include <signal.h>

#ifdef WITH_MAEMO
#include <hildon-lgpl/hildon-widgets/hildon-app.h>
#include <hildon-lgpl/hildon-widgets/hildon-appview.h>
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

#ifdef WITH_GPE
#include "gpesettings_store.h"
#endif

extern int optind;
extern const gchar *filename;

DasherMain *g_pDasherMain;
DasherAppSettings *g_pDasherAppSettings;
DasherPreferencesDialogue *g_pPreferencesDialogue;

// Stuff imported from dasher.cc


GtkWidget *vbox;
GdkPixbuf *p;                   // Hmm... descriptive names
GtkWidget *pw;
GtkStyle *style;
//GtkAccelGroup *dasher_accel;
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

// TODO: reimplement command line parsing - should just be a way of
// temporarily overriding parameters

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

  //parse command line options
  GOptionContext *goptcontext;
  goptcontext = g_option_context_new(("- A text input application honouring accessibility"));
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
#ifndef WITH_MAEMO
  gtk_window_set_default_icon_name("dasher");
#endif


  // Create a main app object
  g_pEditor = dasher_editor_new(argc, argv);

  GladeXML *pGladeXML = dasher_main_get_glade(g_pDasherMain);

  window = glade_xml_get_widget(pGladeXML, "window");
  vbox = glade_xml_get_widget(pGladeXML, "vbox1");
  the_text_view = glade_xml_get_widget(pGladeXML, "the_text_view");
  the_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(the_text_view));
  
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
  
  dasher_main_show(g_pDasherMain);

  gtk_main();
  
  if(g_pEditor)
    g_object_unref(G_OBJECT(g_pEditor));

#ifdef GNOME_LIBS
  gnome_vfs_shutdown();
#endif

  return 0;
}

void sigint_handler(int iSigNum) {
  g_message("Trapped SIGINT - attempting shutdown...");
  if(gtk_main_level() > 0)
    gtk_main_quit();  
  else {
    if(g_pEditor)
      g_object_unref(G_OBJECT(g_pEditor));
    
    exit(0);
  }
}
