#include "../../config.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#ifdef WITH_MAEMOFULLSCREEN
#include <hildon-widgets/hildon-program.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xatom.h>


#include "dasher.h"
#include "dasher_lock_dialogue.h"
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
  GtkWidget *pInnerFrame;
  GtkWidget *pSpeedBox;
  GtkWidget *pAlphabetCombo;
  GtkWidget *pStatusControl;
  GtkWidget *pHideArrow;

  GtkWidget *pContextMenu;

  GtkListStore *pAlphabetList;

  GtkAccelGroup *pAccel;

  //  int iComboCount;

#ifdef WITH_MAEMO
  DasherMaemoHelper *pMaemoHelper;
#ifdef WITH_MAEMOFULLSCREEN
  HildonProgram *pProgram;
  HildonWindow *pHWindow;
#endif
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

  gulong iAlphabetComboHandler;

  bool bWidgetsInitialised;
};

typedef struct _DasherMainPrivate DasherMainPrivate;

enum {
  REALIZED,
  SIGNAL_NUM
};

static guint dasher_main_signals[SIGNAL_NUM] = { 0 };

// Private member functions

static void dasher_main_class_init(DasherMainClass *pClass);
static void dasher_main_init(DasherMain *pMain);
static void dasher_main_destroy(GObject *pObject);
static void dasher_main_refresh_font(DasherMain *pSelf);
static GtkWidget *dasher_main_create_dasher_control(DasherMain *pSelf);
//static void dasher_main_setup_window_position(DasherMain *pSelf);
static void dasher_main_setup_window_style(DasherMain *pSelf);
static void dasher_main_setup_internal_layout(DasherMain *pSelf);
static void dasher_main_toggle_hidden(DasherMain *pSelf);
static void dasher_main_grab(DasherMain *pSelf, GdkEventButton *pEvent);
static void dasher_main_ungrab(DasherMain *pSelf, GdkEventButton *pEvent);
static gboolean dasher_main_motion(DasherMain *pSelf);
static gboolean dasher_main_speed_changed(DasherMain *pSelf);
static void dasher_main_populate_alphabet_combo(DasherMain *pSelf);
void dasher_main_build_context_menu(DasherMain *pSelf);

gboolean grab_focus();

// Private functions not in class
extern "C" gboolean take_real_focus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
extern "C" gboolean edit_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
extern "C" gboolean edit_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
extern "C" GdkFilterReturn keyboard_filter_cb(GdkXEvent *xevent, GdkEvent *event, gpointer data);
extern "C" void on_window_map(GtkWidget* pWidget, gpointer pUserData);
extern "C" gboolean cb_drag_timeout(gpointer pUserData);
extern "C" void alphabet_combo_changed(GtkWidget *pWidget, gpointer pUserData);

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

  dasher_main_signals[REALIZED] = g_signal_new("realized", G_TYPE_FROM_CLASS(pClass), static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), G_STRUCT_OFFSET(DasherMainClass, realized), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void dasher_main_init(DasherMain *pDasherControl) {
  pDasherControl->private_data = new DasherMainPrivate;
  
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pDasherControl->private_data);
  pPrivate->pAppSettings = 0;

  //  dasher_main_load_interface(pDasherControl);
}

static void dasher_main_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...

  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(DASHER_MAIN(pObject)->private_data);

  gtk_widget_destroy(pPrivate->pMainWindow);
}

// Public methods

DasherMain *dasher_main_new() {
  DasherMain *pDasherControl;
  pDasherControl = (DasherMain *)(g_object_new(dasher_main_get_type(), NULL));

  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pDasherControl->private_data);
  pPrivate->bWidgetsInitialised = false;

  return pDasherControl;
}

void dasher_main_load_interface(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  const char *szGladeFilename;

#ifdef WITH_GPE
  szGladeFilename = PROGDATA "/dashergpe.glade";
#elif WITH_MAEMO
#ifdef WITH_MAEMOFULLSCREEN
  //  szGladeFilename = "/var/lib/install" PROGDATA "/dashermaemofullscreen.glade";
  szGladeFilename = PROGDATA "/dashermaemofullscreen.glade";
#else
  //szGladeFilename = "/var/lib/install" PROGDATA "/dashermaemo.glade";
  szGladeFilename = PROGDATA "/dashermaemo.glade";
#endif
#else
  switch(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE)) {
  case 0:
    szGladeFilename = PROGDATA "/dasher.traditional.glade";
    break;
  case 1:
    szGladeFilename = PROGDATA "/dasher.compose.glade";
    break;
  case 2:
    szGladeFilename = PROGDATA "/dasher.direct.glade";
    break;
  case 3:
    szGladeFilename = PROGDATA "/dasher.fullscreen.glade";
    break;
  default:
    g_error("Inconsistent application style specified.");
  }
