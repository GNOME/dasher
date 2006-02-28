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
#include "Output.h"

// C++ STL headers (ideally minimise the use of C++ outside of the control)

#include <iostream>

// GTK/GDK headers and friends

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <gconf/gconf.h>

#ifdef GNOME_SPEECH
#include <libbonobo.h>
#include <gnome-speech/gnome-speech.h>
#endif

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
//#include "accessibility.h"
#include "fileops.h"

#include "Preferences.h"
#include "FontDialogues.h"
#include "Menu.h"
#include "AppSettings.h"
#include "../DasherCore/Parameters.h"
#include "speech.h"
#include "accessibility.h"

#include "dasher_action_keyboard.h"

// We shouldn't need this - the functions which reference it are obsolete
#include "../DasherCore/Event.h"

#include "GtkDasherControl.h"

// Pointers to various GTK widgets

GladeXML *widgets;

GtkWidget *vbox, *toolbar;
GdkPixbuf *p;                   // Hmm... descriptive names
GtkWidget *pw;
GtkStyle *style;
GtkAccelGroup *dasher_accel;
GtkWidget *dasher_menu_bar;
GtkWidget *vpane;
GtkWidget *open_filesel;
GtkWidget *save_filesel;
GtkWidget *save_and_quit_filesel;
GtkWidget *import_filesel;
GtkWidget *append_filesel;
GtkWidget *window;
GtkWidget *g_pHiddenWindow;
GtkWidget *file_selector;
GtkWidget *pDasherWidget = NULL;

GtkWidget *g_pEditPane = 0;
GtkWidget *g_pActionPane = 0;

DasherAction *g_pAction = 0;

// GConf client

GConfClient *g_pGConfClient;

// Boolean application parameters - note that we shouldn't be specifying default values here - see InitialiseAppParameters() instead

bool keyboardmodeon = false;    // I *think* this is whether to 
gboolean timestamp;             // Whether to automatically construct new filenames based on timestamp
gboolean showtoolbar;           // Whether to show the toolbar or not
gboolean keyboardcontrol;       // ?
bool cyclickeyboardmodeon = false;      // ? (obsolete?)
gboolean leavewindowpause;      // Whether to pause Dasher when we leave the window
gboolean speakonstop = FALSE;   // Whether to speak when Dasher is stopped
extern gboolean timedata;       // Whether to output logging data (obsolete?)
//extern gboolean drawoutline;
extern gboolean textentry;      // Keyboard emulation for entering text into other applications
extern gboolean stdoutpipe;     // Whether to output text to stdout

// Interger applications parameters

gint fileencoding;

// String application parameters

const gchar *filename = NULL;   // Filename of file currently being edited
std::string editfont = "Sans 10";       // Font to use in edit box

// Boolean application status flags

gboolean setup = FALSE;         // Has setup been completed (?)
gboolean indrag = FALSE;        // ?
gboolean file_modified = FALSE; // Have unsaved changes been made to the current file
gboolean quitting = FALSE;      // Are we in the process of shutting down Dasher 

const char *g_szAccessibleContext = 0;
int g_iExpectedPosition = -1;
int g_iOldPosition = -1;

// Possibly obsolete global stuff

button buttons[10];

#define NO_PREV_POS -1

gboolean coordcalled;

double bitrate;

gint buttonnum = 0;

gint outputcharacters;

time_t lastdirection = 0;

// 'Private' methods

void LoadWindowState();
void InitialiseAppParameters();

extern "C" void handle_start_event(GtkDasherControl * pDasherControl, gpointer data);
extern "C" void handle_stop_event(GtkDasherControl * pDasherControl, gpointer data);
extern "C" void handle_control_event(GtkDasherControl *pDasherControl, gint iEvent, gpointer data);
extern "C" void parameter_notification(GtkDasherControl * pDasherControl, gint iParameter, gpointer data);
extern "C" void handle_context_request(GtkDasherControl * pDasherControl, gint iMaxLength, gpointer data);
extern "C" bool focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data);
extern "C" void handle_request_settings(GtkDasherControl * pDasherControl, gpointer data);
void SetupPositioning();

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
extern "C" void realize_widget(GtkWidget* pWidget, gpointer pUserData) {
  gtk_dasher_control_set_focus( GTK_DASHER_CONTROL(pDasherWidget));
}

