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
#include "Menu.h"

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
  GtkWidget *pMenuBar;
  GtkWidget *pSideMenu;
  GtkWidget *pDragHandle; 
  GtkWidget *pOuterFrame;
  GtkWidget *pInnerFrame;
  GtkWidget *pSpeedBox;
  GtkWidget *pAlphabetCombo;

  GtkListStore *pAlphabetList;

  GtkAccelGroup *pAccel;

  int iComboCount;

#ifdef WITH_MAEMO
  DasherMaemoHelper *pMaemoHelper;
#endif

  // Properties of the window
  bool bShowEdit;
  bool bShowActions;
  bool bTopMost;
  bool bFullScreen;
  bool bShowSideMenu;

  bool bHidden;

  int iWidth;
  int iHeight;

  int iPosition;

  bool bGrabbed;
  double dDragOffsetX;
  double dDragOffsetY;
};

typedef struct _DasherMainPrivate DasherMainPrivate;

// Private member functions

static void dasher_main_class_init(DasherMainClass *pClass);
static void dasher_main_init(DasherMain *pMain);
static void dasher_main_destroy(GObject *pObject);
static void dasher_main_refresh_font(DasherMain *pSelf);
static GtkWidget *dasher_main_create_dasher_control(DasherMain *pSelf);
static void dasher_main_on_map(DasherMain *pSelf);
//static void dasher_main_on_map_real(DasherMain *pSelf);
static void dasher_main_setup_window_position(DasherMain *pSelf);
static void dasher_main_setup_window_style(DasherMain *pSelf, bool bTopMost);
static void dasher_main_setup_window_type(DasherMain *pSelf);
static void dasher_main_toggle_hidden(DasherMain *pSelf);
static void dasher_main_grab(DasherMain *pSelf, GdkEventButton *pEvent);
static void dasher_main_ungrab(DasherMain *pSelf, GdkEventButton *pEvent);
static gboolean dasher_main_motion(DasherMain *pSelf, GdkEventMotion *pEvent);
static gboolean dasher_main_speed_changed(DasherMain *pSelf);
static void dasher_main_populate_alphabet_combo(DasherMain *pSelf);

// Private functions not in class
extern "C" gboolean take_real_focus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
extern "C" gboolean edit_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
extern "C" gboolean edit_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
extern "C" GdkFilterReturn keyboard_filter_cb(GdkXEvent *xevent, GdkEvent *event, gpointer data);
extern "C" void on_window_map(GtkWidget* pWidget, gpointer pUserData);

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
  //szGladeFilename = PROGDATA "/dashermaemo.glade";
#else
  szGladeFilename = PROGDATA "/dasher.glade";
#endif

  g_message("Glade file is: %s", szGladeFilename);

  pPrivate->pGladeXML = glade_xml_new(szGladeFilename, NULL, NULL);

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
  pPrivate->pMenuBar = glade_xml_get_widget(pPrivate->pGladeXML, "dasher_menu_bar");
  pPrivate->pSideMenu = glade_xml_get_widget(pPrivate->pGladeXML, "SideMenu");
  pPrivate->pDragHandle = glade_xml_get_widget(pPrivate->pGladeXML, "button31");
  pPrivate->pOuterFrame = glade_xml_get_widget(pPrivate->pGladeXML, "OuterFrame");
  pPrivate->pInnerFrame = glade_xml_get_widget(pPrivate->pGladeXML, "vbox1"); 

#ifndef WITH_MAEMO
  pPrivate->pSpeedBox = glade_xml_get_widget(pPrivate->pGladeXML, "spinbutton1");
  pPrivate->pAlphabetCombo = glade_xml_get_widget(pPrivate->pGladeXML, "combobox1");

  pPrivate->pAlphabetList = gtk_list_store_new(1, G_TYPE_STRING);
  gtk_combo_box_set_model(GTK_COMBO_BOX(pPrivate->pAlphabetCombo), 
			  GTK_TREE_MODEL(pPrivate->pAlphabetList));

  GtkCellRenderer *pRenderer;
  pRenderer = gtk_cell_renderer_text_new();
  g_object_set(G_OBJECT(pRenderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(pPrivate->pAlphabetCombo), pRenderer, true);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(pPrivate->pAlphabetCombo), pRenderer, "text", 0, NULL);

  dasher_main_setup_window_type(pSelf);

  gtk_widget_add_events(pPrivate->pDragHandle, GDK_POINTER_MOTION_MASK);