#endif

  pPrivate->pGladeXML = glade_xml_new(szGladeFilename, NULL, NULL);

  if (!pPrivate->pGladeXML) {
    g_error("Can't find Glade file: %s. Dasher is unlikely to be correctly installed.", szGladeFilename);
  }

  glade_xml_signal_autoconnect(pPrivate->pGladeXML);

  // Save the details of some of the widgets for later
  pPrivate->pActionPane = glade_xml_get_widget(pPrivate->pGladeXML, "vbox39");
  pPrivate->pBufferView = glade_xml_get_widget(pPrivate->pGladeXML, "the_text_view");
  pPrivate->pDivider = glade_xml_get_widget(pPrivate->pGladeXML, "main_divider");
  pPrivate->pEditPane = glade_xml_get_widget(pPrivate->pGladeXML, "vbox40");
  pPrivate->pMainWindow = glade_xml_get_widget(pPrivate->pGladeXML, "window");
  pPrivate->pToolbar = glade_xml_get_widget(pPrivate->pGladeXML, "toolbar");
  pPrivate->pMenuBar = glade_xml_get_widget(pPrivate->pGladeXML, "dasher_menu_bar");
//   pPrivate->pSideMenu = glade_xml_get_widget(pPrivate->pGladeXML, "SideMenu");
//   pPrivate->pDragHandle = glade_xml_get_widget(pPrivate->pGladeXML, "button31");
//   pPrivate->pHideArrow = glade_xml_get_widget(pPrivate->pGladeXML, "hide_arrow");

//   gtk_widget_add_events(pPrivate->pDragHandle, 
// 			GDK_BUTTON_PRESS_MASK | 
// 			GDK_BUTTON_RELEASE_MASK |
// 			GDK_POINTER_MOTION_MASK);

  // TODO: This could be made more sensible with consistent naming
// #ifdef WITH_MAEMO
// #ifdef WITH_MAEMOFULLSCREEN
//   pPrivate->pInnerFrame = glade_xml_get_widget(pPrivate->pGladeXML, "hpaned1"); 
// #else
//   pPrivate->pInnerFrame = glade_xml_get_widget(pPrivate->pGladeXML, "hbox1"); 
// #endif
// #else
//   pPrivate->pInnerFrame = glade_xml_get_widget(pPrivate->pGladeXML, "vbox1"); 
// #endif

#ifndef WITH_MAEMO
  pPrivate->pSpeedBox = glade_xml_get_widget(pPrivate->pGladeXML, "spinbutton1");
  pPrivate->pAlphabetCombo = glade_xml_get_widget(pPrivate->pGladeXML, "combobox1");
  pPrivate->pStatusControl = glade_xml_get_widget(pPrivate->pGladeXML, "hbox8"); 

  pPrivate->pAlphabetList = gtk_list_store_new(1, G_TYPE_STRING);
  gtk_combo_box_set_model(GTK_COMBO_BOX(pPrivate->pAlphabetCombo), 
			  GTK_TREE_MODEL(pPrivate->pAlphabetList));

  GtkCellRenderer *pRenderer;
  pRenderer = gtk_cell_renderer_text_new();
#if GTK_CHECK_VERSION(2,6,0)
  g_object_set(G_OBJECT(pRenderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
#endif
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(pPrivate->pAlphabetCombo), pRenderer, true);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(pPrivate->pAlphabetCombo), pRenderer, "text", 0, NULL);


  // gtk_widget_add_events(pPrivate->pDragHandle, GDK_POINTER_MOTION_MASK);
#else

#ifdef WITH_MAEMOFULLSCREEN
  // TODO: This is horrible - no need to get it from the glade file if we're not going to use it

  pPrivate->pProgram = HILDON_PROGRAM(hildon_program_get_instance());
  //  hildon_app_set_title(pPrivate->pApp, "Dasher"); 

  pPrivate->pHWindow = HILDON_WINDOW(hildon_window_new());
  hildon_program_add_window(pPrivate->pProgram, pPrivate->pHWindow);

  gtk_widget_reparent(pPrivate->pInnerFrame, GTK_WIDGET(pPrivate->pHWindow));
  //  gtk_paned_set_position(GTK_PANED(window), 100);

  /* Do menu setup */
  GtkMenu *main_menu;
  GtkWidget *file_menu;
  GtkWidget *file_menu_item;
  GtkWidget *options_menu;
  GtkWidget *options_menu_item;
  GtkWidget *help_menu;
  GtkWidget *help_menu_item;


