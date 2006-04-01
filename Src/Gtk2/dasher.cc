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
#include <gtk/gtkimmodule.h>
#include <gtk/gtkimmulticontext.h>

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
#include "fileops.h"
#include "Preferences.h"
#include "FontDialogues.h"
#include "Menu.h"
#include "AppSettings.h"
#include "../DasherCore/Parameters.h"
#include "accessibility.h"
#include "dasher_lock_dialogue.h"

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

//DasherAction *g_pAction = 0;

const gchar *filename = NULL;   // Filename of file currently being edited

// Apparently not obsolete, but should be sorted out

gboolean file_modified = FALSE; // Have unsaved changes been made to the current file
gint outputcharacters;

const char *g_szAccessibleContext = 0;
int g_iExpectedPosition = -1;
int g_iOldPosition = -1;

// 'Private' methods

void LoadWindowState();
void InitialiseAppParameters();
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

GdkWindow *g_pFocusWindow;
GdkWindow *g_pRandomWindow;
Window g_FWindow;
Window g_FRandomWindow;

GdkFilterReturn peek_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data) {
  //  g_message("foo");

  XEvent *xev = (XEvent *) xevent;
  if(xev->xany.type == ClientMessage) {
    g_message("Atom: %s %d", XGetAtomName(xev->xany.display, xev->xclient.message_type), xev->xclient.format );
    
    for(int i(0); i < 5; ++i) {
      g_print("%x ", (unsigned int)(xev->xclient.data.l[i]));
    }

    g_print("\n");
    

     if(xev->xclient.data.l[3] == 0xd) {
       gtk_widget_show(GTK_WIDGET(data));
       g_FRandomWindow = (Window)xev->xclient.data.l[0];
       g_FWindow = (Window)xev->xclient.data.l[1];
       g_pFocusWindow = gdk_window_foreign_new(g_FWindow);
       g_pRandomWindow = gdk_window_foreign_new(g_FRandomWindow);
       gdk_window_set_transient_for(GDK_WINDOW(GTK_WIDGET(data)->window), g_pFocusWindow);
     } 
     else if(xev->xclient.data.l[3] == 0x12) {
       GdkEventClient sMyEvent;
       
       sMyEvent.type = GDK_CLIENT_EVENT;
       sMyEvent.window = g_pRandomWindow;
       sMyEvent.send_event = true;
       sMyEvent.message_type = gdk_atom_intern("_HILDON_IM_COM", true);
       sMyEvent.data_format = 8; // I know this is wrong...
       sMyEvent.data.l[0] = g_FRandomWindow;
       sMyEvent.data.l[1] = 0x7;
       sMyEvent.data.l[2] = 0;
       sMyEvent.data.l[3] = 0;
       sMyEvent.data.l[4] = 0;
       
       gdk_event_send_client_message((GdkEvent *)(&sMyEvent), g_FRandomWindow); 
     }
     else if(xev->xclient.data.l[3] == 0xb) {
       gtk_widget_hide(GTK_WIDGET(data));
    }
  }
  
return GDK_FILTER_CONTINUE;
}

void enter_text(const char *szText) {
  GdkEventClient sMyEvent;

  for(int i(0); i < strlen(szText); ++i) {
    sMyEvent.type = GDK_CLIENT_EVENT;
    sMyEvent.window = g_pRandomWindow;
    sMyEvent.send_event = true;
    sMyEvent.message_type = gdk_atom_intern("_HILDON_IM_INSERT_UTF8", true);
    sMyEvent.data_format = 8; // I know this is wrong...
    sMyEvent.data.l[0] = 0;
    sMyEvent.data.l[1] = szText[i];
    sMyEvent.data.l[2] = 0;
    sMyEvent.data.l[3] = 0;
    sMyEvent.data.l[4] = 0;

    gdk_event_send_client_message((GdkEvent *)(&sMyEvent), g_FRandomWindow); 
  }

  
}