#endif

  pPrivate->bHidden = false;
  pPrivate->bGrabbed = false;

  pPrivate->iPosition = 100; // FIXME - make this persistant

  // TODO: Specify callbacks in glade file
  // TODO: Rationalise focus
  g_signal_connect(G_OBJECT(pPrivate->pBufferView), "button-release-event", G_CALLBACK(take_real_focus), NULL);
  g_signal_connect(G_OBJECT(pPrivate->pBufferView), "key-press-event", G_CALLBACK(edit_key_press), NULL);
  g_signal_connect(G_OBJECT(pPrivate->pBufferView), "key-release-event", G_CALLBACK(edit_key_release), NULL);
  
  

  // Create a Maemo helper if necessary
#ifdef WITH_MAEMO
  pPrivate->pMaemoHelper = dasher_maemo_helper_new(GTK_WINDOW(pPrivate->pMainWindow));
#endif

  // Set up any non-registry-dependent options
#ifdef WITH_GPE
  gtk_window_set_decorated(GTK_WINDOW(pPrivate->pMainWindow), false);
#endif
}
void dasher_main_setup_window(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  if(dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_GLOBAL_KEYBOARD))
    gdk_window_add_filter(0, keyboard_filter_cb, 0);
   else
     gdk_window_remove_filter(0, keyboard_filter_cb, 0);
}


void dasher_main_handle_pre_parameter_change(DasherMain *pSelf, int iParameter) {
  switch( iParameter ) {
  case APP_LP_STYLE:
    dasher_main_save_state(pSelf);
    break;
  }
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
#ifndef WITH_MAEMO
  case LP_MAX_BITRATE:
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pPrivate->pSpeedBox), dasher_app_settings_get_long(pPrivate->pAppSettings, LP_MAX_BITRATE) / 100.0);
    break;
#endif
  case SP_ALPHABET_ID:
    dasher_main_populate_alphabet_combo(pSelf);
    break;
  case APP_LP_STYLE:
  case APP_BP_DOCK:
    // You can't generally switch window types once the X11 window has
    // been mapped, so when the app style is changed the main window
    // needs to be destroyed and recreated.
    {
      GtkWidget *pOldWindow = pPrivate->pMainWindow;

      pPrivate->pMainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_window_resize(GTK_WINDOW(pPrivate->pMainWindow), 200, 200);

      GtkWidget *pNewOuter;

      if(((dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 1) || (dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 2)) && dasher_app_settings_get_bool(g_pDasherAppSettings, APP_BP_DOCK)) {
	pNewOuter = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(pNewOuter), GTK_SHADOW_OUT);
	gtk_container_add(GTK_CONTAINER(pPrivate->pMainWindow), pNewOuter);
      }
      else {
	pNewOuter = pPrivate->pMainWindow;
      }

      window = pPrivate->pMainWindow;

      GSList *pAccelList = gtk_accel_groups_from_object(G_OBJECT(pOldWindow));

      while(pAccelList) {
	gtk_window_add_accel_group(GTK_WINDOW(pPrivate->pMainWindow), GTK_ACCEL_GROUP(pAccelList->data));
	pAccelList = pAccelList->next;
      }

      dasher_main_setup_window_type(pSelf);

      gtk_widget_hide(pOldWindow);
      gtk_widget_reparent(pPrivate->pInnerFrame, pNewOuter);
      gtk_object_destroy(GTK_OBJECT(pOldWindow));

      g_signal_connect(G_OBJECT(pPrivate->pMainWindow), "map", G_CALLBACK(on_window_map), NULL);
      g_signal_connect(G_OBJECT(pPrivate->pMainWindow), "delete_event", G_CALLBACK(ask_save_before_exit), NULL);

      gtk_widget_show_all(pPrivate->pMainWindow);
    }
    break;
  case BP_GLOBAL_KEYBOARD:
    dasher_main_setup_window(pSelf);
    break;
