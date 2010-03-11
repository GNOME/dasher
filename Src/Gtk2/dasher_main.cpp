#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#ifdef WITH_MAEMOFULLSCREEN
#include <hildon-widgets/hildon-program.h>
#endif
#include <unistd.h>

#include "GtkDasherControl.h"
#include "KeyboardHelper.h"
#include "Preferences.h"
#include "dasher_lock_dialogue.h"
#ifdef WITH_MAEMO
#include "dasher_maemo_helper.h"
#endif
#include "dasher_main.h"

#include "DasherAppSettings.h"
#include "dasher_editor_internal.h"
#include "dasher_editor_external.h"

/* Static instance of singleton, USE SPARINGLY */
static DasherMain *g_pDasherMain = NULL; 

// TODO: The following global variable makes control mode editing work
// - this needs to be sorted out properly.
static gboolean g_bSend = true;

struct _DasherMainPrivate {
  GtkBuilder *pXML;
  GtkBuilder *pPrefXML;

  // Child objects owned here
  DasherAppSettings *pAppSettings;
  DasherPreferencesDialogue *pPreferencesDialogue;
  DasherEditor *pEditor;

  CKeyboardHelper *pKeyboardHelper;

  // Various widgets which need to be cached:
  // GtkWidget *pBufferView;
  GtkPaned  *pDivider;
  GtkWindow *pMainWindow;
  GtkWidget *pToolbar;
  GtkSpinButton *pSpeedBox;
  GtkWidget *pAlphabetCombo;
  GtkWidget *pStatusControl;
  GtkWidget *pDasherWidget;

  GtkListStore *pAlphabetList;
  GtkAccelGroup *pAccel;
  gulong iAlphabetComboHandler;

  // Widgets used for maemo
#ifdef WITH_MAEMO
  DasherMaemoHelper *pMaemoHelper;
#ifdef WITH_MAEMOFULLSCREEN
  HildonProgram *pProgram;
  HildonWindow *pHWindow;
#endif
#endif

  // Properties of the main window
  int iWidth;
  int iHeight;
  bool bWidgetsInitialised;
};

typedef struct _DasherMainPrivate DasherMainPrivate;

// TODO: Make sure this is actually used
#define DASHER_MAIN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_MAIN, DasherMainPrivate))

enum {
  REALIZED,
  SIGNAL_NUM
};

static guint dasher_main_signals[SIGNAL_NUM] = { 0 };

G_DEFINE_TYPE(DasherMain, dasher_main, G_TYPE_OBJECT);

static void dasher_main_finalize(GObject *pObject);

/* Private member functions */
static void dasher_main_setup_window_state(DasherMain *pSelf);
static void dasher_main_setup_window_style(DasherMain *pSelf);
static void dasher_main_setup_internal_layout(DasherMain *pSelf);
static void dasher_main_set_window_title(DasherMain *pSelf);

/* ... Table based menu/toolbar commands */
static void dasher_main_command_import(DasherMain *pSelf);
static void dasher_main_command_quit(DasherMain *pSelf);
static void dasher_main_command_preferences(DasherMain *pSelf);
static void dasher_main_command_preferences_alphabet(DasherMain *pSelf);
static void dasher_main_command_tutorial(DasherMain *pSelf);
static void dasher_main_command_help(DasherMain *pSelf);
static void dasher_main_command_about(DasherMain *pSelf);

/* c.f. WRAP_CPP_CB below */
extern "C" void dasher_main_cb_import(GtkAction*, DasherMain*);
extern "C" void dasher_main_cb_quit(GtkAction*, DasherMain*);
extern "C" void dasher_main_cb_preferences(GtkAction*, DasherMain*);
extern "C" void dasher_main_cb_help(GtkAction*, DasherMain*);
extern "C" void dasher_main_cb_about(GtkAction*, DasherMain*);
extern "C" void dasher_main_cb_editor(GtkAction*, DasherMain*);

static gboolean dasher_main_speed_changed(DasherMain *pSelf);
static void dasher_main_alphabet_combo_changed(DasherMain *pSelf);
// TODO: populate speed slider
static void dasher_main_populate_alphabet_combo(DasherMain *pSelf);

/* TODO: order these in file */
static GtkBuilder *dasher_main_open_gui_xml(DasherMain *, const char *);
static void dasher_main_load_interface(DasherMain *pSelf);
static void dasher_main_create_preferences(DasherMain *pSelf);
static void dasher_main_handle_parameter_change(DasherMain *pSelf, int iParameter);
static void dasher_main_load_state(DasherMain *pSelf);
static void dasher_main_save_state(DasherMain *pSelf);
static void dasher_main_setup_window(DasherMain *pSelf);
static void dasher_main_populate_controls(DasherMain *pSelf);
static void dasher_main_connect_control(DasherMain *pSelf);
static gboolean dasher_main_command(DasherMain *pSelf, const gchar *szCommand);
static gint dasher_main_lookup_key(DasherMain *pSelf, guint iKeyVal);

/* TODO: Various functions which haven't yet been rationalised */
gboolean grab_focus();

/* ... Message handling from main window widgets */
extern "C" void speed_changed(GtkWidget *pWidget, gpointer user_data);
extern "C" void alphabet_combo_changed(GtkWidget *pWidget, gpointer pUserData);
extern "C" void dasher_main_cb_filename_changed(DasherEditor *pEditor, gpointer pUserData);
extern "C" void dasher_main_cb_buffer_changed(DasherEditor *pEditor, gpointer pUserData);
extern "C" void dasher_main_cb_context_changed(DasherEditor *pEditor, gpointer pUserData);
extern "C" gboolean dasher_main_cb_window_close(GtkWidget *pWidget, gpointer pUserData);
extern "C" void parameter_notification(GtkDasherControl *pDasherControl, gint iParameter, gpointer data);

/* ... Focus management and event forwarding */
extern "C" bool focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data);
extern "C" bool edit_focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data);
extern "C" gboolean take_real_focus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);

extern "C" gboolean edit_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
extern "C" gboolean edit_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

/* ... Temporary test/debug functions */
extern "C" gboolean test_focus_handler(GtkWidget *pWidget, GtkDirectionType iDirection, gpointer *pUserData);