extern "C" void on_window_map(GtkWidget* pWidget, gpointer pUserData) {
//   if(g_bOnTop)
//     gtk_window_set_keep_above(GTK_WINDOW(window), true);
  
  //  SetupWMHints();
  SetupPositioning();
  // A11y support disabled for now
  setupa11y();

}

void InitialiseMainWindow(int argc, char **argv, GladeXML *pGladeXML) {

  init_app_settings();
  load_app_parameters();

  dasher_accel = gtk_accel_group_new(); //?

  widgets = pGladeXML;          // obsolete?
  // Grab some pointers to important GTK widgets from the Glade XML
  // FIXME - do we actually need all of these?

  toolbar = glade_xml_get_widget(pGladeXML, "toolbar");

  window = glade_xml_get_widget(pGladeXML, "window");
  g_signal_connect(GTK_WIDGET(window), "map", G_CALLBACK(on_window_map), NULL);
 

  vbox = glade_xml_get_widget(pGladeXML, "vbox1");
  vpane = glade_xml_get_widget(pGladeXML, "hpaned1");
  dasher_menu_bar = glade_xml_get_widget(pGladeXML, "dasher_menu_bar");

  g_pEditPane = glade_xml_get_widget(pGladeXML, "vbox40");
  g_pActionPane = glade_xml_get_widget(pGladeXML, "vbox39");

  if( get_app_parameter_bool( APP_BP_SHOW_TOOLBAR ) ) {
    gtk_widget_show( toolbar );
  }

  // Construct a Dasher control

  pDasherWidget = gtk_dasher_control_new();
  g_signal_connect(GTK_WIDGET(pDasherWidget), "realize", G_CALLBACK(realize_widget), NULL); 
  
  // Add UI control node stuff (might be better elsewhere)
  
  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER, "Speak", -1 );
  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER+1, "All", -1 );
  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER+2, "New", -1 );
  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER+3, "Repeat", -1 );

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER, Dasher::CControlManager::CTL_ROOT, -2);

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER+1, Dasher::CControlManager::CTL_USER, -2);
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER+2, Dasher::CControlManager::CTL_USER, -2);
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_USER+3, Dasher::CControlManager::CTL_USER, -2);
 
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), -1, Dasher::CControlManager::CTL_USER+1, -2);
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_ROOT, Dasher::CControlManager::CTL_USER+1, -2);
  
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), -1, Dasher::CControlManager::CTL_USER+2, -2);
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_ROOT, Dasher::CControlManager::CTL_USER+2, -2);

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), -1, Dasher::CControlManager::CTL_USER+3, -2);
  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pDasherWidget), Dasher::CControlManager::CTL_ROOT, Dasher::CControlManager::CTL_USER+3, -2);
  // ---

  g_signal_connect(pDasherWidget, "dasher_changed", G_CALLBACK(parameter_notification), NULL);
  g_signal_connect(pDasherWidget, "dasher_start", G_CALLBACK(handle_start_event), NULL);
  g_signal_connect(pDasherWidget, "dasher_stop", G_CALLBACK(handle_stop_event), NULL);
  g_signal_connect(pDasherWidget, "dasher_control", G_CALLBACK(handle_control_event), NULL);
  g_signal_connect(pDasherWidget, "dasher_edit_insert", G_CALLBACK(gtk2_edit_output_callback), NULL);
  g_signal_connect(pDasherWidget, "dasher_edit_delete", G_CALLBACK(gtk2_edit_delete_callback), NULL);
  g_signal_connect(pDasherWidget, "dasher_context_request", G_CALLBACK(handle_context_request), NULL); 
  g_signal_connect(pDasherWidget, "dasher_request_settings", G_CALLBACK(handle_request_settings), NULL);
  
  g_signal_connect(pDasherWidget, "focus_in_event", G_CALLBACK(focus_in_event), NULL);


  gtk_container_add(GTK_CONTAINER(glade_xml_get_widget(pGladeXML, "alignment35")), pDasherWidget);