extern "C" void on_y_clicked(GtkWidget* pWidget, gpointer pUserData) {
  enter_text(dasher_editor_get_all_text(g_pEditor));
  dasher_editor_clipboard(g_pEditor, CLIPBOARD_CLEAR);
}

extern "C" void on_window_map(GtkWidget* pWidget, gpointer pUserData) {
//   if(g_bOnTop)
//     gtk_window_set_keep_above(GTK_WINDOW(window), true);
#ifdef WITH_MAEMO

   Window xThisWindow = GDK_WINDOW_XWINDOW(pWidget->window);
   Atom atom_im_window = gdk_x11_get_xatom_by_name("_HILDON_IM_WINDOW");

   XChangeProperty(GDK_WINDOW_XDISPLAY(pWidget->window),
 		  GDK_WINDOW_XWINDOW(gdk_screen_get_root_window (gdk_screen_get_default ())),
 		  atom_im_window,
 		  XA_WINDOW, 32, PropModeReplace,
 		  (guchar *)&xThisWindow, 1);

  gdk_window_add_filter(GDK_WINDOW(pWidget->window), peek_filter, pWidget);
//  gdk_window_add_filter(0, peek_filter, pWidget);

 //  Window xFocusWindow;
//   int xState;
//   XGetInputFocus(GDK_WINDOW_XDISPLAY(pWidget->window),
//  		 &xFocusWindow,
//  		 &xState);
//   GdkWindow *pFocusWindow = gdk_window_foreign_new(xFocusWindow);
//   gdk_window_set_transient_for(GDK_WINDOW(pWidget->window), pFocusWindow);
  
  Atom atom_type[1];
  atom_type[0] = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE_INPUT");
  
  Atom atom_window_type = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE");
  
  XChangeProperty(GDK_WINDOW_XDISPLAY(pWidget->window),
		  GDK_WINDOW_XWINDOW(pWidget->window),
		  atom_window_type,
		  XA_ATOM, 32, PropModeReplace,
		  (guchar *)&atom_type, 1);

#else

  //  SetupWMHints();
  SetupPositioning();
  // A11y support disabled for now
  setupa11y();
#endif


}