extern "C" void handle_context_request(GtkDasherControl * pDasherControl, gint iOffset, gint iLength, gpointer data);
extern "C" void handle_control_event(GtkDasherControl *pDasherControl, gint iEvent, gpointer data);
extern "C" void handle_start_event(GtkDasherControl *pDasherControl, gpointer data);
extern "C" gint dasher_main_key_snooper(GtkWidget *pWidget, GdkEventKey *pEvent, gpointer pUserData);

/* Boilerplate code */
static void 
dasher_main_class_init(DasherMainClass *pClass) {
  g_type_class_add_private(pClass, sizeof(DasherMainPrivate));

  dasher_main_signals[REALIZED] =
      g_signal_new("realized", G_TYPE_FROM_CLASS(pClass), 
                   (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), 
                   G_STRUCT_OFFSET(DasherMainClass, realized), 
                   NULL, NULL, g_cclosure_marshal_VOID__VOID, 
                   G_TYPE_NONE, 0);

  GObjectClass *pObjectClass = (GObjectClass *)pClass;
  pObjectClass->finalize = dasher_main_finalize;
}

static void 
dasher_main_init(DasherMain *pDasherMain) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pDasherMain);
  
  pPrivate->pAppSettings = NULL;
  pPrivate->pEditor = NULL;
  pPrivate->pPreferencesDialogue = NULL;

  pPrivate->pKeyboardHelper = new CKeyboardHelper(NULL);

  pPrivate->bWidgetsInitialised = false;
}

static void
dasher_main_finalize(GObject *pObject) {

  DasherMain *pDasherMain = DASHER_MAIN(pObject);
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pDasherMain);

  dasher_main_save_state(pDasherMain);

  /* TODO: Does unref really do the right thing - check the whole ref counting situation */
  //  if(pPrivate->pEditor)
  //  g_object_unref(pPrivate->pEditor);

  if(pPrivate->pPreferencesDialogue)
    g_object_unref(pPrivate->pPreferencesDialogue);

  if(pPrivate->pAppSettings)
    g_object_unref(pPrivate->pAppSettings);

  gtk_widget_destroy(GTK_WIDGET(pPrivate->pMainWindow));

  /* TODO: Do we need to take down anything else? */
}

/* Public methods */
DasherMain *
dasher_main_new(int *argc, char ***argv, SCommandLine *pCommandLine) {
  if(g_pDasherMain)
    return g_pDasherMain;
  else {
    DasherMain *pDasherMain = (DasherMain *)(g_object_new(dasher_main_get_type(), NULL));
    g_pDasherMain = pDasherMain;

    DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pDasherMain);

    /* Create the app settings object */
    pPrivate->pAppSettings = dasher_app_settings_new(*argc, *argv);
    
    /* Load the user interface from the GUI file */
    if(pCommandLine && pCommandLine->szAppStyle) {
      if(!strcmp(pCommandLine->szAppStyle, "traditional")) {
        dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_STYLE, APP_STYLE_TRAD);
      }
      else if(!strcmp(pCommandLine->szAppStyle, "compose")) {
        dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_STYLE, APP_STYLE_COMPOSE);
      }
      else if(!strcmp(pCommandLine->szAppStyle, "direct")) {
        dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_STYLE, APP_STYLE_DIRECT);
      }
      else if(!strcmp(pCommandLine->szAppStyle, "fullscreen")) {
        dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_STYLE, APP_STYLE_FULLSCREEN);
      }
      else {
        g_critical("Application style %s is not supported", pCommandLine->szAppStyle);
        return 0;
      }
    }
    else { 
      // By default use traditional mode
      dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_STYLE, APP_STYLE_TRAD);
    }

    dasher_main_load_interface(pDasherMain);

    dasher_app_settings_set_widget(pPrivate->pAppSettings, GTK_DASHER_CONTROL(pPrivate->pDasherWidget));


    /* TODO: This parsing code should really be tidied up */
    if(pCommandLine && pCommandLine->szOptions) {
      gchar **pszOptionTerms;
      pszOptionTerms = g_strsplit(pCommandLine->szOptions, ",", 0);

      gchar **pszCurrent = pszOptionTerms;

      while(*pszCurrent) {
        gchar *szJoin = g_strrstr(*pszCurrent, "=");
        // Note to translators: This message will be output for command line errors when the "=" in --options=foo is missing.
        const gchar *errorMessage = _("option setting is missing \"=\".");

        if(szJoin) {
          int iLength = szJoin - *pszCurrent;
          
          gchar *szKey = g_new(gchar, iLength + 1);
          memcpy(szKey, *pszCurrent, iLength);
          szKey[iLength] = '\0';
          
          errorMessage = dasher_app_settings_cl_set(pPrivate->pAppSettings, szKey, szJoin + 1);
          
          g_free(szKey);
        }
        
        if (errorMessage) {
          // Note to translators: This string will be output when --options= specifies an unknown option.
          g_critical("%s: '%s', %s", _("Invalid option string specified"), *pszCurrent, errorMessage);
          return 0;
        }

        ++pszCurrent;
      }

      g_strfreev(pszOptionTerms);
    }
    /* --- */


    dasher_editor_initialise(pPrivate->pEditor, pPrivate->pAppSettings, pDasherMain, pPrivate->pXML, NULL);


    dasher_main_setup_window(pDasherMain);

    /* Create the editor */
    gchar *szFullPath = NULL;

    if(pCommandLine) {
      if(pCommandLine->szFilename) {
        if(!g_path_is_absolute(pCommandLine->szFilename)) {
          char *cwd;
          cwd = (char *)malloc(1024 * sizeof(char));
          getcwd(cwd, 1024);
          szFullPath = g_build_path("/", cwd, pCommandLine->szFilename, NULL);
        }
        else {
          szFullPath = g_strdup(pCommandLine->szFilename);
        }
      }
    }

    // TODO: Fix this
//     pPrivate->pEditor = GTK_EDITOR(