#ifndef GNOME_SPEECH
  // This ought to be greyed out if not built with speech support
  gtk_widget_set_sensitive(glade_xml_get_widget(pGladeXML, "speakbutton"), false);
#endif

#ifndef JAPANESE
  gtk_widget_hide(glade_xml_get_widget(pGladeXML, "radiobutton9"));
#endif

  // Initialise the various components

  initialise_edit(pGladeXML);
  PopulateMenus(pGladeXML);
  initialise_preferences_dialogue(pGladeXML);
  InitialiseFontDialogues(pGladeXML);


  LoadWindowState();
#ifdef PJC_EXPERIMENTAL
  //  SetupPositioning();
#endif

#ifdef WITH_GPE
  // We always want this on in the GPE version, otherwise it's entirely useless
  // Well, I suppose you could give it to kids, or impress primitive tribes,
  // or convince members of the appropriate sex that you're somehow deeply cool,
  // but they're not really our design goals.
  //  dasher_set_parameter_bool(BOOL_KEYBOARDMODE, true);
#endif

  InitialiseAppParameters();

  g_pAction = DASHER_ACTION(dasher_action_keyboard_new());
}

///
/// Initialise the application parameters from GConf - not currently
/// implemented.
///

void InitialiseAppParameters() {
}

///
/// Functions that need to be called as we're shutting down
/// This gets called after the Gtk loop has exited, so the widgets will no longer exist
///

void interface_cleanup() {
  cleanup_edit();
  delete_app_settings();
}


void SetupWMHints(bool bTopMost) {
#ifndef WITH_MAEMO
  XWMHints wm_hints;
  Atom wm_window_protocols[3];

  wm_window_protocols[0] = gdk_x11_get_xatom_by_name("WM_DELETE_WINDOW");
  wm_window_protocols[1] = gdk_x11_get_xatom_by_name("_NET_WM_PING");
  wm_window_protocols[2] = gdk_x11_get_xatom_by_name("WM_TAKE_FOCUS");

  wm_hints.flags = InputHint;
  wm_hints.input = False;

  std::cout << "bTopMost is: " << bTopMost << " " << window->window << std::endl;

  if(bTopMost) {
    XSetWMHints(GDK_WINDOW_XDISPLAY(window->window), GDK_WINDOW_XWINDOW(window->window), &wm_hints);
    XSetWMProtocols(GDK_WINDOW_XDISPLAY(window->window), GDK_WINDOW_XWINDOW(window->window), wm_window_protocols, 3);
    gdk_window_add_filter(window->window, dasher_discard_take_focus_filter, NULL);
  }
  else {
    gdk_window_remove_filter(window->window, dasher_discard_take_focus_filter, NULL);
  }

//   XSetWMHints(GDK_WINDOW_XDISPLAY(g_pHiddenWindow->window), GDK_WINDOW_XWINDOW(g_pHiddenWindow->window), &wm_hints);
//   XSetWMProtocols(GDK_WINDOW_XDISPLAY(g_pHiddenWindow->window), GDK_WINDOW_XWINDOW(g_pHiddenWindow->window), wm_window_protocols, 3);
//   gdk_window_add_filter(g_pHiddenWindow->window, dasher_discard_take_focus_filter, NULL);
#endif

  gtk_window_set_keep_above(GTK_WINDOW(window), bTopMost);
  //  gtk_window_set_keep_above(GTK_WINDOW(g_pHiddenWindow), true);
}