//   main_menu = hildon_appview_get_menu(appview);

  main_menu = GTK_MENU(gtk_menu_new());
  file_menu = glade_xml_get_widget(pPrivate->pGladeXML, "menuitem4_menu");
  options_menu = glade_xml_get_widget(pPrivate->pGladeXML, "options1_menu");
  help_menu = glade_xml_get_widget(pPrivate->pGladeXML, "menuitem7_menu");
  file_menu_item = gtk_menu_item_new_with_label ("File");
  options_menu_item = gtk_menu_item_new_with_label ("Options");
  help_menu_item = gtk_menu_item_new_with_label ("Help");

  g_object_ref(file_menu);
  g_object_ref(options_menu);
  g_object_ref(help_menu);

  gtk_menu_item_remove_submenu(GTK_MENU_ITEM(glade_xml_get_widget(pPrivate->pGladeXML, "menuitem4")));
  gtk_menu_item_remove_submenu(GTK_MENU_ITEM(glade_xml_get_widget(pPrivate->pGladeXML, "options1")));
  gtk_menu_item_remove_submenu(GTK_MENU_ITEM(glade_xml_get_widget(pPrivate->pGladeXML, "menuitem7")));

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item),file_menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(options_menu_item),options_menu); 
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item),help_menu);
  gtk_menu_append(main_menu, file_menu_item);
  gtk_menu_append(main_menu, options_menu_item);
  gtk_menu_append(main_menu, help_menu_item);

  g_object_unref(file_menu);
  g_object_unref(options_menu);
  g_object_unref(help_menu);

  hildon_program_set_common_menu(pPrivate->pProgram, main_menu); 

  gtk_widget_show_all( GTK_WIDGET( main_menu ) );

//   /* And toolbar */
//   GtkWidget *toolbar;
//   toolbar = glade_xml_get_widget(pPrivate->pGladeXML, "toolbar");
//   g_print("Got %p\n",toolbar);
//   gtk_widget_reparent (toolbar, appview->vbox);

  gtk_widget_show_all(GTK_WIDGET(pPrivate->pHWindow));

  gtk_widget_destroy(pPrivate->pMainWindow);
  pPrivate->pMainWindow = GTK_WIDGET(pPrivate->pHWindow);

  g_signal_connect(G_OBJECT(pPrivate->pHWindow), "delete_event", G_CALLBACK(ask_save_before_exit), NULL);

#endif // Maemo fullscreen
#endif // Maemo 

  pPrivate->bHidden = false;
  pPrivate->bGrabbed = false;

  pPrivate->iPosition = 100; // FIXME - make this persistant

  // TODO: Specify callbacks in glade file
  // TODO: Rationalise focus
  g_signal_connect(G_OBJECT(pPrivate->pBufferView), "button-release-event", G_CALLBACK(take_real_focus), NULL);
  g_signal_connect(G_OBJECT(pPrivate->pBufferView), "key-press-event", G_CALLBACK(edit_key_press), NULL);
  g_signal_connect(G_OBJECT(pPrivate->pBufferView), "key-release-event", G_CALLBACK(edit_key_release), NULL);

  pPrivate->iAlphabetComboHandler = g_signal_connect(G_OBJECT(pPrivate->pAlphabetCombo), "changed", G_CALLBACK(alphabet_combo_changed), NULL);
  
  dasher_main_build_context_menu(pSelf);

  // Create a Maemo helper if necessary
#if defined WITH_MAEMO && !defined WITH_MAEMOFULLSCREEN
  pPrivate->pMaemoHelper = dasher_maemo_helper_new(GTK_WINDOW(pPrivate->pMainWindow));
#endif

  // Set up any non-registry-dependent options
#ifdef WITH_GPE
  gtk_window_set_decorated(GTK_WINDOW(pPrivate->pMainWindow), false);
#endif
  pDasherWidget = glade_xml_get_widget(pPrivate->pGladeXML, "DasherControl");

  pPrivate->bWidgetsInitialised = true;
}

// Functions to do with configuring the main window