// dasher_editor_initialise(pPrivate->pAppSettings, pDasherMain, pPrivate->pXML, szFullPath);

    g_free(szFullPath);

    // TODO: Were these really needed?
    //    g_signal_connect(pPrivate->pEditor, "filename_changed", G_CALLBACK(dasher_main_cb_filename_changed), pDasherMain);
    //    g_signal_connect(pPrivate->pEditor, "buffer_changed", G_CALLBACK(dasher_main_cb_buffer_changed), pDasherMain);
    //    g_signal_connect(pPrivate->pEditor, "context_changed", G_CALLBACK(dasher_main_cb_context_changed), pDasherMain);

    /* Create the preferences window */
    dasher_main_create_preferences(pDasherMain);

    /* Create the lock dialogue (to be removed in future versions) */
#ifndef WITH_MAEMO
    dasher_lock_dialogue_new(pPrivate->pXML, pPrivate->pMainWindow);
#else
    dasher_lock_dialogue_new(pPrivate->pXML, 0);
#endif

    g_object_unref(pPrivate->pXML);
    pPrivate->pXML = 0;

    g_object_unref(pPrivate->pPrefXML);
    pPrivate->pPrefXML = 0;

    /* Set up various bits and pieces */
    dasher_main_set_window_title(pDasherMain);
    dasher_main_populate_controls(pDasherMain);
    dasher_main_connect_control(pDasherMain);

    gtk_key_snooper_install(dasher_main_key_snooper, pDasherMain);

    return pDasherMain;   
  }
}

static GtkBuilder *
dasher_main_open_gui_xml(DasherMain *pSelf, const char *szGUIFilename) {
  GError *e = NULL;
  GtkBuilder *xml = gtk_builder_new();

  g_message("Opening GUI file: %s", szGUIFilename);

  if (!gtk_builder_add_from_file(xml, szGUIFilename, &e)) {
    g_message("Can't find GUI file: %s. Dasher is unlikely to be correctly "
              "installed. (%s)", szGUIFilename, e->message);
    exit(1);
  }

  gtk_builder_connect_signals(xml, pSelf);
  
  return xml;
}


#define WRAP_CPP_CB(item) \
extern "C" void \
dasher_main_cb_##item(GtkAction *obj, DasherMain *p)\
{\
  dasher_main_command_##item(p);\
}

/* XXX PRLW: There is mention of "tutorial", but no function, (and
 * preferences_alphabet isn't called externally.)
 * editor passes on the action strings to dasher_editor_command which
 * land in dasher_editor_internal.
 */
WRAP_CPP_CB(import)
WRAP_CPP_CB(quit)
WRAP_CPP_CB(preferences)
WRAP_CPP_CB(help)
WRAP_CPP_CB(about)

extern "C" void
dasher_main_cb_editor(GtkAction *obj, DasherMain *pSelf)
{
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
  DASHER_ASSERT(pPrivate->pEditor != NULL);
  const gchar *action = gtk_action_get_name(obj);
  dasher_editor_command(pPrivate->pEditor, action);
}

static void 
dasher_main_load_interface(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
  const char *szGUIFilename = NULL;
  const char *szPrefGUIFilename = NULL;

#if WITH_MAEMO
#ifdef WITH_MAEMOFULLSCREEN
  szGUIFilename = PROGDATA "/dashermaemofullscreen.ui";
#else
  szGUIFilename = PROGDATA "/dashermaemo.ui";
#endif
  szPrefGUIFilename = PROGDATA "/dashermaemo.preferences.ui";
#else
  switch(dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_STYLE)) {
  case APP_STYLE_TRAD:
    szGUIFilename = PROGDATA "/dasher.traditional.ui";
    break;
  case APP_STYLE_COMPOSE:
    szGUIFilename = PROGDATA "/dasher.compose.ui";
    break;
  case APP_STYLE_DIRECT:
    szGUIFilename = PROGDATA "/dasher.direct.ui";
    break;
  case APP_STYLE_FULLSCREEN:
    szGUIFilename = PROGDATA "/dasher.fullscreen.ui";
    break;
  default:
    g_error("Inconsistent application style specified.");
  }
  szPrefGUIFilename = PROGDATA "/dasher.preferences.ui";
#endif

  if(!szGUIFilename) {
    g_error("Failure to determine GUI filename");
  }

  pPrivate->pXML     = dasher_main_open_gui_xml(pSelf, szGUIFilename);
  pPrivate->pPrefXML = dasher_main_open_gui_xml(pSelf, szPrefGUIFilename);

#ifndef HAVE_GTK_SHOW_URI
  GtkAction *helpact =
           GTK_ACTION(gtk_builder_get_object(pPrivate->pXML, "action_help"));
  gtk_action_set_sensitive(helpact, false);
  gtk_action_set_visible(helpact, false);
#endif

  // Save the details of some of the widgets for later
  //  pPrivate->pActionPane = gtk_builder_get_object(pPrivate->pXML, "vbox39");
  // pPrivate->pBufferView = gtk_builder_get_object(pPrivate->pXML, "the_text_view");
  pPrivate->pDivider = GTK_PANED(gtk_builder_get_object(pPrivate->pXML, "main_divider"));
  //  pPrivate->pEditPane = gtk_builder_get_object(pPrivate->pXML, "vbox40");
  pPrivate->pMainWindow = GTK_WINDOW(gtk_builder_get_object(pPrivate->pXML, "window"));
  pPrivate->pToolbar = GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "toolbar"));
  //  pPrivate->pMenuBar = gtk_builder_get_object(pPrivate->pXML, "dasher_menu_bar");
  pPrivate->pDasherWidget = GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "DasherControl"));