void SetupPositioning() {

  if(!GTK_WIDGET_MAPPED(window))
    return;

  // FIXME - what does gravity actually achieve here?

//   gint iWidth;
//   gint iHeight;

//   gtk_window_set_gravity(GTK_WINDOW(window), GDK_GRAVITY_SOUTH_EAST);

//   gtk_window_resize(GTK_WINDOW(window),(int)round(gdk_screen_width() * g_dXFraction), (int)round(gdk_screen_height() * g_dYFraction) );

//   gtk_window_get_size(GTK_WINDOW(window), &iWidth, &iHeight);
//   gtk_window_move(GTK_WINDOW(window), gdk_screen_width() - iWidth - 32,  gdk_screen_height() - iHeight - 32);

//   gtk_window_get_size(GTK_WINDOW(g_pHiddenWindow), &iWidth, &iHeight);
//   gtk_window_set_gravity(GTK_WINDOW(g_pHiddenWindow), GDK_GRAVITY_SOUTH_EAST);
//   gtk_window_move(GTK_WINDOW(g_pHiddenWindow), gdk_screen_width() - iWidth - 32,  gdk_screen_height() - iHeight - 32);

  bool bShowEdit = false;
  bool bShowActions = false;
  bool bTopMost = false;

  switch(get_app_parameter_long(APP_LP_STYLE)) {
  case 0:
    std::cout << "Classic style" << std::endl;
    bShowEdit = true;
    bShowActions = false;
    bTopMost = false;
    break;
  case 1:
    std::cout << "Composition style" << std::endl;
    bShowEdit = true;
    bShowActions = true;
    bTopMost = true;
    break;
  case 2:
    std::cout << "Direct style" << std::endl;
    bShowEdit = false;
    bShowActions = false;
    bTopMost = true;
    break;
  }

  if(bShowActions) {
    gtk_widget_show(g_pActionPane);
  }
  else {
    gtk_widget_hide(g_pActionPane);
  }

  if(bShowEdit) {
    gtk_widget_show(g_pEditPane);
  }
  else {
    gtk_widget_hide(g_pEditPane);
  }

  SetupWMHints(bTopMost);
}

///
/// Notification callbacks for parameters having changed
///

extern "C" void parameter_notification(GtkDasherControl *pDasherControl, gint iParameter, gpointer data) {
  if(iParameter == LP_DASHER_FONTSIZE) {
    switch (gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_DASHER_FONTSIZE)) {
    case Opts::Normal:
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets, "fontsizenormal")), TRUE);
      break;
    case Opts::Big:
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets, "fontsizelarge")), TRUE);
      break;
    case Opts::VBig:
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(widgets, "fontsizevlarge")), TRUE);
      break;
    }
  }
  else if(iParameter == LP_UNIFORM) {
    gtk_range_set_value(GTK_RANGE(glade_xml_get_widget(widgets, "uniformhscale")), gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_UNIFORM) / 10.0);
  }
  else if(iParameter == LP_LANGUAGE_MODEL_ID) {
    switch (gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_LANGUAGE_MODEL_ID)) {
    case 0:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton6"))) != TRUE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton6")), TRUE);
      break;
    case 2:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton7"))) != TRUE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton7")), TRUE);
      break;
    case 3:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton8"))) != TRUE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton8")), TRUE);
      break;
#ifdef JAPANESE
    case 4:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton9"))) != TRUE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton9")), TRUE);
      break;