void dasher_main_setup_window_style(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  switch(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE)) {
  case 0:
    // Nothing to do
    break;
  case 1:
    // Nothing to do
    break;
  case 2:
    // Direct mode - set always on top
    gtk_window_set_keep_above(GTK_WINDOW(pPrivate->pMainWindow), true);

    // Refuse focus
    gtk_window_set_accept_focus(GTK_WINDOW(pPrivate->pMainWindow), false);
    break;
  case 3:
    // Fullscreen mode - set fullscreen
    gtk_window_fullscreen(GTK_WINDOW(pPrivate->pMainWindow));
    break;
  default:
    g_error("Inconsistent application style specified.");
  }
}

void dasher_main_setup_window_state(DasherMain *pSelf) {
  dasher_main_load_state(pSelf);
  
  // TODO: Setup positioning here - need to think up a policy on this
}

void dasher_main_setup_internal_layout(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
 
  if(pPrivate->pToolbar) {
    if( dasher_app_settings_get_bool(pPrivate->pAppSettings, APP_BP_SHOW_TOOLBAR))
      gtk_widget_show(pPrivate->pToolbar);
    else
      gtk_widget_hide(pPrivate->pToolbar);
  }    

  if(pPrivate->pStatusControl) {
    if( dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_SHOW_SLIDER))
      gtk_widget_show(pPrivate->pStatusControl);
    else
      gtk_widget_hide(pPrivate->pStatusControl);
  }

  dasher_main_refresh_font(pSelf);
}

void dasher_main_setup_window(DasherMain *pSelf) {
  dasher_main_setup_window_style(pSelf);
  dasher_main_setup_window_state(pSelf);
  dasher_main_setup_internal_layout(pSelf);

//   DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  
//   if(dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_GLOBAL_KEYBOARD))
//     gdk_window_add_filter(0, keyboard_filter_cb, 0);
//   else
//     gdk_window_remove_filter(0, keyboard_filter_cb, 0);
}

void dasher_main_populate_controls(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  // Populate the alphabet chooser
  dasher_main_populate_alphabet_combo(pSelf);
  
  // Set the value of the speed spinner
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(pPrivate->pSpeedBox), 
			    dasher_app_settings_get_long(pPrivate->pAppSettings, LP_MAX_BITRATE) / 100.0);
}

// Parameter change handling

void dasher_main_handle_pre_parameter_change(DasherMain *pSelf, int iParameter) {
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
  case BP_SHOW_SLIDER: // TODO: Shouldn't be a core parmeter
    if( dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_SHOW_SLIDER))
      gtk_widget_show(pPrivate->pStatusControl);
    else
      gtk_widget_hide(pPrivate->pStatusControl);
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
  case BP_GLOBAL_KEYBOARD:
    dasher_main_setup_window(pSelf);
    break;
#if defined WITH_MAEMO && !defined WITH_MAEMOFULLSCREEN
  case APP_LP_MAEMO_SIZE: {
    g_message("Maemo size");

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

#ifndef WITH_MAEMO
  gtk_window_resize(GTK_WINDOW(pPrivate->pMainWindow), iWindowWidth, iWindowHeight);
#endif

  gtk_paned_set_position(GTK_PANED(pPrivate->pDivider), iEditHeight);

  pPrivate->iWidth = iWindowWidth;
  pPrivate->iHeight = iWindowHeight;
  
  pPrivate->iPosition = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_DOCK_POSITION);
}

void dasher_main_save_state(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  if(!pPrivate->bWidgetsInitialised)
    return;
  
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
  GtkWidget *pDasherControl = gtk_dasher_control_new();

#ifdef WITH_MAEMO
  gtk_widget_set_size_request(pDasherControl, 175, -1);  
#endif

  return pDasherControl;
}


// TODO: Rationalise window setup functions
// void dasher_main_setup_window_position(DasherMain *pSelf) {

//   DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

//   Atom atom_strut_partial = gdk_x11_get_xatom_by_name("_NET_WM_STRUT_PARTIAL");
//   guint32 struts[12] = {0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0 ,0 ,0};
  
//   if(((dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 1) || (dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 2))  && dasher_app_settings_get_bool(g_pDasherAppSettings, APP_BP_DOCK)) {
    
// //     GdkRectangle sFrameRect;
// //     gdk_window_get_frame_extents(GDK_WINDOW(pPrivate->pMainWindow->window), &sFrameRect);
    
// //     int iTargetWidth = sFrameRect.width;
// //     int iTargetHeight = sFrameRect.height;

//     int iTargetWidth = pPrivate->iWidth;
//     int iTargetHeight = pPrivate->iHeight;
    