#ifndef WITH_MAEMO
  pPrivate->pSpeedBox = GTK_SPIN_BUTTON(gtk_builder_get_object(pPrivate->pXML, "spinbutton1"));
  pPrivate->pAlphabetCombo = GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "combobox1"));
  pPrivate->pStatusControl = GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "hbox8"));

  pPrivate->pAlphabetList = gtk_list_store_new(1, G_TYPE_STRING);
  gtk_combo_box_set_model(GTK_COMBO_BOX(pPrivate->pAlphabetCombo), 
                          GTK_TREE_MODEL(pPrivate->pAlphabetList));

  GtkCellRenderer *pRenderer;
  pRenderer = gtk_cell_renderer_text_new();
  g_object_set(G_OBJECT(pRenderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
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
  file_menu = gtk_builder_get_object(pPrivate->pXML, "file_menu");
  options_menu = gtk_builder_get_object(pPrivate->pXML, "options1_menu");
  help_menu = gtk_builder_get_object(pPrivate->pXML, "help_menu");
  file_menu_item = gtk_menu_item_new_with_label ("File");
  options_menu_item = gtk_menu_item_new_with_label ("Options");
  help_menu_item = gtk_menu_item_new_with_label ("Help");

  g_object_ref(file_menu);
  g_object_ref(options_menu);
  g_object_ref(help_menu);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(gtk_builder_get_object(pPrivate->pXML, "file_menu")), NULL);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(gtk_builder_get_object(pPrivate->pXML, "options1")), NULL);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(gtk_builder_get_object(pPrivate->pXML, "help_menu")), NULL);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item),file_menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(options_menu_item),options_menu); 
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item),help_menu);
  gtk_menu_shell_append((GtkMenuShell *)main_menu, file_menu_item);
  gtk_menu_shell_append((GtkMenuShell *)main_menu, options_menu_item);
  gtk_menu_shell_append((GtkMenuShell *)main_menu, help_menu_item); 

  g_object_unref(file_menu);
  g_object_unref(options_menu);
  g_object_unref(help_menu);

  hildon_program_set_common_menu(pPrivate->pProgram, main_menu); 

  gtk_widget_show_all( GTK_WIDGET( main_menu ) );

//   /* And toolbar */
//   GtkWidget *toolbar;
//   toolbar = gtk_builder_get_object(pPrivate->pXML, "toolbar");
//   g_print("Got %p\n",toolbar);
//   gtk_widget_reparent (toolbar, appview->vbox);

  gtk_widget_show_all(GTK_WIDGET(pPrivate->pHWindow));

  gtk_widget_destroy(GTK_WIDGET(pPrivate->pMainWindow));
  pPrivate->pMainWindow = pPrivate->pHWindow;

  g_signal_connect(G_OBJECT(pPrivate->pHWindow), "delete_event", G_CALLBACK(ask_save_before_exit), NULL);

#endif // Maemo fullscreen
#endif // Maemo 

  //  pPrivate->bHidden = false;
  //  pPrivate->bGrabbed = false;

  //  pPrivate->iPosition = 100; // FIXME - make this persistant

  // TODO: Specify callbacks in glade file
  // TODO: Rationalise focus
  //  g_signal_connect(G_OBJECT(pPrivate->pBufferView), "button-release-event", G_CALLBACK(take_real_focus), NULL);
  // g_signal_connect(G_OBJECT(pPrivate->pBufferView), "key-press-event", G_CALLBACK(edit_key_press), NULL);
  //g_signal_connect(G_OBJECT(pPrivate->pBufferView), "key-release-event", G_CALLBACK(edit_key_release), NULL);

  pPrivate->iAlphabetComboHandler = g_signal_connect(G_OBJECT(pPrivate->pAlphabetCombo), "changed", G_CALLBACK(alphabet_combo_changed), NULL);
  
  //  dasher_main_build_context_menu(pSelf);

  // Create a Maemo helper if necessary
#if defined WITH_MAEMO && !defined WITH_MAEMOFULLSCREEN
  pPrivate->pMaemoHelper = dasher_maemo_helper_new(pPrivate->pMainWindow);
#endif

  // Set up any non-registry-dependent options
#ifdef WITH_GPE
  gtk_window_set_decorated(pPrivate->pMainWindow, false);
#endif
  
  // Hide any widgets which aren't appropriate for this mode
  // XXX PRLW: chances are these aren't defined in direct.ui anyway => we are
  // hiding non-existent widgets.
  
  if(dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_STYLE) == APP_STYLE_DIRECT) {
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "dasher_menu_bar")));
      
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "tb_command_new")));
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "tb_command_open")));
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "tb_command_save")));
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "tb_command_saveas")));
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "separatortoolitem1")));
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "tb_command_cut")));
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "tb_command_copy")));
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "tb_command_paste")));
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "separatortoolitem2")));
  }
    
  pPrivate->pEditor = DASHER_EDITOR(gtk_builder_get_object(pPrivate->pXML, "DasherEditor"));
  // TODO: szFullPath
  pPrivate->bWidgetsInitialised = true;
}

static void 
dasher_main_create_preferences(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
  pPrivate->pPreferencesDialogue = dasher_preferences_dialogue_new(pPrivate->pPrefXML, pPrivate->pEditor, pPrivate->pAppSettings, pPrivate->pMainWindow);
}

// DasherEditor *
// dasher_main_get_editor(DasherMain *pSelf) {
//   DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
//   return pPrivate->pEditor;
// }

static void 
dasher_main_handle_parameter_change(DasherMain *pSelf, int iParameter) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

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
#ifndef WITH_MAEMO
  case LP_MAX_BITRATE:
    gtk_spin_button_set_value(pPrivate->pSpeedBox, dasher_app_settings_get_long(pPrivate->pAppSettings, LP_MAX_BITRATE) / 100.0);
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
    if(dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_MAEMO_SIZE) == 0) {
      int iWidth;
      gtk_window_get_size(GTK_WINDOW(pPrivate->pMainWindow), &iWidth, NULL);
      gtk_widget_set_size_request(pPrivate->pMainWindow, -1, 150);
      gtk_window_resize(GTK_WINDOW(pPrivate->pMainWindow), iWidth, 150);
      gtk_widget_set_size_request(pPrivate->pDasherWidget, 175, -1);
    }
    else {
      int iWidth;
      gtk_window_get_size(GTK_WINDOW(pPrivate->pMainWindow), &iWidth, NULL); 
      gtk_widget_set_size_request(pPrivate->pMainWindow, -1, 250);
      gtk_window_resize(GTK_WINDOW(pPrivate->pMainWindow), iWidth, 250);
      gtk_widget_set_size_request(pPrivate->pDasherWidget, 280, -1);
    }
    if(bVisible)
      gtk_widget_show(pPrivate->pMainWindow);
    break;
  }