#ifdef WITH_MAEMO
  case APP_LP_MAEMO_SIZE: {
    bool bVisible = GTK_WIDGET_VISIBLE(pPrivate->pMainWindow);
    gtk_widget_hide(pPrivate->pMainWindow);
    if(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_MAEMO_SIZE) == 0) {
      int iWidth;
      gtk_window_get_size(GTK_WINDOW(pPrivate->pMainWindow), &iWidth, NULL);
      gtk_widget_set_size_request(pPrivate->pMainWindow, -1, 150);
      gtk_window_resize(GTK_WINDOW(pPrivate->pMainWindow), iWidth, 150);
      gtk_widget_set_size_request(pDasherWidget, 175, -1);
    }
    else {
      int iWidth;
      gtk_window_get_size(GTK_WINDOW(pPrivate->pMainWindow), &iWidth, NULL); 
      gtk_widget_set_size_request(pPrivate->pMainWindow, -1, 250);
      gtk_window_resize(GTK_WINDOW(pPrivate->pMainWindow), iWidth, 250);
      gtk_widget_set_size_request(pDasherWidget, 280, -1);
    }
    if(bVisible)
      gtk_widget_show(pPrivate->pMainWindow);
    break;
  }
#endif
  }

}

GladeXML *dasher_main_get_glade(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  return pPrivate->pGladeXML;
}

GtkWidget *dasher_main_get_window(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  return pPrivate->pMainWindow;
}

void dasher_main_set_app_settings(DasherMain *pSelf, DasherAppSettings *pAppSettings) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  pPrivate->pAppSettings = pAppSettings;

  dasher_main_setup_window(pSelf);

  // Now we have access to the settings, we can set up the intial
  // values
  
#ifndef WITH_MAEMO

  // TODO: put status bar initialisation somewhere else
  pPrivate->iComboCount = 0;
  dasher_main_populate_alphabet_combo(pSelf);

  gtk_spin_button_set_value(GTK_SPIN_BUTTON(pPrivate->pSpeedBox), dasher_app_settings_get_long(pPrivate->pAppSettings, LP_MAX_BITRATE) / 100.0);
  // TODO: bring into object framework
  PopulateMenus(pPrivate->pGladeXML);
  
  dasher_main_load_state(pSelf);
#endif

  dasher_main_refresh_font(pSelf);
}

void dasher_main_load_state(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  int iWindowWidth;
  int iWindowHeight;
  int iEditHeight;
  
  if(dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_STYLE) != 1) {
    iEditHeight = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_EDIT_HEIGHT);
    iWindowWidth = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_SCREEN_WIDTH);
    iWindowHeight = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_SCREEN_HEIGHT);
  }
  else {
    iEditHeight = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_EDIT_WIDTH);
    iWindowWidth = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_SCREEN_WIDTH_H);
    iWindowHeight = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_SCREEN_HEIGHT_H);
  }

  gtk_window_resize(GTK_WINDOW(pPrivate->pMainWindow), iWindowWidth, iWindowHeight);
  gtk_paned_set_position(GTK_PANED(pPrivate->pDivider), iEditHeight);

  pPrivate->iWidth = iWindowWidth;
  pPrivate->iHeight = iWindowHeight;
  
  pPrivate->iPosition = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_DOCK_POSITION);
}

void dasher_main_save_state(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

   int iWindowWidth;
   int iWindowHeight;
   int iEditHeight;

   gtk_window_get_size(GTK_WINDOW(pPrivate->pMainWindow), &iWindowWidth, &iWindowHeight);
   iEditHeight = gtk_paned_get_position(GTK_PANED(pPrivate->pDivider));

   if(dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_STYLE) != 1) {
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_EDIT_HEIGHT, iEditHeight);
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_WIDTH, iWindowWidth);
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_HEIGHT, iWindowHeight);
   }
   else {
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_EDIT_WIDTH, iEditHeight);
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_WIDTH_H, iWindowWidth);
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_HEIGHT_H, iWindowHeight);
   } 

   dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_DOCK_POSITION, pPrivate->iPosition);
}

