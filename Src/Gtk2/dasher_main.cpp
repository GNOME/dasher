#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "dasher.h"
#ifdef WITH_MAEMO
#include "dasher_maemo_helper.h"
#endif
#include "dasher_main.h"
#include "GtkDasherControl.h"

struct _DasherMainPrivate {
  GladeXML *pGladeXML;
  DasherAppSettings *pAppSettings;

  // Various widgets which need to be cached:
  GtkWidget *pActionPane;
  GtkWidget *pBufferView;
  GtkWidget *pDivider;
  GtkWidget *pEditPane;
  GtkWidget *pMainWindow;
  GtkWidget *pToolbar;
#ifdef WITH_MAEMO
  DasherMaemoHelper *pMaemoHelper;
#endif
};

typedef struct _DasherMainPrivate DasherMainPrivate;

// Private member functions

static void dasher_main_class_init(DasherMainClass *pClass);
static void dasher_main_init(DasherMain *pMain);
static void dasher_main_destroy(GObject *pObject);
static void dasher_main_refresh_font(DasherMain *pSelf);
static GtkWidget *dasher_main_create_dasher_control(DasherMain *pSelf);
static void dasher_main_on_map(DasherMain *pSelf);
static void dasher_main_setup_window_position(DasherMain *pSelf);
static void dasher_main_setup_window_style(DasherMain *pSelf, bool bTopMost);

GType dasher_main_get_type() {

  static GType dasher_main_type = 0;

  if(!dasher_main_type) {
    static const GTypeInfo dasher_main_info = {
      sizeof(DasherMainClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_main_class_init,
      NULL,
      NULL,
      sizeof(DasherMain),
      0,
      (GInstanceInitFunc) dasher_main_init,
      NULL
    };

    dasher_main_type = g_type_register_static(G_TYPE_OBJECT, "DasherMain", &dasher_main_info, static_cast < GTypeFlags > (0));
  }

  return dasher_main_type;
}

static void dasher_main_class_init(DasherMainClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_main_destroy;
}

static void dasher_main_init(DasherMain *pDasherControl) {
  pDasherControl->private_data = new DasherMainPrivate;
  
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pDasherControl->private_data);
  pPrivate->pAppSettings = 0;

  dasher_main_load_interface(pDasherControl);
  dasher_main_setup_window(pDasherControl);

  // TODO: Rationalise this
  gtk_window_set_accept_focus(GTK_WINDOW(pPrivate->pMainWindow), false);
  gtk_window_set_focus_on_map(GTK_WINDOW(pPrivate->pMainWindow), false);
}

static void dasher_main_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

// Public methods

DasherMain *dasher_main_new() {
  DasherMain *pDasherControl;
  pDasherControl = (DasherMain *)(g_object_new(dasher_main_get_type(), NULL));

  return pDasherControl;
}

void dasher_main_load_interface(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  const char *szGladeFilename;

#ifdef WITH_GPE
  szGladeFilename = PROGDATA "/dashergpe.glade";
#elif WITH_MAEMO
  szGladeFilename = "/var/lib/install" PROGDATA "/dashermaemo.glade";
  //xml = glade_xml_new(PROGDATA "/dashermaemo.glade", NULL, NULL);
#else
  szGladeFilename = PROGDATA "/dasher.glade";
#endif

  pPrivate->pGladeXML = glade_xml_new(PROGDATA "/dasher.glade", NULL, NULL);

  if (!pPrivate->pGladeXML) {
    g_error("Can't find Glade file: %s. Dasher is unlikely to be correctly installed.", szGladeFilename);
  }

  glade_xml_signal_autoconnect(pPrivate->pGladeXML);

  // Save the details of some of the widgets for later
  pPrivate->pActionPane = glade_xml_get_widget(pPrivate->pGladeXML, "vbox39");
  pPrivate->pBufferView = glade_xml_get_widget(pPrivate->pGladeXML, "the_text_view");
  pPrivate->pDivider = glade_xml_get_widget(pPrivate->pGladeXML, "hpaned1");
  pPrivate->pEditPane = glade_xml_get_widget(pPrivate->pGladeXML, "vbox40");
  pPrivate->pMainWindow = glade_xml_get_widget(pPrivate->pGladeXML, "window");
  pPrivate->pToolbar = glade_xml_get_widget(pPrivate->pGladeXML, "toolbar");

  // Create a Maemo helper if necessary
#ifdef WITH_MAEMO
  pPrivate->pMaemoHelper = dasher_maemo_helper_new(pPrivate->pBufferView);
#endif

  // Set up any non-registry-dependent options
#ifdef WITH_GPE
  gtk_window_set_decorated(GTK_WINDOW(pPrivate->pMainWindow), false);
#endif
}

void dasher_main_setup_window(DasherMain *pSelf) {
}

void dasher_main_handle_parameter_change(DasherMain *pSelf, int iParameter) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  switch( iParameter ) {
  case APP_BP_SHOW_TOOLBAR:
    if( dasher_app_settings_get_bool(pPrivate->pAppSettings, APP_BP_SHOW_TOOLBAR))
      gtk_widget_show(pPrivate->pToolbar);
    else
      gtk_widget_hide(pPrivate->pToolbar);
    break;
  case APP_SP_EDIT_FONT:
    dasher_main_refresh_font(pSelf);
    break;
  case APP_LP_STYLE:
    dasher_main_on_map(pSelf);
    break;
  }

}

GladeXML *dasher_main_get_glade(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  return pPrivate->pGladeXML;
}