#endif
    }
  }
  else if(iParameter == LP_ORIENTATION) {
    switch (gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ORIENTATION)) {
    case Opts::Alphabet:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton1"))) != TRUE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton1")), TRUE);
      break;
    case Opts::LeftToRight:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton2"))) != TRUE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton2")), TRUE);
      break;
    case Opts::RightToLeft:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton3"))) != TRUE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton3")), TRUE);
      break;
    case Opts::TopToBottom:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton4"))) != TRUE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton4")), TRUE);
      break;
    case Opts::BottomToTop:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton5"))) != TRUE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "radiobutton5")), TRUE);
      break;
    }
  }
  else if(iParameter == BP_SHOW_SLIDER) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "speedsliderbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_SHOW_SLIDER));
  }
  else if(iParameter == BP_DRAW_MOUSE) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "showmousebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_DRAW_MOUSE));
  }
  else if(iParameter == BP_DRAW_MOUSE_LINE) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "showmouselinebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_DRAW_MOUSE_LINE));
  }
  else if(iParameter == BP_NUMBER_DIMENSIONS) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "onedbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_NUMBER_DIMENSIONS));
  }
  else if(iParameter == BP_EYETRACKER_MODE) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "eyetrackerbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_EYETRACKER_MODE));
  }
  else if(iParameter == BP_START_MOUSE) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "leftbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_START_MOUSE));
  }
  else if(iParameter == BP_START_SPACE) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "spacebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_START_SPACE));
  }
  else if((iParameter == BP_MOUSEPOS_MODE) || (iParameter == BP_CIRCLE_START)) {
    // FIXME - duplicated code from Preferences.cpp
    if(get_app_parameter_bool(BP_MOUSEPOS_MODE)) {
      gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(widgets, "MousePosStyle")), 1);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "mouseposbutton")), true);
    }
    else if(get_app_parameter_bool(BP_CIRCLE_START)) {
      gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(widgets, "MousePosStyle")), 0);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "mouseposbutton")), true);
    }
    else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "mouseposbutton")), false);
    }
  }
  else if(iParameter == BP_KEY_CONTROL) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "keyboardbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_KEY_CONTROL));
  }
  else if(iParameter == BP_CONTROL_MODE) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "keyboardbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_KEY_CONTROL));
  }
  else if(iParameter == BP_OUTLINE_MODE) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "outlinebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_OUTLINE_MODE));
  }
//   else if(iParameter == BP_KEYBOARD_MODE) {
//     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "keyboardbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_KEYBOARD_MODE));
//   }

// FIXME - this is now part of the colour selection group

//   else if(iParameter == BP_PALETTE_CHANGE) {
//     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "palettebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_PALETTE_CHANGE));
//   }

  // See if AppSettings wants to do anything:

  // Hmm - this seems to be the wrong way around...

  handle_core_change(iParameter);

}

void main_handle_parameter_change(int iParameter) {
  switch(iParameter) {
  case APP_SP_EDIT_FONT:
    set_editbox_font(get_app_parameter_string(APP_SP_EDIT_FONT));
    break;
  case APP_LP_STYLE:
    SetupPositioning();
    break;
  }
}


///
/// Load the window state (dimensions etc.) from GConf
///

void LoadWindowState() {

  int iWindowWidth;
  int iWindowHeight;
  int iEditHeight;

  GError *pGConfError = NULL;
  GConfValue *pGConfValue;

  pGConfValue = gconf_client_get_without_default(g_pGConfClient, "/apps/dasher/ScreenWidth", &pGConfError);

  if(!pGConfValue)
    return;

  iWindowWidth = gconf_value_get_int(pGConfValue);
  gconf_value_free(pGConfValue);

  pGConfValue = gconf_client_get_without_default(g_pGConfClient, "/apps/dasher/ScreenHeight", &pGConfError); 

  if(!pGConfValue)
    return;

  iWindowHeight = gconf_value_get_int(pGConfValue);
  gconf_value_free(pGConfValue);

  gtk_window_set_default_size(GTK_WINDOW(window), iWindowWidth, iWindowHeight);

  pGConfValue = gconf_client_get_without_default(g_pGConfClient, "/apps/dasher/EditHeight", &pGConfError);

  if(!pGConfValue)
    return;
  
  iEditHeight = gconf_value_get_int(pGConfValue);
  gconf_value_free(pGConfValue);

  gtk_paned_set_position(GTK_PANED(glade_xml_get_widget(widgets, "hpaned1")), iEditHeight);
}

///
/// Save the window state to GConf
///

void SaveWindowState() {

  int iWindowWidth;
  int iWindowHeight;
  int iEditHeight;

  GError *pGConfError = NULL;

  gtk_window_get_size(GTK_WINDOW(window), &iWindowWidth, &iWindowHeight);

  gconf_client_set_int(g_pGConfClient, "/apps/dasher/ScreenWidth", iWindowWidth, &pGConfError);
  gconf_client_set_int(g_pGConfClient, "/apps/dasher/ScreenHeight", iWindowHeight, &pGConfError);

  iEditHeight = gtk_paned_get_position(GTK_PANED(glade_xml_get_widget(widgets, "hpaned1")));

  gconf_client_set_int(g_pGConfClient, "/apps/dasher/EditHeight", iEditHeight, &pGConfError);
}