void dasher_main_refresh_font(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  const gchar *szFontName = dasher_app_settings_get_string(pPrivate->pAppSettings, APP_SP_EDIT_FONT);
  PangoFontDescription *pFD = pango_font_description_from_string(szFontName);

  if(strcmp(szFontName, "")) {
    gtk_widget_modify_font(pPrivate->pBufferView, pFD);
  }
}

void dasher_main_show(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  gtk_widget_show(pPrivate->pMainWindow);
}

GtkWidget *dasher_main_create_dasher_control(DasherMain *pSelf) {

  g_message("Creating control");

  GtkWidget *pDasherControl = gtk_dasher_control_new();

#ifdef WITH_MAEMO
  gtk_widget_set_size_request(pDasherControl, 175, -1);  
#endif

  return pDasherControl;
}


// TODO: Rationalise window setup functions
void dasher_main_setup_window_position(DasherMain *pSelf) {

  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);


  Atom atom_strut_partial = gdk_x11_get_xatom_by_name("_NET_WM_STRUT_PARTIAL");
  guint32 struts[12] = {0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0 ,0 ,0};
  
  if(((dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 1) || (dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 2))  && dasher_app_settings_get_bool(g_pDasherAppSettings, APP_BP_DOCK)) {
    
//     GdkRectangle sFrameRect;
//     gdk_window_get_frame_extents(GDK_WINDOW(pPrivate->pMainWindow->window), &sFrameRect);
    
//     int iTargetWidth = sFrameRect.width;
//     int iTargetHeight = sFrameRect.height;

    int iTargetWidth = pPrivate->iWidth;
    int iTargetHeight = pPrivate->iHeight;
    
    int iScreenWidth = gdk_screen_width();
    int iScreenHeight = gdk_screen_height();
    int iScreenTop;
    int iLeft;
    int iTop;
    int iBuffer = 0;


 

//   XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
//   		  GDK_WINDOW_XWINDOW(window->window),
//   		  atom_strut_partial,
//   		  XA_CARDINAL, 32, PropModeReplace,
//   		  (guchar *)&struts, 12);

 
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

  

    if(pPrivate->bHidden)
      iLeft = iScreenWidth - 32; //pPrivate->pSideMenu->allocation.x;
    else
      iLeft = iScreenWidth - iTargetWidth - iBuffer;

    if(pPrivate->iPosition < iScreenTop)
      pPrivate->iPosition = iScreenTop;

    if(pPrivate->iPosition > iScreenTop + iScreenHeight - iTargetHeight)
      pPrivate->iPosition = iScreenTop + iScreenHeight - iTargetHeight;
 
    iTop = pPrivate->iPosition;

    struts[1] = 32;
    
    struts[6] = iTop;
    struts[7] = iTop + iTargetWidth;

    gtk_window_set_gravity(GTK_WINDOW(pPrivate->pMainWindow), GDK_GRAVITY_NORTH_WEST);

  
//   Atom atom_type[1];
//   atom_type[0] = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE_DOCK");
  
//   Atom atom_window_type = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE");
  
//   XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
// 		  GDK_WINDOW_XWINDOW(window->window),
// 		  atom_window_type,
// 		  XA_ATOM, 32, PropModeReplace,
// 		  (guchar *)&atom_type, 1);
  
  gdk_window_move((GdkWindow *)window->window, iLeft, iTop);

  }

  
    XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
		    GDK_WINDOW_XWINDOW(window->window),
		    atom_strut_partial,
		    XA_CARDINAL, 32, PropModeReplace,
		    (guchar *)&struts, 12);


}