#endif
  }

  if(pPrivate->pPreferencesDialogue)
    dasher_preferences_dialogue_handle_parameter_change(pPrivate->pPreferencesDialogue, iParameter);

  if(pPrivate->pEditor)
    dasher_editor_handle_parameter_change(pPrivate->pEditor, iParameter);
}

static void 
dasher_main_load_state(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  int iWindowWidth;
  int iWindowHeight;
  int iEditHeight;
  
  if(dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_STYLE) != APP_STYLE_COMPOSE) {
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

  gtk_paned_set_position(pPrivate->pDivider, iEditHeight);

  pPrivate->iWidth = iWindowWidth;
  pPrivate->iHeight = iWindowHeight;


  int iWindowX;
  int iWindowY;
 
  iWindowX = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_X);
  iWindowY = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_Y);

  gtk_window_move(GTK_WINDOW(pPrivate->pMainWindow), iWindowX, iWindowY);

  
  //  pPrivate->iPosition = dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_DOCK_POSITION);
}

static void 
dasher_main_save_state(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  if(!pPrivate->bWidgetsInitialised)
    return;
  
  int iWindowWidth;
  int iWindowHeight;
  int iEditHeight;
  
   gtk_window_get_size(GTK_WINDOW(pPrivate->pMainWindow), &iWindowWidth, &iWindowHeight);
   iEditHeight = gtk_paned_get_position(pPrivate->pDivider);

   if(dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_STYLE) != APP_STYLE_COMPOSE) {
     // APP_STYLE_DIRECT doesn't have an edit window.
     if (dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_STYLE) != APP_STYLE_DIRECT)
       dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_EDIT_HEIGHT, iEditHeight);
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_WIDTH, iWindowWidth);
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_HEIGHT, iWindowHeight);
   }
   else {
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_EDIT_WIDTH, iEditHeight);
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_WIDTH_H, iWindowWidth);
     dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_SCREEN_HEIGHT_H, iWindowHeight);
   } 

   int iWindowX;
   int iWindowY;
   gtk_window_get_position(GTK_WINDOW(pPrivate->pMainWindow), &iWindowX, &iWindowY);

   dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_X, iWindowX);
   dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_Y, iWindowY);

   //   dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_DOCK_POSITION, pPrivate->iPosition);
}

void 
dasher_main_show(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
  gtk_widget_show(GTK_WIDGET(pPrivate->pMainWindow));
}

static void 
dasher_main_setup_window(DasherMain *pSelf) {
  dasher_main_setup_window_style(pSelf);
  dasher_main_setup_window_state(pSelf);
  dasher_main_setup_internal_layout(pSelf);

//   DasherMainPrivate *pPrivate = (DasherMainPrivate *)(pSelf->private_data);
  
//   if(dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_GLOBAL_KEYBOARD))
//     gdk_window_add_filter(0, keyboard_filter_cb, 0);
//   else
//     gdk_window_remove_filter(0, keyboard_filter_cb, 0);
}

static void 
dasher_main_populate_controls(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  // Populate the alphabet chooser
  dasher_main_populate_alphabet_combo(pSelf);
  
  // Set the value of the speed spinner
  gtk_spin_button_set_value(pPrivate->pSpeedBox, 
                            dasher_app_settings_get_long(pPrivate->pAppSettings, LP_MAX_BITRATE) / 100.0);
}

static void 
dasher_main_connect_control(DasherMain *pSelf) {
  /* TODO: This is very much temporary - we need to think of a better
     way of presenting application commands in a unified way */
#ifdef GNOME_SPEECH
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                    Dasher::CControlManager::CTL_USER,
                                    "Speak", -1 );

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                   Dasher::CControlManager::CTL_USER, 
                                   Dasher::CControlManager::CTL_ROOT, -2);


  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                    Dasher::CControlManager::CTL_USER + 1, 
                                    "All", -1 );

  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                    Dasher::CControlManager::CTL_USER + 2, 
                                    "Last", -1 );

  gtk_dasher_control_register_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                    Dasher::CControlManager::CTL_USER + 3, 
                                    "Repeat", -1 );

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                   Dasher::CControlManager::CTL_USER + 1, 
                                   Dasher::CControlManager::CTL_USER, -2);

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                   -1, 
                                   Dasher::CControlManager::CTL_USER + 1, -2);

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                   Dasher::CControlManager::CTL_USER + 2, 
                                   Dasher::CControlManager::CTL_USER, -2);

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                   -1,
                                   Dasher::CControlManager::CTL_USER + 2, -2);

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                   Dasher::CControlManager::CTL_USER + 3, 
                                   Dasher::CControlManager::CTL_USER, -2);

  gtk_dasher_control_connect_node( GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                   -1,
                                   Dasher::CControlManager::CTL_USER + 3, -2);

#endif

}

static gboolean 
dasher_main_command(DasherMain *pSelf, const gchar *szCommand) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  if(!strcmp(szCommand, "import")) {
    dasher_main_command_import(pSelf);
    return TRUE;
  }

  if(!strcmp(szCommand, "quit")) {
    dasher_main_command_quit(pSelf);
    return TRUE;
  }
  
  if(!strcmp(szCommand, "preferences")) {
    dasher_main_command_preferences(pSelf);
    return TRUE;
  }

  if(!strcmp(szCommand, "preferences_alphabet")) {
    dasher_main_command_preferences_alphabet(pSelf);
    return TRUE;
  }

  if(!strcmp(szCommand, "tutorial")) {
    dasher_main_command_tutorial(pSelf);
    return TRUE;
  }
  
  if(!strcmp(szCommand, "help")) {
    dasher_main_command_help(pSelf);
    return TRUE;
  }

  if(!strcmp(szCommand, "about")) {
    dasher_main_command_about(pSelf);
    return TRUE;
  }

  if(pPrivate->pEditor)
    return dasher_editor_command(pPrivate->pEditor, szCommand);

  return FALSE;
}

/* Private methods */

/* Window state is basically size and position */
static void 
dasher_main_setup_window_state(DasherMain *pSelf) {
  dasher_main_load_state(pSelf);
  
  // TODO: Setup positioning here - need to think up a policy on this
}