//     int iScreenWidth = gdk_screen_width();
//     int iScreenHeight = gdk_screen_height();
//     int iScreenTop;
//     int iLeft;
//     int iTop;
//     int iBuffer = 0;


 

// //   XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
// //   		  GDK_WINDOW_XWINDOW(window->window),
// //   		  atom_strut_partial,
// //   		  XA_CARDINAL, 32, PropModeReplace,
// //   		  (guchar *)&struts, 12);

 
//   Atom atom_work_area = gdk_x11_get_xatom_by_name("_NET_WORKAREA");
//   Atom aReturn;
//   int iFormatReturn;
//   unsigned long iItemsReturn;
//   unsigned long iBytesAfterReturn;
//   unsigned char *iData;

//   XGetWindowProperty(GDK_WINDOW_XDISPLAY(GDK_ROOT_PARENT()),
// 		     GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()),
// 		     atom_work_area,
// 		     0, 4, false,
// 		     XA_CARDINAL, 
// 		     &aReturn,
// 		     &iFormatReturn,
// 		     &iItemsReturn,
// 		     &iBytesAfterReturn,
// 		     &iData);

//   // TODO: need more error checking with raw X11 stuff
  
//   iScreenTop = ((unsigned long *)iData)[1];
//   iScreenHeight = ((unsigned long *)iData)[3];

//   XFree(iData);

  
//   if(pPrivate->bHidden)
//     iLeft = iScreenWidth - pPrivate->pSideMenu->allocation.width;
//   else
//     iLeft = iScreenWidth - iTargetWidth - iBuffer;
  
//   if(pPrivate->iPosition < iScreenTop)
//     pPrivate->iPosition = iScreenTop;
  
//   if(pPrivate->iPosition > iScreenTop + iScreenHeight - iTargetHeight)
//     pPrivate->iPosition = iScreenTop + iScreenHeight - iTargetHeight;
  
//   iTop = pPrivate->iPosition;
  
//   struts[1] = pPrivate->pSideMenu->allocation.width;
  
//   struts[6] = iTop;
//   struts[7] = iTop + iTargetWidth;
  
//   gtk_window_set_gravity(GTK_WINDOW(pPrivate->pMainWindow), GDK_GRAVITY_NORTH_WEST);

  
// //   Atom atom_type[1];
// //   atom_type[0] = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE_DOCK");
  
// //   Atom atom_window_type = gdk_x11_get_xatom_by_name("_NET_WM_WINDOW_TYPE");
  
// //   XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
// // 		  GDK_WINDOW_XWINDOW(window->window),
// // 		  atom_window_type,
// // 		  XA_ATOM, 32, PropModeReplace,
// // 		  (guchar *)&atom_type, 1);
  
//   gdk_window_move((GdkWindow *)window->window, iLeft, iTop);
//   }
 
//   XChangeProperty(GDK_WINDOW_XDISPLAY(window->window),
// 		  GDK_WINDOW_XWINDOW(window->window),
// 		  atom_strut_partial,
// 		  XA_CARDINAL, 32, PropModeReplace,
// 		  (guchar *)&struts, 12);
// }



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

  if(pPrivate->bHidden)
    gtk_arrow_set(GTK_ARROW(pPrivate->pHideArrow), GTK_ARROW_LEFT, GTK_SHADOW_NONE);
  else
    gtk_arrow_set(GTK_ARROW(pPrivate->pHideArrow), GTK_ARROW_RIGHT, GTK_SHADOW_NONE);

  //  dasher_main_on_map(pSelf);
}

void dasher_main_grab(DasherMain *pSelf, GdkEventButton *pEvent) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  if(pEvent->button == 3) {
    // Right click -> context menu

    gtk_menu_popup(GTK_MENU(pPrivate->pContextMenu), NULL,
		   NULL, NULL, NULL,
		   pEvent->button, pEvent->time);
  }
  else {
    // Left / middle click -> drag dock
    pPrivate->bGrabbed = true;
    
    int iWindowX;
    int iWindowY;
    
    gtk_window_get_position(GTK_WINDOW(pPrivate->pMainWindow), &iWindowX, &iWindowY);
    
    pPrivate->dDragOffsetX = pEvent->x_root - iWindowX;
    pPrivate->dDragOffsetY = pEvent->y_root - iWindowY;

    // Note that we get a grab automatically from X, as we're
    // configured to receive press and release events. No need to
    // explicitly add a grab then (and in fact this doesn't work as a
    // result).
    
    // GdkCursor *pCursor = gdk_cursor_new(GDK_FLEUR);

    // g_message("window %d", pPrivate->pDragHandle->window);

//      int iValue = gdk_pointer_grab(NULL, //pPrivate->pMainWindow->window,
//  				  TRUE,
//  				  GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK,
//  				  NULL,
//  				  pCursor,
//  				  pEvent->time);
     
//     g_message("grab value %d", iValue==GDK_GRAB_SUCCESS);
    
//    gdk_cursor_unref(pCursor);

    g_timeout_add(25, cb_drag_timeout, pSelf);
  }
}