// TODO: Don't pass topmost etc - store in object
void dasher_main_setup_window_style(DasherMain *pSelf, bool bTopMost) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  
  // Stup the global structure
  GtkWidget *pDividerNew;
  
  switch(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE)) {
  case 0: // Classic style
    pDividerNew = gtk_vpaned_new();
    gtk_widget_reparent(pPrivate->pEditPane, pDividerNew);
    gtk_widget_reparent(pDasherWidget, pDividerNew);
    break;
  case 1: // Composition
    pDividerNew = gtk_hpaned_new();
    gtk_widget_reparent(pDasherWidget, pDividerNew);
    gtk_widget_reparent(pPrivate->pEditPane, pDividerNew);
    break;
  case 2: // Direct
    pDividerNew = gtk_hpaned_new();
    gtk_widget_reparent(pDasherWidget, pDividerNew);
    gtk_widget_reparent(pPrivate->pEditPane, pDividerNew);
    break;
  case 3: // Full Screen
    pDividerNew = gtk_vpaned_new();
    gtk_widget_reparent(pPrivate->pEditPane, pDividerNew);
    gtk_widget_reparent(pDasherWidget, pDividerNew);
    break;
  default:
    g_error("Invalid style");
    break;
  }

  GtkWidget *pOldParent = gtk_widget_get_parent(pPrivate->pDivider);
  gtk_widget_destroy(pPrivate->pDivider);
  //  gtk_box_pack_start(GTK_BOX(pOldParent), pDividerNew, true, true, 0); 
  gtk_container_add(GTK_CONTAINER(pOldParent), pDividerNew);
  gtk_widget_show(pDividerNew);
  pPrivate->pDivider = pDividerNew;

  dasher_main_load_state(pSelf);

  // Visibility of components

  if(pPrivate->bShowActions) {
    gtk_widget_show(pPrivate->pActionPane);
  }
  else {
    gtk_widget_hide(pPrivate->pActionPane);
  }

  if(pPrivate->bShowEdit) {
    gtk_widget_show(pPrivate->pEditPane);
  }
  else {
    gtk_widget_hide(pPrivate->pEditPane);
  }

  if(pPrivate->bShowSideMenu) {
    gtk_widget_show(pPrivate->pSideMenu);
    gtk_widget_hide(pPrivate->pToolbar);
    gtk_widget_hide(pPrivate->pMenuBar);
    //    gtk_window_set_decorated(GTK_WINDOW(pPrivate->pMainWindow), false);
    // TODO: Need to figure out how to hide this sensibly - may involve reparenting and the like


//     gtk_widget_reparent(pPrivate->pInnerFrame, pPrivate->pOuterFrame);
    
//     gtk_container_add(GTK_CONTAINER(pPrivate->pMainWindow), pPrivate->pOuterFrame);
//     gtk_widget_reparent(pPrivate->pOuterFrame, pPrivate->pMainWindow);
  }
  else {
    gtk_widget_hide(pPrivate->pSideMenu);
    if(dasher_app_settings_get_bool(pPrivate->pAppSettings, APP_BP_SHOW_TOOLBAR))
      gtk_widget_show(pPrivate->pToolbar);
    else
      gtk_widget_hide(pPrivate->pToolbar);
    gtk_widget_show(pPrivate->pMenuBar);
    //    gtk_window_set_decorated(GTK_WINDOW(pPrivate->pMainWindow), true);
    //    gtk_frame_set_shadow_type(GTK_FRAME(pPrivate->pOuterFrame), GTK_SHADOW_NONE);

//     gtk_widget_ref(pPrivate->pOuterFrame);
//     gtk_container_remove(GTK_CONTAINER(pPrivate->pMainWindow), pPrivate->pOuterFrame);

//     gtk_widget_reparent(pPrivate->pInnerFrame, pPrivate->pMainWindow);
  }

  if(pPrivate->bFullScreen) {
    gtk_window_fullscreen(GTK_WINDOW(pPrivate->pMainWindow));
  }
  else {
    gtk_window_unfullscreen(GTK_WINDOW(pPrivate->pMainWindow));
  }

  gtk_window_set_keep_above(GTK_WINDOW(pPrivate->pMainWindow), pPrivate->bTopMost);

