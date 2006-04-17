// /*! \file dasher.cc 
//   \brief The core of the Dasher application

//   Core routines for the Dasher application
// */

// // System headers

// // #include "../Common/Common.h"

// // #include <sys/types.h>
// // #include <sys/time.h>
// // #include <sys/stat.h>
// // #include <stdlib.h>
// // #include <libintl.h>
// // #include <locale.h>
// // #include <string>
// // #include <vector>
// // #include <stdio.h>
// // #include <time.h>
// // #include <dirent.h>
// // #include <X11/Xlib.h>
// // #include <X11/Xatom.h>
// // #include "Output.h"

// // // C++ STL headers (ideally minimise the use of C++ outside of the control)

// // #include <iostream>

// // GTK/GDK headers and friends

// #include <gtk/gtk.h>
// #include <gdk/gdk.h>
// #include <gdk/gdkx.h>

// #ifdef GNOME_LIBS
// #include <libgnomeui/libgnomeui.h>
// #include <libgnome/libgnome.h>
// #endif

// #ifdef WITH_GPE
// #include "gpesettings_store.h"
// #endif

// // Other Dasher components

// #include "dasher.h"
// #include "edit.h"
// #include "fileops.h"
// #include "Preferences.h"

// #include "Menu.h"
// #include "AppSettings.h"
// #include "../DasherCore/Parameters.h"
// #include "accessibility.h"
// #include "dasher_lock_dialogue.h"

// // We shouldn't need this - the functions which reference it are obsolete
// //#include "../DasherCore/Event.h"

// #include "GtkDasherControl.h"

// // Pointers to various GTK widgets

// // GladeXML *widgets;


// ///
// /// Initialise the main window and child components 
// /// This is actually closer to 'initialise application', so name
// /// should really be changed to reflect this
// ///

// void InitialiseMainWindow(int argc, char **argv) {
// }