void InitialiseMainWindow(int argc, char **argv, GladeXML *pGladeXML) {

  //  init_app_settings();
  //  load_app_parameters();

  dasher_accel = gtk_accel_group_new(); //?

    widgets = pGladeXML;          // obsolete? NO - used later in this file, but should be
  // Grab some pointers to important GTK widgets from the Glade XML
  // FIXME - do we actually need all of these?

  window = glade_xml_get_widget(pGladeXML, "window");
  gtk_window_set_accept_focus(GTK_WINDOW(window), false);
  gtk_window_set_focus_on_map(GTK_WINDOW(window), false);

  toolbar = glade_xml_get_widget(pGladeXML, "toolbar");
  vbox = glade_xml_get_widget(pGladeXML, "vbox1");
  vpane = glade_xml_get_widget(pGladeXML, "hpaned1");
  dasher_menu_bar = glade_xml_get_widget(pGladeXML, "dasher_menu_bar");
  g_pEditPane = glade_xml_get_widget(pGladeXML, "vbox40");
  g_pActionPane = glade_xml_get_widget(pGladeXML, "vbox39");
  pDasherWidget = glade_xml_get_widget(pGladeXML, "DasherControl");

  dasher_lock_dialogue_new(pGladeXML, GTK_WINDOW(window));

#ifndef WITH_MAEMO
  if( dasher_app_settings_get_bool(g_pDasherAppSettings,  APP_BP_SHOW_TOOLBAR ) ) {
    gtk_widget_show( toolbar );
  }
#endif

#ifndef GNOME_SPEECH
  // This ought to be greyed out if not built with speech support
  gtk_widget_set_sensitive(glade_xml_get_widget(pGladeXML, "speakbutton"), false);
#endif

#ifndef JAPANESE
  gtk_widget_hide(glade_xml_get_widget(pGladeXML, "radiobutton9"));
#endif

  // Initialise the various components

  initialise_edit(pGladeXML);
#ifndef WITH_MAEMO
  PopulateMenus(pGladeXML);
#endif
  initialise_preferences_dialogue(pGladeXML);
  InitialiseFontDialogues(pGladeXML);

#ifndef WITH_MAEMO
  LoadWindowState();
#endif

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

  //  g_pAction = DASHER_ACTION(dasher_action_keyboard_new());  

  // Hacky stuff to figure out how the hildon input method works

//   GdkAtom atom, type, actual_type;
  
//   gint actual_format, actual_length;
//   guchar *context_id;
  
//   atom = gdk_atom_intern("gtk-global-immodule", FALSE);
//   type = gdk_atom_intern("STRING", FALSE);
//   if(!gdk_property_get(gdk_screen_get_root_window (gdk_screen_get_default ()),
// 		       atom,
// 		       type,
// 		       0,
// 		       G_MAXLONG,
// 		       FALSE,
// 		       &actual_type,
// 		       &actual_format,
// 		       &actual_length,
// 		       &context_id)) {
//     gchar *locale = _gtk_get_lc_ctype ();
//     context_id = (guchar*)_gtk_im_module_get_default_context_id (locale);
//     g_free (locale);
//   }

//   g_message("Context ID: %s", context_id); 

//   gpointer pObject = _gtk_im_module_create ((gchar*)context_id);

//   g_message("Type: %s", g_type_name(G_OBJECT_TYPE(pObject)));

//   guint *pIDs;
//   guint iNumIDs;
//   pIDs = g_signal_list_ids(G_OBJECT_TYPE(pObject), &iNumIDs);

//   for(int i(0); i < iNumIDs; ++i) {
//     g_message("%d - %s", pIDs[i], g_signal_name( pIDs[i]));
//   }

//   DasherIMContext *pIMContext = dasher_im_context_new();


  
}

///
/// Functions that need to be called as we're shutting down
/// This gets called after the Gtk loop has exited, so the widgets will no longer exist
///