///
/// Signal handler for "dasher_start" events - emitted whenever Dasher is started
///

extern "C" void handle_start_event(GtkDasherControl *pDasherControl, gpointer data) {
  set_mark();
}

///
/// Signal handler for "dasher_stop" events - emitted whenever Dasher
/// is stopped. This is the place to deal with things like speak on
/// stop, copy all on stop etc.
///

extern "C" void handle_stop_event(GtkDasherControl *pDasherControl, gpointer data) {
  if(get_app_parameter_bool(APP_BP_SPEECH_MODE))
    SPEAK_DAMN_YOU(get_new_text());

  if(get_app_parameter_bool(APP_BP_COPY_ALL_ON_STOP))
    gtk2_clipboard_callback(CLIPBOARD_COPYALL);

  // Send the text to an external application

  //  SendText(get_new_text());
}

extern "C" void handle_context_request(GtkDasherControl * pDasherControl, gint iMaxLength, gpointer data) {
  RefreshContext(iMaxLength);
};

///
/// Signal handler for control nodes
///


extern "C" void handle_control_event(GtkDasherControl *pDasherControl, gint iEvent, gpointer data) {

  if(!edit_handle_control_event(iEvent)) {
    switch( iEvent ) {
    case Dasher::CControlManager::CTL_USER+1: // Speak all
      SPEAK_DAMN_YOU(get_all_text());
      break;
    case Dasher::CControlManager::CTL_USER+2: // Speak new
      SPEAK_DAMN_YOU(get_new_text());
      break;
    case Dasher::CControlManager::CTL_USER+3: // Repeat speech
      repeat_speech();
      break;
    }
  }
}

extern "C" bool focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
  return grab_focus();
}

extern "C" void handle_request_settings(GtkDasherControl * pDasherControl, gpointer data) {
  preferences_display(0,0);
}

extern "C" void RestoreButton_Clicked(GtkWidget *widget, gpointer user_data) {
#ifdef PJC_EXPERIMENTAL
  gtk_widget_show(window);
  gtk_widget_hide(g_pHiddenWindow);
  //  SetupPositioning();
  //  SetupWMHints();
#endif
}

extern "C" void DoneButton_Clicked(GtkWidget *widget, gpointer user_data) {
  SendText(get_all_text());

  if(g_pAction)
    dasher_action_execute(g_pAction, get_all_text());

  clear_edit();
  gtk_dasher_control_invalidate_context(GTK_DASHER_CONTROL(pDasherWidget));

  //  gtk_widget_show(g_pHiddenWindow);
  // gtk_widget_hide(window);
  // SetupPositioning();
  //SetupWMHints();
}

extern "C" void CancelButton_Clicked(GtkWidget *widget, gpointer user_data) {  
  clear_edit();
  gtk_dasher_control_invalidate_context(GTK_DASHER_CONTROL(pDasherWidget));
  //  gtk_widget_show(g_pHiddenWindow);
  //  gtk_widget_hide(window);
  //  SetupPositioning();
  // SetupWMHints();
}

// -------------
// Everything below here is either obsolete, or should definitely be moved to a new home
// -------------

#if GTK_CHECK_VERSION(2,3,0)

/* Fudge to avoid Glade complaining about being unable to find this signal 
   handler */
extern "C" void filesel_hide(GtkWidget *widget, gpointer user_data) {
  return;
}

#endif

// Some markers for stuff which is no longer handled by the core

//     case INT_ENCODING: // Shouldn't be done through core, probably shouldn't be done at all
//     case BOOL_SHOWTOOLBAR: // Shouldn't be done through core
//       gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"toolbarbutton")), value);
//       showtoolbar=value;

//       if (toolbar==NULL) 
//      break;

