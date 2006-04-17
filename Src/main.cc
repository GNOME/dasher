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
#include "edit.h"
#include "DasherControl.h"
#include "Menu.h"
#include "accessibility.h"


#ifdef WITH_GPE
#include "gpesettings_store.h"
#endif

extern int optind;
extern const gchar *filename;

DasherMain *g_pDasherMain;
DasherAppSettings *g_pDasherAppSettings;
DasherPreferencesDialogue *g_pPreferencesDialogue;

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

GdkFilterReturn dasher_discard_take_focus_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data) {
  XEvent *xev = (XEvent *) xevent;
  if(xev->xany.type == ClientMessage && (Atom) xev->xclient.data.l[0] == gdk_x11_atom_to_xatom(gdk_atom_intern("WM_TAKE_FOCUS", False))) {
    return GDK_FILTER_REMOVE;
  }
  else {
    return GDK_FILTER_CONTINUE;
  }
}

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


  // TODO: Do we really need thread support still?

  // We need thread support for updating the splash window while
  // training...

#ifndef GNOME_LIBS
  if (!g_thread_supported()) 
    g_thread_init(NULL);
#endif



  //  g_type_class_ref(dasher_gtk_text_view_get_type());

  g_set_application_name("Dasher");
#ifndef WITH_MAEMO
  gtk_window_set_default_icon_name("dasher");
#endif


#if (defined GNOME_SPEECH || defined GNOME_A11Y)
  if(!bonobo_init(&argc, argv)) {
    g_error("Can't initialize Bonobo...\n");
  }
  bonobo_activate();
#endif

  // Initialise the main window and show it
  g_pDasherMain = dasher_main_new();
  
  g_pDasherAppSettings = dasher_app_settings_new(argc, argv);
  dasher_main_set_app_settings(g_pDasherMain, g_pDasherAppSettings);

  InitialiseMainWindow(argc, argv);
  
  dasher_main_show(g_pDasherMain);

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
  
  if(g_pEditor)
    g_object_unref(G_OBJECT(g_pEditor));

  // TODO: Figure out what this is supposed to do and reimplement if necessary
  
// #ifdef X_HAVE_UTF8_STRING
//   // We want to set the keymap back to whatever it was before,
//   // if that's possible
//   int min, max;
//   Display *dpy = gdk_x11_get_default_xdisplay();
//   XDisplayKeycodes(dpy, &min, &max);
//   XChangeKeyboardMapping(dpy, min, numcodes, origkeymap, (max - min));
// #endif

#ifdef GNOME_LIBS
  gnome_vfs_shutdown();
#endif

  return 0;
}

void sigint_handler(int iSigNum) {
  g_message("Trapped SIGINT - attempting shutdown...");
  gtk_main_quit();
}
