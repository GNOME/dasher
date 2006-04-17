/*! \file dasher.cc 
  \brief The core of the Dasher application

  Core routines for the Dasher application
*/

// System headers

#include "../Common/Common.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "Output.h"

// C++ STL headers (ideally minimise the use of C++ outside of the control)

#include <iostream>

// GTK/GDK headers and friends

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#ifdef GNOME_LIBS
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#endif

#ifdef WITH_GPE
#include "gpesettings_store.h"
#endif

// Other Dasher components

#include "dasher.h"
#include "edit.h"
#include "fileops.h"
#include "Preferences.h"
#include "FontDialogues.h"
#include "Menu.h"
#include "AppSettings.h"
#include "../DasherCore/Parameters.h"
#include "accessibility.h"
#include "dasher_lock_dialogue.h"

// We shouldn't need this - the functions which reference it are obsolete
//#include "../DasherCore/Event.h"

#include "GtkDasherControl.h"

// Pointers to various GTK widgets

// GladeXML *widgets;

GtkWidget *vbox;
GdkPixbuf *p;                   // Hmm... descriptive names
GtkWidget *pw;
GtkStyle *style;
GtkAccelGroup *dasher_accel;
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


///
/// Initialise the main window and child components 
/// This is actually closer to 'initialise application', so name
/// should really be changed to reflect this
///

void InitialiseMainWindow(int argc, char **argv) {

  GladeXML *pGladeXML;
  pGladeXML = dasher_main_get_glade(g_pDasherMain);

  dasher_accel = gtk_accel_group_new(); //?

  //    widgets = pGladeXML;          // obsolete? NO - used later in this file, but should be
  // Grab some pointers to important GTK widgets from the Glade XML
  // FIXME - do we actually need all of these?

  window = glade_xml_get_widget(pGladeXML, "window");

  vbox = glade_xml_get_widget(pGladeXML, "vbox1");
  dasher_menu_bar = glade_xml_get_widget(pGladeXML, "dasher_menu_bar");
  pDasherWidget = glade_xml_get_widget(pGladeXML, "DasherControl");

  dasher_lock_dialogue_new(pGladeXML, GTK_WINDOW(window));


  // Initialise the various components

  initialise_edit(pGladeXML);
#ifndef WITH_MAEMO
  PopulateMenus(pGladeXML);
#endif

  g_pPreferencesDialogue = dasher_preferences_dialogue_new(pGladeXML);

  InitialiseFontDialogues(pGladeXML);

}