// #ifndef WITH_GPE // Don't show the toolbar if running under GPE
//       if (value) {
//      if (textentry==FALSE) {
//        gtk_widget_show(toolbar);
//      }
//       } else {
//      gtk_widget_hide(toolbar);
//       }
// #endif

//       break;
//     case BOOL_TIMESTAMPNEWFILES: // Not in core
//     case BOOL_WINDOWPAUSE: // Not in core
//     case BOOL_SPEECHMODE: // Not in core

extern "C" void file_encoding(GtkWidget *widget, gpointer user_data) {
  //  signed int realaction = action -3;
  //  if( GTK_TOGGLE_BUTTON(widget)->active) {
  //    dasher_set_encoding( Dasher::Opts::FileEncodingFormats(realaction) );
  //  }
  //  interface->SetFileEncoding(Opts::FileEncodingFormats(realaction));
  //FIXME - need to reimplement this
  //Actually, I'd be inclined to just get rid of the damn thing.
  //When we open the file, we have no real idea what format it's in - 
  //We assume that it's UTF-8 because that's sane. We should save files
  //based on the user locale and just forget about it.
}


// Probably obsolete button preferences code

extern "C" void button_cyclical_mode(GtkWidget *widget, gpointer user_data) {
  cyclickeyboardmodeon = GTK_TOGGLE_BUTTON(widget)->active;
  // FIXME - no longer have access to registry here
  //  set_bool_option_callback("Cyclicalbuttons",cyclickeyboardmodeon);
}


extern "C" gboolean button_coordinates_changed(GtkWidget *widget, gpointer user_data) {
  GtkSpinButton *spinbutton = GTK_SPIN_BUTTON(widget);
  int value = int (gtk_spin_button_get_value(spinbutton));

  // Really dreadfully hacky stuff to avoid recursion
  //
  // The recursion only seems to happen if the value ends up as 0, so
  // if we read a 0 twice in a row from the same widget then just break
  // out and assume that it really is a 0
  if(coordcalled == true && value == 0) {
    return true;
  }
  else if(value == 0) {
    coordcalled = true;
  }
  gtk_spin_button_update(spinbutton);
  coordcalled = false;
  value = int (gtk_spin_button_get_value(spinbutton));

  // FIXME - registry stuff is broken now - this probably shouldn't be in the UI anyway - put in the core

  // FIXME
  // See previous comment about enums
  // (Mind you, the whole of this is a mess anyway...)
  if(widget == glade_xml_get_widget(widgets, "spinbutton1")) {
    buttons[1].x = value;
    //    set_long_option_callback("Button1X",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton2")) {
    buttons[2].x = value;
    //  set_long_option_callback("Button2X",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton3")) {
    buttons[3].x = value;
    //  set_long_option_callback("Button3X",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton4")) {
    buttons[4].x = value;
    //  set_long_option_callback("Button4X",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton5")) {
    buttons[5].x = value;
    //  set_long_option_callback("Button5X",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton6")) {
    buttons[6].x = value;
    //  set_long_option_callback("Button6X",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton7")) {
    buttons[7].x = value;
    // set_long_option_callback("Button7X",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton8")) {
    buttons[8].x = value;
    // set_long_option_callback("Button8X",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton9")) {
    buttons[9].x = value;
    // set_long_option_callback("Button9X",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton10")) {
    buttons[1].y = value;
    // set_long_option_callback("Button1Y",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton11")) {
    buttons[2].y = value;
    // set_long_option_callback("Button2Y",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton12")) {
    buttons[3].y = value;
    // set_long_option_callback("Button3Y",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton13")) {
    buttons[4].y = value;
    //set_long_option_callback("Button4Y",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton14")) {
    buttons[5].y = value;
    // set_long_option_callback("Button5Y",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton15")) {
    buttons[6].y = value;
    //set_long_option_callback("Button6Y",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton16")) {
    buttons[7].y = value;
    //set_long_option_callback("Button7Y",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton17")) {
    buttons[8].y = value;
    //set_long_option_callback("Button8Y",value);
  }
  else if(widget == glade_xml_get_widget(widgets, "spinbutton18")) {
    buttons[9].y = value;
    //set_long_option_callback("Button9Y",value);
  }
  return FALSE;
}