/* Setup the window style - this is defined to be window manager hints and the like */
static void 
dasher_main_setup_window_style(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  switch(dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_STYLE)) {
  case APP_STYLE_TRAD:
    // Nothing to do
    break;
  case APP_STYLE_COMPOSE:
    // Nothing to do
    break;
  case APP_STYLE_DIRECT:
    // Direct mode - set always on top
    gtk_window_set_keep_above(GTK_WINDOW(pPrivate->pMainWindow), true);

    // Refuse focus
    gtk_window_set_accept_focus(GTK_WINDOW(pPrivate->pMainWindow), false);

    // Stick on all desktops
    gtk_window_stick(GTK_WINDOW(pPrivate->pMainWindow));
    break;
  case APP_STYLE_FULLSCREEN:
    // Fullscreen mode - set fullscreen
    gtk_window_fullscreen(GTK_WINDOW(pPrivate->pMainWindow));
    break;
  default:
    g_error("Inconsistent application style specified.");
  }
}

/* Internal layout is the visibility of various widgets */
static void 
dasher_main_setup_internal_layout(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
 
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
}

// TODO: Fold into setup controls?
static void 
dasher_main_set_window_title(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  const gchar *szFilename = dasher_editor_get_filename(pPrivate->pEditor);

  // Note to translators: This is the name of the dasher program as it appears
  // in a window title.
  gchar * dasher = _("Dasher");
  if(szFilename == 0) {
    gtk_window_set_title(GTK_WINDOW(pPrivate->pMainWindow), dasher);
  }
  else {
    gchar *title = g_strdup_printf("%s - %s", dasher, szFilename);
    gtk_window_set_title(GTK_WINDOW(pPrivate->pMainWindow), title);
    g_free (title);
  }
}

static void 
dasher_main_command_import(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  GtkWidget *pFileSel = gtk_file_chooser_dialog_new(_("Select File"), 
                                                    GTK_WINDOW(pPrivate->pMainWindow), 
                                                    GTK_FILE_CHOOSER_ACTION_OPEN, 
                                                    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, 
                                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
                                                    NULL);

#ifdef TEACH_TRAINING_HELPER_LOAD_FILE_ABOUT_URI
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(pFileSel), FALSE);
#endif

  if(gtk_dialog_run(GTK_DIALOG(pFileSel)) == GTK_RESPONSE_ACCEPT) {

#ifdef TEACH_TRAINING_HELPER_LOAD_FILE_ABOUT_URI
    gchar *szFilename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(pFileSel));
#else
    gchar *szFilename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pFileSel));
#endif

    gtk_dasher_control_train(GTK_DASHER_CONTROL(pPrivate->pDasherWidget), szFilename);

    g_free(szFilename);
  }

  gtk_widget_destroy(pFileSel);
}

static void dasher_main_command_quit(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  GtkWidget *pDialogue = NULL;

  if(dasher_editor_file_changed(pPrivate->pEditor)) {
// XXX PRLW: Just open the save dialogue box.
#if 0
    const gchar *szFilename = dasher_editor_get_filename(pPrivate->pEditor);

    if(szFilename) {
      pDialogue = gtk_message_dialog_new(GTK_WINDOW(pPrivate->pMainWindow), GTK_DIALOG_MODAL, 
                                         GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, 
                                         _("Do you want to save your changes to %s?\n\nYour changes will be lost if you don't save them."),
                                         szFilename);
    }
    else {
#endif
      pDialogue = gtk_message_dialog_new(GTK_WINDOW(pPrivate->pMainWindow), GTK_DIALOG_MODAL, 
                                         GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, 
                                         _("Do you want to save your changes?\n\nYour changes will be lost if you don't save them."));
#if 0
    }
#endif

    gtk_dialog_add_buttons(GTK_DIALOG(pDialogue), 
                           _("Don't save"), GTK_RESPONSE_REJECT,
                           _("Don't quit"), GTK_RESPONSE_CANCEL, 
                           _("Save and quit"), GTK_RESPONSE_ACCEPT, 
                           NULL);

    switch (gtk_dialog_run(GTK_DIALOG(pDialogue))) {
    case GTK_RESPONSE_REJECT:
      gtk_main_quit();
      break;
    case GTK_RESPONSE_CANCEL:
      gtk_widget_destroy(GTK_WIDGET(pDialogue));
      break;
    case GTK_RESPONSE_ACCEPT:
      dasher_editor_command(pPrivate->pEditor, "save");
      gtk_main_quit();
      break;
    }
  }
  else {
        pDialogue = gtk_message_dialog_new(GTK_WINDOW(pPrivate->pMainWindow), GTK_DIALOG_MODAL, 
                                         GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, 
                                         _("Are you sure you wish to quit?"));

            gtk_dialog_add_buttons(GTK_DIALOG(pDialogue), 
                           _("Don't quit"), GTK_RESPONSE_REJECT,
                           _("Quit"), GTK_RESPONSE_ACCEPT, 
                           NULL);

    switch (gtk_dialog_run(GTK_DIALOG(pDialogue))) {
    case GTK_RESPONSE_REJECT:
        gtk_widget_destroy(GTK_WIDGET(pDialogue));
      break;
    case GTK_RESPONSE_ACCEPT:
      gtk_main_quit();
    }
  }
}

static void 
dasher_main_command_preferences(DasherMain *pSelf) { 
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
  dasher_preferences_dialogue_show(pPrivate->pPreferencesDialogue, 0);
}

static void 
dasher_main_command_preferences_alphabet(DasherMain *pSelf) { 
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
  dasher_preferences_dialogue_show(pPrivate->pPreferencesDialogue, 1);
}

static void 
dasher_main_command_tutorial(DasherMain *pSelf) {
  // TODO: Implement this
}

static void 
dasher_main_command_help(DasherMain *pSelf) {
#ifdef HAVE_GTK_SHOW_URI
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
  GdkScreen *scr;
  GError *err = NULL;

  scr = gtk_widget_get_screen(GTK_WIDGET(pPrivate->pMainWindow));
  if (!gtk_show_uri(scr, "ghelp:dasher", gtk_get_current_event_time(), &err)) {
    GtkWidget *d;
    d = gtk_message_dialog_new(GTK_WINDOW(pPrivate->pMainWindow),
                               GTK_DIALOG_MODAL,
                               GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                               "%s", _("Unable to open help file"));
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(d),
                               "%s", err->message);
    g_signal_connect(d, "response", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_window_present(GTK_WINDOW(d));

    g_error_free (err);
  }
#endif
}