void interface_cleanup() {
  cleanup_edit();
  //  delete_app_settings();
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

  if(bTopMost) {
    XSetWMHints(GDK_WINDOW_XDISPLAY(window->window), GDK_WINDOW_XWINDOW(window->window), &wm_hints);
    XSetWMProtocols(GDK_WINDOW_XDISPLAY(window->window), GDK_WINDOW_XWINDOW(window->window), wm_window_protocols, 3);
    gdk_window_add_filter(window->window, dasher_discard_take_focus_filter, NULL);
  }
  else {
    gdk_window_remove_filter(window->window, dasher_discard_take_focus_filter, NULL);
  }
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

  switch(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE)) {
  case 0:
    bShowEdit = true;
    bShowActions = false;
    bTopMost = false;
    break;
  case 1:
    bShowEdit = true;
    bShowActions = true;
    bTopMost = true;
    break;
  case 2:
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

  // This is neat, but needs to be thought through

  int iTargetWidth;
  int iTargetHeight;
  int iScreenWidth = gdk_screen_width();
  int iScreenHeight = gdk_screen_height();
  int iScreenTop;
  int iLeft;
  int iTop;


  Atom atom_strut_partial = gdk_x11_get_xatom_by_name("_NET_WM_STRUT_PARTIAL");
  guint32 struts[12] = {0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0 ,0 ,0};

  XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
  		  GDK_WINDOW_XWINDOW(window->window),
  		  atom_strut_partial,
  		  XA_CARDINAL, 32, PropModeReplace,
  		  (guchar *)&struts, 12);

  int iDockPosition(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_DOCK_STYLE));

  if(iDockPosition < 4) {
    gtk_window_unfullscreen(GTK_WINDOW(window));
  gtk_window_get_size(GTK_WINDOW(window), &iTargetWidth, &iTargetHeight);

  // TODO: Need full struts for old window managers

  Atom atom_work_area = gdk_x11_get_xatom_by_name("_NET_WORKAREA");
  Atom aReturn;
  int iFormatReturn;
  unsigned long iItemsReturn;
  unsigned long iBytesAfterReturn;
  unsigned char *iData;

  XGetWindowProperty(GDK_WINDOW_XDISPLAY(GDK_ROOT_PARENT()),
		     GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()),
		     atom_work_area,
		     0, 4, false,
		     XA_CARDINAL, 
		     &aReturn,
		     &iFormatReturn,
		     &iItemsReturn,
		     &iBytesAfterReturn,
		     &iData);

  // TODO: need to use width here
  // TODO: need more error checking with raw X11 stuff

  iScreenTop = ((unsigned long *)iData)[1];
  iScreenHeight = ((unsigned long *)iData)[3];

  XFree(iData);

  switch(iDockPosition) {
  case 0: // Top left
    struts[0] = iTargetWidth;
    struts[4] = iScreenTop;
    struts[5] = iTargetHeight + iScreenTop;

    iLeft = 0;
    iTop = iScreenTop;
    break;
  case 1: // Top right
    struts[1] = iTargetWidth;
    struts[6] = iScreenTop;
    struts[7] = iTargetHeight + iScreenTop;

    iLeft = iScreenWidth - iTargetWidth;
    iTop = iScreenTop;
    break;
  case 2: // Bottom left
    struts[0] = iTargetWidth;
    struts[4] = iScreenHeight + iScreenTop - iTargetHeight;
    struts[5] = iScreenHeight + iScreenTop;

    iLeft = 0;
    iTop =  iScreenHeight + iScreenTop - iTargetHeight;
    break;
  case 3: // Bottom right
    struts[1] = iTargetWidth;
    struts[6] = iScreenHeight + iScreenTop - iTargetHeight;
    struts[7] = iScreenHeight + iScreenTop;

    iLeft = iScreenWidth - iTargetWidth;
    iTop = iScreenHeight + iScreenTop - iTargetHeight;
    break;
  }
  
  XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
  		  GDK_WINDOW_XWINDOW(window->window),
  		  atom_strut_partial,
  		  XA_CARDINAL, 32, PropModeReplace,
  		  (guchar *)&struts, 12);
  
  Atom atom_type[1];
  atom_type[0] = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE_DOCK");
  
  Atom atom_window_type = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE");
  
  XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
		  GDK_WINDOW_XWINDOW(window->window),
		  atom_window_type,
		  XA_ATOM, 32, PropModeReplace,
		  (guchar *)&atom_type, 1);
  
  gdk_window_move_resize((GdkWindow *)window->window, iLeft, iTop, iTargetWidth, iTargetHeight);

  }
  else if(iDockPosition == 4) {
    Atom atom_type[1];
    atom_type[0] = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE_NORMAL");
    
    Atom atom_window_type = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE");
    
    XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
		    GDK_WINDOW_XWINDOW(window->window),
		    atom_window_type,
		    XA_ATOM, 32, PropModeReplace,
		    (guchar *)&atom_type, 1);

    gtk_window_unfullscreen(GTK_WINDOW(window));
  }
  else if(iDockPosition == 5) {
    Atom atom_type[1];
    atom_type[0] = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE_NORMAL");
    
    Atom atom_window_type = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE");
    
    XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
		    GDK_WINDOW_XWINDOW(window->window),
		    atom_window_type,
		    XA_ATOM, 32, PropModeReplace,
		    (guchar *)&atom_type, 1);


    gtk_window_fullscreen(GTK_WINDOW(window));
  }
  
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
    //    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "speedsliderbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_SHOW_SLIDER));
    RefreshWidget(iParameter);
  }
  else if(iParameter == BP_DRAW_MOUSE) {
    //   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "showmousebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_DRAW_MOUSE));
    RefreshWidget(iParameter);
  }
  else if(iParameter == BP_DRAW_MOUSE_LINE) {
    //    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "showmouselinebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_DRAW_MOUSE_LINE));
    RefreshWidget(iParameter);
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
    if(dasher_app_settings_get_bool(g_pDasherAppSettings, BP_MOUSEPOS_MODE)) {
      gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(widgets, "MousePosStyle")), 1);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "mouseposbutton")), true);
    }
    else if(dasher_app_settings_get_bool(g_pDasherAppSettings, BP_CIRCLE_START)) {
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
    //    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(widgets, "outlinebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_OUTLINE_MODE));
    RefreshWidget(iParameter);
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
    set_editbox_font(dasher_app_settings_get_string(g_pDasherAppSettings, APP_SP_EDIT_FONT));
    break;
  case APP_LP_STYLE:
    create_buffer();
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
  
  iWindowWidth = dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_SCREEN_WIDTH);
  iWindowHeight = dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_SCREEN_HEIGHT);
  gtk_window_set_default_size(GTK_WINDOW(window), iWindowWidth, iWindowHeight);
  
  iEditHeight = dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_EDIT_HEIGHT);
  gtk_paned_set_position(GTK_PANED(glade_xml_get_widget(widgets, "hpaned1")), iEditHeight);
}