//void interface_setup(GladeXML *xml) {

  // What's this doing here? I'm sure we ought to just be using whatever
  // the core provides us with
  // float initial_bitrate = 3.0;

  //  the_canvas=glade_xml_get_widget(xml, "the_canvas");

  // Needed so we can make it visible or not as we wish

  // FIXME - all of the stuff below commented out - we no longer have access to the registry here - need to fix it

//   // interface specific preferences
//   if(get_long_option_callback("Mouseposstartdistance",&mouseposstartdist)!=false) {
//     gtk_range_set_value(GTK_RANGE(glade_xml_get_widget(widgets,"mouseposstartscale")),mouseposstartdist);
//   }

//   if(get_long_option_callback("YScale",&yscale)!=false) {
//     gtk_range_set_value(GTK_RANGE(glade_xml_get_widget(widgets,"yaxisscale")),yscale);
//   }

//   if(get_bool_option_callback("Cyclicalbuttons",&cyclickeyboardmodeon)!=false) {
//     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets,"cyclicalbuttons")),cyclickeyboardmodeon);
//   }

//   // Configure the buttons. FIXME - more enums?
//   if (get_long_option_callback("Button1X",&(buttons[1].x))==false) {
//     buttons[1].x=0;
//   }
//   if (get_long_option_callback("Button2X",&(buttons[2].x))==false) {
//     buttons[2].x=0;
//   }
//   if (get_long_option_callback("Button3X",&(buttons[3].x))==false) {
//     buttons[3].x=0;
//   }
//   if (get_long_option_callback("Button4X",&(buttons[4].x))==false) {
//     buttons[4].x=0;
//   }
//   if (get_long_option_callback("Button5X",&(buttons[5].x))==false) {
//     buttons[5].x=0;
//   }
//   if (get_long_option_callback("Button6X",&(buttons[6].x))==false) {
//     buttons[6].x=0;
//   }
//   if (get_long_option_callback("Button7X",&(buttons[7].x))==false) {
//     buttons[7].x=0;
//   }
//   if (get_long_option_callback("Button8X",&(buttons[8].x))==false) {
//     buttons[8].x=0;
//   }
//   if (get_long_option_callback("Button9X",&(buttons[9].x))==false) {
//     buttons[9].x=0;
//   }
//   if (get_long_option_callback("Button1Y",&(buttons[1].y))==false) {
//     buttons[1].y=0;
//   }
//   if (get_long_option_callback("Button2Y",&(buttons[2].y))==false) {
//     buttons[2].y=0;
//   }
//   if (get_long_option_callback("Button3Y",&(buttons[3].y))==false) {
//     buttons[3].y=0;
//   }
//   if (get_long_option_callback("Button4Y",&(buttons[4].y))==false) {
//     buttons[4].y=0;
//   }
//   if (get_long_option_callback("Button5Y",&(buttons[5].y))==false) {
//     buttons[5].y=0;
//   }
//   if (get_long_option_callback("Button6Y",&(buttons[6].y))==false) {
//     buttons[6].y=0;
//   }
//   if (get_long_option_callback("Button7Y",&(buttons[7].y))==false) {
//     buttons[7].y=0;
//   }
//   if (get_long_option_callback("Button8Y",&(buttons[8].y))==false) {
//     buttons[8].y=0;
//   }
//   if (get_long_option_callback("Button9Y",&(buttons[9].y))==false) {
//     buttons[9].y=0;
//   }

//}

//void
//interface_late_setup() {
  // Stuff that needs to be done after the core has
  // set itself up
  //  dasher_set_parameter_int(INT_ONEBUTTON, 0);
//   alphabet=dasher_get_current_alphabet();
//   colourscheme=dasher_get_current_colours();

//}

//void
//open_window(GladeXML *xml) {

  // I have no idea why we need to do this when Glade has theoretically done
  // so already, but...

  // FIXME - REIMPLEMENT

//}