#ifdef WITH_MAEMO
  gtk_window_set_accept_focus(GTK_WINDOW(pPrivate->pMainWindow), false);
  gtk_window_set_focus_on_map(GTK_WINDOW(pPrivate->pMainWindow), false);
#else
  gtk_window_set_accept_focus(GTK_WINDOW(pPrivate->pMainWindow), !(pPrivate->bTopMost));
  gtk_window_set_focus_on_map(GTK_WINDOW(pPrivate->pMainWindow), !(pPrivate->bTopMost));
#endif
}


// TODO: Ugly hack - sort out calling order
// void dasher_main_on_map_real(DasherMain *pSelf) {
//   dasher_main_setup_window_type(pSelf);
//   dasher_main_on_map(pSelf);
// }

void dasher_main_on_map(DasherMain *pSelf) {

  g_message("In map routine");

  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  // Refresh the properties of the window

  switch(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE)) {
  case 0:
    pPrivate->bShowEdit = true;
    pPrivate->bShowActions = false;
    pPrivate->bTopMost = false;
    pPrivate->bFullScreen = false;
    pPrivate->bShowSideMenu = false;
    break;
  case 1:
    pPrivate->bShowEdit = true;
    pPrivate->bShowActions = true;
    pPrivate->bTopMost = true;
    pPrivate->bFullScreen = false;
    pPrivate->bShowSideMenu = true;
    break;
  case 2:
    pPrivate->bShowEdit = false;
    pPrivate->bShowActions = false;
    pPrivate->bTopMost = true;
    pPrivate->bFullScreen = false;
    pPrivate->bShowSideMenu = true;
    break;
  case 3:
    pPrivate->bShowEdit = true;
    pPrivate->bShowActions = false;
    pPrivate->bTopMost = false;
    pPrivate->bFullScreen = true;
    pPrivate->bShowSideMenu = false;
    break;

  }

#ifdef WITH_MAEMO
  
  Window xThisWindow = GDK_WINDOW_XWINDOW(pPrivate->pMainWindow->window);
  Atom atom_im_window = gdk_x11_get_xatom_by_name("_HILDON_IM_WINDOW");
  
  XChangeProperty(GDK_WINDOW_XDISPLAY(pPrivate->pMainWindow->window),
 		  GDK_WINDOW_XWINDOW(gdk_screen_get_root_window (gdk_screen_get_default ())),
 		  atom_im_window,
 		  XA_WINDOW, 32, PropModeReplace,
 		  (guchar *)&xThisWindow, 1);
  
  Atom atom_type[1];
  atom_type[0] = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE_INPUT");
  
  Atom atom_window_type = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE");
  
  XChangeProperty(GDK_WINDOW_XDISPLAY(pPrivate->pMainWindow->window),
		  GDK_WINDOW_XWINDOW(pPrivate->pMainWindow->window),
		  atom_window_type,
		  XA_ATOM, 32, PropModeReplace,
		  (guchar *)&atom_type, 1); 

  gtk_window_set_accept_focus(GTK_WINDOW(pPrivate->pMainWindow), false);
  gtk_window_set_focus_on_map(GTK_WINDOW(pPrivate->pMainWindow), false);

  if(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_MAEMO_SIZE) == 0) {
    gtk_widget_set_size_request(pPrivate->pMainWindow, -1, 150);
    gtk_widget_set_size_request(pDasherWidget, 175, -1);
  }
  else { 
    gtk_widget_set_size_request(pPrivate->pMainWindow, -1, 250);
    gtk_widget_set_size_request(pDasherWidget, 280, -1);
  }

  dasher_maemo_helper_setup_window(pPrivate->pMaemoHelper);
#else
  dasher_main_setup_window_style(pSelf, false);
  dasher_main_setup_window_position(pSelf);
#endif
}