///
/// Save the window state to GConf
///

void SaveWindowState() {
   int iWindowWidth;
   int iWindowHeight;
   int iEditHeight;

   gtk_window_get_size(GTK_WINDOW(window), &iWindowWidth, &iWindowHeight);
   dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_SCREEN_WIDTH, iWindowWidth);
   dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_SCREEN_HEIGHT, iWindowHeight);

   iEditHeight = gtk_paned_get_position(GTK_PANED(glade_xml_get_widget(widgets, "hpaned1")));
   dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_EDIT_HEIGHT, iEditHeight);
}

///
/// Signal handler for "dasher_start" events - emitted whenever Dasher is started
///

extern "C" void handle_start_event(GtkDasherControl *pDasherControl, gpointer data) {
  set_mark();
  dasher_editor_handle_start(g_pEditor);
}

///
/// Signal handler for "dasher_stop" events - emitted whenever Dasher
/// is stopped. This is the place to deal with things like speak on
/// stop, copy all on stop etc.
///

extern "C" void handle_stop_event(GtkDasherControl *pDasherControl, gpointer data) {
  dasher_editor_handle_stop(g_pEditor);
}

extern "C" void handle_context_request(GtkDasherControl * pDasherControl, gint iMaxLength, gpointer data) {
  RefreshContext(iMaxLength);
};

///
/// Signal handler for control nodes
///


extern "C" void handle_control_event(GtkDasherControl *pDasherControl, gint iEvent, gpointer data) {
  dasher_editor_handle_control(g_pEditor, iEvent);
  edit_handle_control_event(iEvent);
}

extern "C" bool focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
  return grab_focus();
}

extern "C" void handle_request_settings(GtkDasherControl * pDasherControl, gpointer data) {
  preferences_display(0,0);
}

extern "C" GtkWidget *create_dasher_control(gchar *szName, gchar *szString1, gchar *szString2, gint iInt1, gint iInt2) {
  GtkWidget *pDasherControl = gtk_dasher_control_new();
#ifdef WITH_MAEMO
  gtk_widget_set_size_request(pDasherControl, 175, -1);  
#endif
  return pDasherControl;
}

extern "C" void on_message(GtkDasherControl *pDasherControl, gpointer pMessageInfo, gpointer pUserData) {
  DasherMessageInfo *pInfo = (DasherMessageInfo *)pMessageInfo;
  
  GtkMessageDialog *pDialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, pInfo->szMessage));
  gtk_dialog_run(GTK_DIALOG(pDialog));

  gtk_widget_destroy(GTK_WIDGET(pDialog));
}