static void 
dasher_main_command_about(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  // In alphabetical order - please keep this in sync with the AUTHORS
  // file at root of the package tree
  const gchar *authors[] = {
    "Chris Ball",
    "Ignas Budvytis",
    "Peter Conlon",
    "Phil Cowans",
    "Frederik Eaton",
    "Behdad Esfahbod",
    "Matthew Garrett",
    "Chris Hack",
    "Takashi Kaburagi",
    "Sega Kazue",
    "Alan Lawrence",
    "David MacKay",
    "Iain Murray",
    "Martijn van Veen",
    "Keith Vertanen",
    "Hanna Wallach",
    "David Ward",
    "Patrick Welche",
    "Brian Williams",
    "Seb Wills",
    "Will Zou",
    NULL
  };

  // Yeah, should really do some Gnome documentation for it...
  const gchar *documenters[] = {
    "Chris Ball",
    "Matthew Garrett",
    "David MacKay",
    NULL
  };

  gtk_show_about_dialog(GTK_WINDOW(pPrivate->pMainWindow),
                        "authors", authors,
                        "comments", _("Dasher is a predictive text entry application"), 
                        "copyright", "Copyright \xC2\xA9 1998-2010 The Dasher Project", 
                        "documenters", documenters,
                        "license", "GPL 2+",
                        "logo-icon-name", "dasher",
                        "translator-credits", _("translator-credits"),
                        "version", PACKAGE_VERSION,
                        "website", PACKAGE_URL,
                        "wrap-license", true,
                        NULL);
}

static gboolean 
dasher_main_speed_changed(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);
  
  int iNewValue( static_cast<int>(round(gtk_spin_button_get_value(pPrivate->pSpeedBox) * 100)));
  
  if(dasher_app_settings_get_long(pPrivate->pAppSettings, LP_MAX_BITRATE) != iNewValue)
    dasher_app_settings_set_long(pPrivate->pAppSettings, LP_MAX_BITRATE, iNewValue);

  return true;
}

static void 
dasher_main_alphabet_combo_changed(DasherMain *pSelf) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  GtkTreeIter sIter;
  
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(pPrivate->pAlphabetCombo), &sIter)) {
    char *szSelected;
    gtk_tree_model_get(GTK_TREE_MODEL(pPrivate->pAlphabetList), &sIter, 0, &szSelected, -1);
    
    if(!strcmp("More Alphabets...", szSelected)) {
      gtk_combo_box_set_active(GTK_COMBO_BOX(pPrivate->pAlphabetCombo), 0);
      //    dasher_preferences_dialogue_show(pPrivate->pPreferencesDialogue);
      dasher_main_command(pSelf, "preferences_alphabet");
    }
    else 
      dasher_app_settings_set_string(pPrivate->pAppSettings, SP_ALPHABET_ID, szSelected);

    g_free(szSelected);
  }
}

static void 
dasher_main_populate_alphabet_combo(DasherMain *pSelf) {
#ifndef WITH_MAEMO
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

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

static gint
dasher_main_lookup_key(DasherMain *pSelf, guint iKeyVal) {
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

  if(pPrivate->pKeyboardHelper)
    return pPrivate->pKeyboardHelper->ConvertKeycode(iKeyVal);
  else
    return -1;
}


gboolean 
grab_focus() {

  // TODO: reimplement (text view member of class)
  //  gtk_widget_grab_focus(the_text_view);
  //  g_bForwardKeyboard = true;
  return true;
}

/* Callbacks */

extern "C" void 
speed_changed(GtkWidget *pWidget, gpointer user_data) {
  if(g_pDasherMain)
    dasher_main_speed_changed(g_pDasherMain);
}

extern "C" void 
alphabet_combo_changed(GtkWidget *pWidget, gpointer pUserData) {
  if(g_pDasherMain)
    dasher_main_alphabet_combo_changed(g_pDasherMain);
}

extern "C" void 
dasher_main_cb_filename_changed(DasherEditor *pEditor, gpointer pUserData) {
  if(g_pDasherMain)
    dasher_main_set_window_title(g_pDasherMain);
}

extern "C" void 
dasher_main_cb_buffer_changed(DasherEditor *pEditor, gpointer pUserData) {
  if(!g_pDasherMain)
    return;

  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);

  gtk_dasher_control_set_buffer(GTK_DASHER_CONTROL(pPrivate->pDasherWidget), dasher_editor_get_offset(pPrivate->pEditor));
}

extern "C" void 
dasher_main_cb_context_changed(DasherEditor *pEditor, gpointer pUserData) {
  if(!g_pDasherMain)
    return;

  if(!g_bSend)
    return;

  DasherMain *pDasherMain = DASHER_MAIN(g_pDasherMain);
  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pDasherMain);

  gtk_dasher_control_set_offset(GTK_DASHER_CONTROL(pPrivate->pDasherWidget), dasher_editor_get_offset(pPrivate->pEditor));
}

extern "C" gboolean 
dasher_main_cb_window_close(GtkWidget *pWidget, gpointer pUserData) {
  dasher_main_command(g_pDasherMain, "quit");
  
  /* Returning true stops further propagation */
  return TRUE; 
}

extern "C" void 
parameter_notification(GtkDasherControl *pDasherControl, gint iParameter, gpointer data) { 
  if(g_pDasherMain)
    dasher_main_handle_parameter_change(g_pDasherMain, iParameter);
}

// TODO: Not really sure what happens here - need to sort out focus behaviour in general
extern "C" bool 
focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
  return grab_focus();
}

extern "C" bool 
edit_focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
  return true;
}

extern "C" gboolean 
take_real_focus(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
  //  g_bForwardKeyboard = false;
  return false;
}

extern "C" gboolean 
edit_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
  // TODO: Reimplement

//   if(g_bForwardKeyboard) {
//     gboolean *returnType;
//     g_signal_emit_by_name(GTK_OBJECT(pPrivate->pDasherWidget), "key_press_event", event, &returnType);
//     return true;
//   }
//   else {
//     return false;
//   }

  // TODO: Check callback return functions
  return false;
}