static void dasher_main_setup_window_type(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  std::cout << "App style: " << dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) << std::endl;
  
  if(((dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 1) || (dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 2)) && dasher_app_settings_get_bool(g_pDasherAppSettings, APP_BP_DOCK)) {

    std::cout << "Mapped: " << GTK_WIDGET_MAPPED(pPrivate->pMainWindow) << std::endl;

    gtk_window_set_type_hint(GTK_WINDOW(pPrivate->pMainWindow), GDK_WINDOW_TYPE_HINT_DOCK);
  }
  else {
    gtk_window_set_type_hint(GTK_WINDOW(pPrivate->pMainWindow), GDK_WINDOW_TYPE_HINT_NORMAL);
  }
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

void dasher_main_toggle_hidden(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  pPrivate->bHidden = !(pPrivate->bHidden);

  dasher_main_on_map(pSelf);
}

void dasher_main_grab(DasherMain *pSelf, GdkEventButton *pEvent) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  pPrivate->bGrabbed = true;

  int iWindowX;
  int iWindowY;

  gtk_window_get_position(GTK_WINDOW(pPrivate->pMainWindow), &iWindowX, &iWindowY);

  pPrivate->dDragOffsetX = pEvent->x_root - iWindowX;
  pPrivate->dDragOffsetY = pEvent->y_root - iWindowY;

}

void dasher_main_ungrab(DasherMain *pSelf, GdkEventButton *pEvent) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  pPrivate->bGrabbed = false;
  dasher_main_save_state(pSelf);
}

gboolean dasher_main_motion(DasherMain *pSelf, GdkEventMotion *pEvent) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  if(pPrivate->bGrabbed) {
    double dNewX = pEvent->x_root - pPrivate->dDragOffsetX;
    double dNewY = pEvent->y_root - pPrivate->dDragOffsetY;

    pPrivate->iPosition = (int)(floor(dNewY));

    dasher_main_setup_window_position(pSelf);

    return true;
  }
  else {
    return false;
  }
}

gboolean dasher_main_topmost(DasherMain *pSelf) { 
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  return pPrivate->bTopMost;
}

gboolean dasher_main_speed_changed(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  
  int iNewValue( static_cast<int>(round(gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(pPrivate->pSpeedBox)) * 100)));
  
  if(dasher_app_settings_get_long(pPrivate->pAppSettings, LP_MAX_BITRATE) != iNewValue)
    dasher_app_settings_set_long(pPrivate->pAppSettings, LP_MAX_BITRATE, iNewValue);

  return true;
}

void dasher_main_populate_alphabet_combo(DasherMain *pSelf) {
#ifndef WITH_MAEMO
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  gtk_list_store_clear(pPrivate->pAlphabetList);

  GtkTreeIter sIter;
  const char *szValue;
  
  szValue = dasher_app_settings_get_string(pPrivate->pAppSettings, SP_ALPHABET_ID);
  if(strlen(szValue) > 0) {
    gtk_list_store_append(pPrivate->pAlphabetList, &sIter);
    gtk_list_store_set(pPrivate->pAlphabetList, &sIter, 0, szValue, -1);
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(pPrivate->pAlphabetCombo), &sIter);
  }
  
  szValue = dasher_app_settings_get_string(pPrivate->pAppSettings, SP_ALPHABET_1);
  if(strlen(szValue) > 0) {
    gtk_list_store_append(pPrivate->pAlphabetList, &sIter);
    gtk_list_store_set(pPrivate->pAlphabetList, &sIter, 0, szValue, -1);
  }
  
  szValue = dasher_app_settings_get_string(pPrivate->pAppSettings, SP_ALPHABET_2);
  if(strlen(szValue) > 0) {
    gtk_list_store_append(pPrivate->pAlphabetList, &sIter);
    gtk_list_store_set(pPrivate->pAlphabetList, &sIter, 0, szValue, -1);
  }
  
  szValue = dasher_app_settings_get_string(pPrivate->pAppSettings, SP_ALPHABET_3);
  if(strlen(szValue) > 0) {
    gtk_list_store_append(pPrivate->pAlphabetList, &sIter);
    gtk_list_store_set(pPrivate->pAlphabetList, &sIter, 0, szValue, -1);
  }
  
  szValue = dasher_app_settings_get_string(pPrivate->pAppSettings, SP_ALPHABET_4);
  if(strlen(szValue) > 0) {
    gtk_list_store_append(pPrivate->pAlphabetList, &sIter);
    gtk_list_store_set(pPrivate->pAlphabetList, &sIter, 0, szValue, -1);
  }
  
  gtk_list_store_append(pPrivate->pAlphabetList, &sIter);
  gtk_list_store_set(pPrivate->pAlphabetList, &sIter, 0, "More Alphabets...", -1);