void dasher_main_ungrab(DasherMain *pSelf, GdkEventButton *pEvent) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  pPrivate->bGrabbed = false;
  dasher_main_save_state(pSelf);

  gdk_pointer_ungrab(pEvent->time);
}

gboolean dasher_main_motion(DasherMain *pSelf) {
   DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

   if(!(pPrivate->bGrabbed))
     return FALSE;

   int iX;
   int iY;

   gdk_display_get_pointer(gdk_display_get_default(), NULL, &iX, &iY, NULL);

   //   double dNewX = iX - pPrivate->dDragOffsetX;
   double dNewY = iY - pPrivate->dDragOffsetY;

   pPrivate->iPosition = (int)(floor(dNewY));

   //   dasher_main_setup_window_position(pSelf);

  return TRUE;
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

  // Disconnect the event handler temporarily, otherwise this will
  // trigger alphabet changes

  g_signal_handler_block(pPrivate->pAlphabetCombo, pPrivate->iAlphabetComboHandler);

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

  g_signal_handler_unblock(pPrivate->pAlphabetCombo, pPrivate->iAlphabetComboHandler);

#endif
}

void dasher_main_alphabet_combo_changed(DasherMain *pSelf) {
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

void dasher_main_build_context_menu(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);

  pPrivate->pContextMenu = gtk_menu_new();

  GtkWidget *pNewItem;

  pNewItem = gtk_menu_item_new_with_label("Prefences...");
  gtk_widget_show(pNewItem);
  gtk_menu_shell_append(GTK_MENU_SHELL(pPrivate->pContextMenu), pNewItem);
  g_signal_connect(G_OBJECT(pNewItem), "activate", G_CALLBACK(preferences_display), NULL);

  pNewItem = gtk_menu_item_new_with_label("About...");
  gtk_widget_show(pNewItem);
  gtk_menu_shell_append(GTK_MENU_SHELL(pPrivate->pContextMenu), pNewItem);
  g_signal_connect(G_OBJECT(pNewItem), "activate", G_CALLBACK(about_dasher), NULL);


  pNewItem = gtk_separator_menu_item_new();
  gtk_widget_show(pNewItem);
  gtk_menu_shell_append(GTK_MENU_SHELL(pPrivate->pContextMenu), pNewItem);

  pNewItem = gtk_menu_item_new_with_label("Quit");
  gtk_widget_show(pNewItem);
  gtk_menu_shell_append(GTK_MENU_SHELL(pPrivate->pContextMenu), pNewItem);  
  g_signal_connect(G_OBJECT(pNewItem), "activate", G_CALLBACK(ask_save_before_exit), NULL);
}

// Callbacks

extern "C" GtkWidget *create_dasher_control(gchar *szName, gchar *szString1, gchar *szString2, gint iInt1, gint iInt2) {
  return dasher_main_create_dasher_control(g_pDasherMain);
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

extern "C" bool edit_focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
  return true;
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

// extern "C" gboolean sidemenu_press(GtkWidget *pWidget, GdkEventButton *pEvent, gpointer pData) {
//   dasher_main_grab(g_pDasherMain, pEvent);
//   return false;
// }

// extern "C" gboolean sidemenu_release(GtkWidget *pWidget, GdkEventButton *pEvent, gpointer pData) {
//   dasher_main_ungrab(g_pDasherMain, pEvent);
//   return false;
// }

// extern "C" gboolean sidemenu_motion(GtkWidget *pWidget, GdkEventMotion *pEvent, gpointer pData) {
//   //  return dasher_main_motion(g_pDasherMain, pEvent);
//   return false;
// }


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
  dasher_main_alphabet_combo_changed(g_pDasherMain);
}
 
extern "C" gboolean cb_drag_timeout(gpointer pUserData) {
  return dasher_main_motion(DASHER_MAIN(pUserData));
}