extern "C" gboolean 
edit_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data) { 
  // TODO: reimplement

//   if(g_bForwardKeyboard) {
//     gboolean *returnType;
//     g_signal_emit_by_name(GTK_OBJECT(pPrivate->pDasherWidget), "key_release_event", event, &returnType);
//     return true;
//   }
//   else {
//     return false;
//   }

  // TODO: Check callback return functions
  return false;
}

extern "C" gboolean 
test_focus_handler(GtkWidget *pWidget, GtkDirectionType iDirection, gpointer *pUserData) {
  return FALSE;
}

extern "C" void 
handle_context_request(GtkDasherControl * pDasherControl, gint iOffset, gint iLength, gpointer data) { 
  if(!g_pDasherMain)
    return;

  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);

  if(!pPrivate->pEditor || !pPrivate->pDasherWidget)
    return;

  gtk_dasher_control_set_context(GTK_DASHER_CONTROL(pPrivate->pDasherWidget), dasher_editor_get_context(pPrivate->pEditor, iOffset, iLength));
}

extern "C" void 
handle_control_event(GtkDasherControl *pDasherControl, gint iEvent, gpointer data) { 
  if(!g_pDasherMain)
    return;
  
  /* TODO: replace this with something a little more sensible */

  switch(iEvent) {
  case Dasher::CControlManager::CTL_USER + 1:
    dasher_main_command(g_pDasherMain, "speakall");
    return;
  case Dasher::CControlManager::CTL_USER + 2:
    dasher_main_command(g_pDasherMain, "speaklast");
    return;
  case Dasher::CControlManager::CTL_USER + 3:
    dasher_main_command(g_pDasherMain, "speakrepeat");
    return;
  default:
    break;
  }


  // TODO: This is a horrible hack here to make the release work!  

  g_bSend = false;

  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);
  dasher_editor_handle_control(pPrivate->pEditor, iEvent);

  gtk_dasher_control_set_control_offset(GTK_DASHER_CONTROL(pPrivate->pDasherWidget), 
                                        dasher_editor_get_offset(pPrivate->pEditor));

  g_bSend = true;
  // ---
}


extern "C" void 
handle_start_event(GtkDasherControl *pDasherControl, gpointer data) { 
  if(!g_pDasherMain)
    return;

  DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);
  
  dasher_editor_grab_focus(pPrivate->pEditor);
}


// TODO: Make this only work for children of the main window
extern "C" gint 
dasher_main_key_snooper(GtkWidget *pWidget, GdkEventKey *pEvent,
                        gpointer pUserData) {
  DasherMain *pSelf = DASHER_MAIN(pUserData);

  gint iButton = dasher_main_lookup_key(pSelf, pEvent->keyval);
  
  if(iButton != -1) {
    DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(pSelf);

    if (pWidget == GTK_WIDGET(pPrivate->pMainWindow) ||
        gtk_widget_is_ancestor(pWidget, GTK_WIDGET(pPrivate->pMainWindow))) {
      if(pPrivate->pDasherWidget) {
        if(pEvent->type == GDK_KEY_PRESS)
          gtk_dasher_control_external_key_down(GTK_DASHER_CONTROL(pPrivate->pDasherWidget), iButton);
        else
          gtk_dasher_control_external_key_up(GTK_DASHER_CONTROL(pPrivate->pDasherWidget), iButton);
      }
      
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  else {
    return FALSE;
  }
}

// Callbacks from the Dasher widget

extern "C" void 
handle_stop_event(GtkDasherControl *pDasherControl, gpointer data) {
  if(g_pDasherMain) {
    DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);

    if(pPrivate->pEditor)
      dasher_editor_handle_stop(pPrivate->pEditor);
  }
}

extern "C" void 
on_message(GtkDasherControl *pDasherControl, gpointer pMessageInfo, gpointer pUserData) { 
  // TODO: I don't believe that this is widely used, but possibly need to reimplement

//   if(g_pDasherMain) {
//     DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);

//     if(pPrivate->pEditor)
//        dasher_editor_display_message(pPrivate->pEditor, (DasherMessageInfo *)pMessageInfo);
//   }
}

extern "C" void 
on_command(GtkDasherControl *pDasherControl, gchar *szCommand, gpointer pUserData) {
  if(g_pDasherMain) {
    DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);

    if(pPrivate->pEditor)
      dasher_editor_command(pPrivate->pEditor, szCommand);
  }
}

// TODO: The following two should probably be made the same
extern "C" void 
handle_request_settings(GtkDasherControl * pDasherControl, gpointer data) {
  // TODO: reimplement
  //  dasher_preferences_dialogue_show(g_pPreferencesDialogue);
}

extern "C" void 
gtk2_edit_delete_callback(GtkDasherControl *pDasherControl, const gchar *szText, int iOffset, gpointer user_data) {
  if(g_pDasherMain) {
    DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);

    if(pPrivate->pEditor) {
      gint displaylength = g_utf8_strlen(szText, -1);
      dasher_editor_delete(pPrivate->pEditor, displaylength, iOffset);
    }
  }
}

extern "C" void 
gtk2_edit_output_callback(GtkDasherControl *pDasherControl, const gchar *szText, int iOffset, gpointer user_data) {
  if(g_pDasherMain) {
    DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);

    if(pPrivate->pEditor) {
      dasher_editor_output(pPrivate->pEditor, szText, iOffset);
    }
  }
}

// TODO: The following aren't exported from the editor - need to fix this

extern "C" void 
convert_cb(GtkDasherControl *pDasherControl, gpointer pUserData) {
//   if(g_pDasherMain) {
//     DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);

//     if(pPrivate->pEditor) {
//       dasher_editor_convert(pPrivate->pEditor);
//     }
//   }
}

extern "C" void 
protect_cb(GtkDasherControl *pDasherControl, gpointer pUserData) {
//   if(g_pDasherMain) {
//     DasherMainPrivate *pPrivate = DASHER_MAIN_GET_PRIVATE(g_pDasherMain);

//     if(pPrivate->pEditor) {
//       dasher_editor_protect(pPrivate->pEditor);
//     }
//   }
}