#endif
}

gboolean dasher_main_alphabet_combo_changed(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  GtkTreeIter sIter;
  
  gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pPrivate->pAlphabetCombo), &sIter);

  const char *szSelected;
  gtk_tree_model_get(GTK_TREE_MODEL(pPrivate->pAlphabetList), &sIter, 0, &szSelected, -1);

  if(!strcmp("More Alphabets...", szSelected)) {
    gtk_combo_box_set_active(GTK_COMBO_BOX(pPrivate->pAlphabetCombo), 0);
    dasher_preferences_dialogue_show(g_pPreferencesDialogue);
  }
  else 
    dasher_app_settings_set_string(pPrivate->pAppSettings, SP_ALPHABET_ID, szSelected);
}

// Callbacks

extern "C" GtkWidget *create_dasher_control(gchar *szName, gchar *szString1, gchar *szString2, gint iInt1, gint iInt2) {
  return dasher_main_create_dasher_control(g_pDasherMain);
}

extern "C" void on_window_map(GtkWidget* pWidget, gpointer pUserData) {
  dasher_main_on_map(g_pDasherMain);
}

extern "C" void toggle_hidden(GtkWidget* pWidget, gpointer pUserData) {
  dasher_main_toggle_hidden(g_pDasherMain);
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

extern "C" gboolean sidemenu_press(GtkWidget *pWidget, GdkEventButton *pEvent, gpointer pData) {
  dasher_main_grab(g_pDasherMain, pEvent);
  return false;
}

extern "C" gboolean sidemenu_release(GtkWidget *pWidget, GdkEventButton *pEvent, gpointer pData) {
  dasher_main_ungrab(g_pDasherMain, pEvent);
  return false;
}

extern "C" gboolean sidemenu_motion(GtkWidget *pWidget, GdkEventMotion *pEvent, gpointer pData) {
  return dasher_main_motion(g_pDasherMain, pEvent);
}


extern "C" GdkFilterReturn keyboard_filter_cb(GdkXEvent *xevent, GdkEvent *event, gpointer data) {
  GtkDasherControl *pControl = (GtkDasherControl *)pDasherWidget;

  XEvent *xev = (XEvent *)xevent;

  if(xev->xany.type == KeyPress) {
    GdkKeymapKey sKeyDetails;

    sKeyDetails.keycode = ((XKeyPressedEvent *)xevent)->keycode;
    sKeyDetails.group = 0;
    sKeyDetails.level = 0;

    gtk_dasher_control_external_key_down(pControl, gdk_keymap_lookup_key(0, &sKeyDetails));

    return GDK_FILTER_REMOVE;
  }

  if(xev->xany.type == KeyRelease) { 
    GdkKeymapKey sKeyDetails;

    sKeyDetails.keycode = ((XKeyReleasedEvent *)xevent)->keycode;
    sKeyDetails.group = 0;
    sKeyDetails.level = 0;

    gtk_dasher_control_external_key_up(pControl, gdk_keymap_lookup_key(0, &sKeyDetails));

    return GDK_FILTER_REMOVE;
  }

  return GDK_FILTER_CONTINUE;
}

extern "C" gboolean speed_changed(GtkWidget *pWidget, gpointer user_data) {
  return dasher_main_speed_changed(g_pDasherMain);
}

extern "C" void alphabet_combo_changed(GtkWidget *pWidget, gpointer pUserData) {
  //  static_cast<CDasherControl*>(pUserData)->AlphabetComboChanged();
  dasher_main_alphabet_combo_changed(g_pDasherMain);
}