void dasher_main_set_app_settings(DasherMain *pSelf, DasherAppSettings *pAppSettings) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  pPrivate->pAppSettings = pAppSettings;

  // Now we have access to the settings, we can set up the intial
  // values
  
#ifndef WITH_MAEMO
  if(dasher_app_settings_get_bool(pPrivate->pAppSettings, APP_BP_SHOW_TOOLBAR)) {
    gtk_widget_show(pPrivate->pToolbar);
  }

  dasher_main_load_state(pSelf);
#endif

  dasher_main_refresh_font(pSelf);
}

void dasher_main_load_state(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  int iWindowWidth;
  int iWindowHeight;
  int iEditHeight;
  
  iWindowWidth = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_SCREEN_WIDTH);
  iWindowHeight = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_SCREEN_HEIGHT);
  gtk_window_set_default_size(GTK_WINDOW(pPrivate->pMainWindow), iWindowWidth, iWindowHeight);
  
  iEditHeight = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_EDIT_HEIGHT);
  gtk_paned_set_position(GTK_PANED(pPrivate->pDivider), iEditHeight);
}

void dasher_main_save_state(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

   int iWindowWidth;
   int iWindowHeight;
   int iEditHeight;

   gtk_window_get_size(GTK_WINDOW(pPrivate->pMainWindow), &iWindowWidth, &iWindowHeight);
   dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_WIDTH, iWindowWidth);
   dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_HEIGHT, iWindowHeight);

   iEditHeight = gtk_paned_get_position(GTK_PANED(pPrivate->pDivider));
   dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_EDIT_HEIGHT, iEditHeight);
}

void dasher_main_refresh_font(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  const gchar *szFontName = dasher_app_settings_get_string(pPrivate->pAppSettings, APP_SP_EDIT_FONT);
  
  if(!strcmp(szFontName, "")) {
    gtk_widget_modify_font(pPrivate->pBufferView, pango_font_description_from_string(szFontName));
  }
}

void dasher_main_show(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  gtk_widget_show(pPrivate->pMainWindow);
}

GtkWidget *dasher_main_create_dasher_control(DasherMain *pSelf) {
  GtkWidget *pDasherControl = gtk_dasher_control_new();

#ifdef WITH_MAEMO
  gtk_widget_set_size_request(pDasherControl, 175, -1);  
#endif

  return pDasherControl;
}


// TODO: Rationalise window setup functions
void dasher_main_setup_window_position(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

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
    gtk_widget_show(pPrivate->pActionPane);
  }
  else {
    gtk_widget_hide(pPrivate->pActionPane);
  }

  if(bShowEdit) {
    gtk_widget_show(pPrivate->pEditPane);
  }
  else {
    gtk_widget_hide(pPrivate->pEditPane);
  }

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

// TODO: Don't pass topmost etc - store in object
void dasher_main_setup_window_style(DasherMain *pSelf, bool bTopMost) {
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

void dasher_main_on_map(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

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
  
  Atom atom_type[1];
  atom_type[0] = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE_INPUT");
  
  Atom atom_window_type = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE");
  
  XChangeProperty(GDK_WINDOW_XDISPLAY(pWidget->window),
		  GDK_WINDOW_XWINDOW(pWidget->window),
		  atom_window_type,
		  XA_ATOM, 32, PropModeReplace,
		  (guchar *)&atom_type, 1);

#else

  dasher_main_setup_window_position(pSelf);
  // TODO: put topmost back here
  dasher_main_setup_window_style(pSelf, false);
  
#endif

#ifdef WITH_MAEMO
  dasher_maemo_helper_setup_window(pPrivate->pMainWindow);
#endif
}

void dasher_main_set_filename(DasherMain *pSelf, const gchar *szFilename) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  if(szFilename == 0) {
    gtk_window_set_title(GTK_WINDOW(pPrivate->pMainWindow), "Dasher");
  }
  else {
    // TODO: Prepend 'Dasher - ' to filename?
    gtk_window_set_title(GTK_WINDOW(pPrivate->pMainWindow), szFilename);
  }
}

// Callbacks

extern "C" GtkWidget *create_dasher_control(gchar *szName, gchar *szString1, gchar *szString2, gint iInt1, gint iInt2) {
  return dasher_main_create_dasher_control(g_pDasherMain);
}

extern "C" void on_window_map(GtkWidget* pWidget, gpointer pUserData) {
  dasher_main_on_map(g_pDasherMain);
}

// TODO: Incorporate this into class
gboolean g_bForwardKeyboard(false);

gboolean grab_focus() {
  gtk_widget_grab_focus(the_text_view);
  g_bForwardKeyboard = true;
  return true;
}

// TODO: Not really sure what happens here - need to sort out focus behaviour in general
extern "C" bool focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
  return grab_focus();
}

// TODO: Next three handlers should just forward into class
extern "C" gboolean take_real_focus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
  g_bForwardKeyboard = false;
  return false;
}

extern "C" gboolean edit_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
  if(g_bForwardKeyboard) {
    gboolean *returnType;
    g_signal_emit_by_name(GTK_OBJECT(pDasherWidget), "key_press_event", event, &returnType);
    return true;
  }
  else {
    return false;
  }
}

extern "C" gboolean edit_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data) { 
  if(g_bForwardKeyboard) {
    gboolean *returnType;
    g_signal_emit_by_name(GTK_OBJECT(pDasherWidget), "key_release_event", event, &returnType);
    return true;
  }
  else {
    return false;
  }
}
