#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <libintl.h>
#include <cstring>

#include "../Common/Common.h"
#include "DasherTypes.h"
//#include "FontDialogues.h"
#include "Preferences.h"
#include "Parameters.h"
#include "module_settings_window.h"

#define DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf) (DasherPreferencesDialoguePrivate *)(pSelf->private_data);

using namespace Dasher;

/* Static global member - use to deal with callbacks */
static DasherPreferencesDialogue *g_pPreferencesDialogue = NULL;

// TODO: Reintroduce advanced settings and start on mouse position offset

typedef struct _BoolTranslation BoolTranslation;

struct _BoolTranslation {
  gint iParameter;
  const gchar *szWidgetName;
  GtkWidget *pWidget;
};

// Checkbox widgets which map directly to boolean parameters

#ifndef WITH_MAEMO

BoolTranslation sBoolTranslationTable[] = {
  {BP_DRAW_MOUSE_LINE, "showmouselinebutton", NULL},
  {BP_DRAW_MOUSE, "showmousebutton", NULL},
  {BP_SHOW_SLIDER, "speedsliderbutton", NULL},
  {APP_BP_SHOW_TOOLBAR, "toolbarbutton", NULL},
  {BP_OUTLINE_MODE, "outlinebutton", NULL},
  {BP_CONTROL_MODE, "control_controlmode", NULL},
  {APP_BP_TIME_STAMP, "timestampbutton", NULL},
  {APP_BP_DOCK, "dockbutton", NULL},
  {BP_START_MOUSE, "leftbutton", NULL},  
  {BP_START_SPACE, "spacebutton", NULL},
  {BP_PAUSE_OUTSIDE, "winpausebutton", NULL},
  {BP_AUTO_SPEEDCONTROL, "adaptivebutton", NULL},
  {BP_LM_ADAPTIVE, "cb_adaptive", NULL}
};

#endif

// List widgets which map directly to string parameters

typedef struct _StringTranslation StringTranslation;

struct _StringTranslation {
  gint iParameter;
  const gchar *szWidgetName;
  GtkWidget *pWidget;
  const gchar *szHelperName;
  GtkWidget *pHelper;
};

#ifdef WITH_MAEMO

StringTranslation sStringTranslationTable[] = {
  {SP_ALPHABET_ID, "AlphabetTree", NULL, NULL, NULL},
  {SP_COLOUR_ID, "ColorTree", NULL, NULL, NULL},
};

#else

StringTranslation sStringTranslationTable[] = {
  {SP_ALPHABET_ID, "AlphabetTree", NULL, NULL, NULL},
  {SP_COLOUR_ID, "ColorTree", NULL, NULL, NULL},
  {SP_INPUT_FILTER, "input_filter_tree_view", NULL, "button13", NULL},
  {SP_INPUT_DEVICE, "input_tree_view", NULL, "button25", NULL}
};

#endif

enum {
  ACTIONS_ID_COLUMN,
  ACTIONS_NAME_COLUMN,
  ACTIONS_SHOW_COLUMN,
  ACTIONS_CONTROL_COLUMN,
  ACTIONS_AUTO_COLUMN,
  ACTIONS_N_COLUMNS
};

// TODO: Look at coding convention stuff for gobjets

/// Newer, object based stuff

struct _DasherPreferencesDialoguePrivate {
  GtkWindow *pPreferencesWindow;
  DasherEditor *pEditor;
  DasherAppSettings *pAppSettings;
  GtkWindow *pMainWindow;

  // TODO: it really would be nice not to have to keep this arround
  GtkBuilder *pXML;

  // TODO: check all of these are really needed
  GtkListStore *pListStore;
  GtkToggleButton *pLRButton;
  GtkToggleButton *pRLButton;
  GtkToggleButton *pTBButton;
  GtkToggleButton *pBTButton;
  GtkRange *pSpeedSlider;
  GtkToggleButton *pMousePosButton;
  GtkComboBox *pMousePosStyle;
  GtkTreeView *pActionTreeView;
  GtkNotebook *pNotebook;

  // Set this to ignore signals (ie loops coming back from setting widgets in response to parameters having changed)
  bool bIgnoreSignals;
};

typedef struct _DasherPreferencesDialoguePrivate DasherPreferencesDialoguePrivate;

// Private member functions
static void dasher_preferences_dialogue_class_init(DasherPreferencesDialogueClass *pClass);
static void dasher_preferences_dialogue_init(DasherPreferencesDialogue *pMain);
static void dasher_preferences_dialogue_destroy(GObject *pObject);

static GtkWidget *dasher_preferences_dialogue_get_helper(DasherPreferencesDialogue *pSelf, int iParameter, const gchar *szValue);
static void dasher_preferences_dialogue_initialise_tables(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_refresh_widget(DasherPreferencesDialogue *pSelf, gint iParameter);
static void dasher_preferences_dialogue_populate_list(DasherPreferencesDialogue *pSelf, GtkTreeView *pView, int iParameter, GtkWidget *pHelper);
static void dasher_preferences_dialogue_update_special(DasherPreferencesDialogue *pSelf, int iID);
static void dasher_preferences_dialogue_refresh_parameter(DasherPreferencesDialogue *pSelf, GtkWidget *pWidget, gpointer pUserData);

/* Special private members called via table (see below) */
static void dasher_preferences_dialogue_populate_special_speed(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_mouse_start(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_orientation(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_appstyle(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_linewidth(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_lm(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_uniform(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_colour(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_dasher_font(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_edit_font(DasherPreferencesDialogue *pSelf);
static void dasher_preferences_dialogue_populate_special_fontsize(DasherPreferencesDialogue *pSelf);

static void dasher_preferences_dialogue_populate_actions(DasherPreferencesDialogue *pSelf);

typedef struct _SpecialControl SpecialControl;

struct _SpecialControl {
  int iID;
  void (*pPopulate)(DasherPreferencesDialogue *);
  gboolean bPrimary;
};

SpecialControl sSpecialControlTable[] = {
  {LP_MAX_BITRATE, dasher_preferences_dialogue_populate_special_speed, true},
  {BP_MOUSEPOS_MODE, dasher_preferences_dialogue_populate_special_mouse_start, true},
  {BP_CIRCLE_START, dasher_preferences_dialogue_populate_special_mouse_start, false},
  {LP_ORIENTATION, dasher_preferences_dialogue_populate_special_orientation, true},
  {LP_REAL_ORIENTATION, dasher_preferences_dialogue_populate_special_orientation, false},
  {APP_LP_STYLE, dasher_preferences_dialogue_populate_special_appstyle, true},
  {LP_LINE_WIDTH, dasher_preferences_dialogue_populate_special_linewidth, true},
  {LP_LANGUAGE_MODEL_ID, dasher_preferences_dialogue_populate_special_lm, true},
  {LP_UNIFORM, dasher_preferences_dialogue_populate_special_uniform, true},
  {BP_PALETTE_CHANGE, dasher_preferences_dialogue_populate_special_colour, true},
  {SP_DASHER_FONT, dasher_preferences_dialogue_populate_special_dasher_font, true},
  {APP_SP_EDIT_FONT, dasher_preferences_dialogue_populate_special_edit_font, true}, 
  {LP_DASHER_FONTSIZE, dasher_preferences_dialogue_populate_special_fontsize, true}
};

// Callback functions
extern "C" void on_action_toggle(GtkCellRendererToggle *pRenderer, gchar *szPath, gpointer pUserData);
extern "C" void on_list_selection(GtkTreeSelection *pSelection, gpointer pUserData);
extern "C" void on_widget_realize(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean show_helper_window(GtkWidget *pWidget, gpointer *pUserData);
extern "C" gboolean dasher_preferences_refresh_foreach_function(GtkTreeModel *pModel, GtkTreePath *pPath, GtkTreeIter *pIter, gpointer pUserData);


/* Start definitions */


GType dasher_preferences_dialogue_get_type() {

  static GType dasher_preferences_dialogue_type = 0;

  if(!dasher_preferences_dialogue_type) {
    static const GTypeInfo dasher_preferences_dialogue_info = {
      sizeof(DasherPreferencesDialogueClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_preferences_dialogue_class_init,
      NULL,
      NULL,
      sizeof(DasherPreferencesDialogue),
      0,
      (GInstanceInitFunc) dasher_preferences_dialogue_init,
      NULL
    };

    dasher_preferences_dialogue_type = g_type_register_static(G_TYPE_OBJECT, "DasherPreferencesDialogue", &dasher_preferences_dialogue_info, static_cast < GTypeFlags > (0));
  }

  return dasher_preferences_dialogue_type;
}

static void dasher_preferences_dialogue_class_init(DasherPreferencesDialogueClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_preferences_dialogue_destroy;
}

static void dasher_preferences_dialogue_init(DasherPreferencesDialogue *pDasherControl) {
  pDasherControl->private_data = new DasherPreferencesDialoguePrivate;
}

static void dasher_preferences_dialogue_destroy(GObject *pObject) {
  DasherPreferencesDialogue *pSelf = (DasherPreferencesDialogue *)pObject;
  DasherPreferencesDialoguePrivate *pPrivate = (DasherPreferencesDialoguePrivate *)(pSelf->private_data);

  g_object_unref(pPrivate->pXML);

  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
  delete pPrivate;
}

// Public methods

DasherPreferencesDialogue *dasher_preferences_dialogue_new(GtkBuilder *pXML, DasherEditor *pEditor, DasherAppSettings *pAppSettings, GtkWindow *pMainWindow) {
  DasherPreferencesDialogue *pDasherControl;
  pDasherControl = (DasherPreferencesDialogue *)(g_object_new(dasher_preferences_dialogue_get_type(), NULL));

  g_pPreferencesDialogue = pDasherControl;

  DasherPreferencesDialoguePrivate *pPrivate = (DasherPreferencesDialoguePrivate *)(pDasherControl->private_data);

  pPrivate->bIgnoreSignals = false;

  pPrivate->pEditor = pEditor;
  pPrivate->pAppSettings = pAppSettings;

  pPrivate->pXML = (GtkBuilder *)g_object_ref(pXML);

  pPrivate->pPreferencesWindow = GTK_WINDOW(gtk_builder_get_object(pXML, "preferences"));

  pPrivate->pActionTreeView = GTK_TREE_VIEW(gtk_builder_get_object(pXML, "action_tree_view"));
  pPrivate->pNotebook = GTK_NOTEBOOK(gtk_builder_get_object(pXML, "notebook1"));

  gtk_window_set_transient_for(pPrivate->pPreferencesWindow, pMainWindow);

  pPrivate->pMainWindow = pMainWindow;

  pPrivate->pSpeedSlider = GTK_RANGE(gtk_builder_get_object(pXML, "hscale1"));
  dasher_preferences_dialogue_initialise_tables(pDasherControl);
  dasher_preferences_dialogue_refresh_widget(pDasherControl, -1);
  dasher_preferences_dialogue_update_special(pDasherControl, -1);

#ifdef WITH_MAEMO
#ifndef WITH_MAEMOFULLSCREEN
  gtk_widget_hide(gtk_builder_get_object(pXML, "displaysizebox"));
#endif
#endif

#ifndef JAPANESE
  gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(pXML, "radiobutton9")));
#endif

  dasher_preferences_dialogue_populate_actions(pDasherControl);

  //  InitialiseFontDialogues(pXML, pAppSettings);

  return pDasherControl;
}

void dasher_preferences_dialogue_show(DasherPreferencesDialogue *pSelf, gint iPage) {
  DasherPreferencesDialoguePrivate *pPrivate = (DasherPreferencesDialoguePrivate *)(pSelf->private_data);
  // FIXME - REIMPLEMENT

  // Keep the preferences window in the correct position relative to the
  // main Dasher window
  
  //  gtk_window_set_transient_for(pPrivate->pPreferencesWindow,pPrivate->pMainWindow);
#ifdef WITH_MAEMO
#ifndef WITH_MAEMOFULLSCREEN
  // gtk_window_set_keep_above(pPrivate->pPreferencesWindow, true);
#endif
#endif
  // TODO: reimplement
  //  gtk_window_set_keep_above(pPrivate->pPreferencesWindow, dasher_main_topmost(g_pDasherMain));

  if(iPage > 0) {
    gtk_notebook_set_current_page(pPrivate->pNotebook, iPage - 1);
  }

  gtk_window_present(pPrivate->pPreferencesWindow);
}


void dasher_preferences_dialogue_handle_parameter_change(DasherPreferencesDialogue *pSelf, int iParameter) {
  dasher_preferences_dialogue_refresh_widget(pSelf, iParameter);
  dasher_preferences_dialogue_update_special(pSelf, iParameter);
}


// --- Generic Options ---

void dasher_preferences_dialogue_initialise_tables(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
 

#ifndef WITH_MAEMO
  int iNumBoolEntries = sizeof(sBoolTranslationTable) / sizeof(BoolTranslation);
  for(int i(0); i < iNumBoolEntries; ++i) {
    sBoolTranslationTable[i].pWidget = GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, sBoolTranslationTable[i].szWidgetName));
  }
#endif

  int iNumStringEntries = sizeof(sStringTranslationTable) / sizeof(StringTranslation);
  for(int i(0); i < iNumStringEntries; ++i) {
    sStringTranslationTable[i].pWidget = GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, sStringTranslationTable[i].szWidgetName));
    if(sStringTranslationTable[i].szHelperName)
      sStringTranslationTable[i].pHelper = GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, sStringTranslationTable[i].szHelperName));

    dasher_preferences_dialogue_populate_list(pSelf, GTK_TREE_VIEW(sStringTranslationTable[i].pWidget), sStringTranslationTable[i].iParameter, sStringTranslationTable[i].pHelper);
    g_signal_connect(sStringTranslationTable[i].pWidget, "realize", (GCallback)on_widget_realize, &sStringTranslationTable[i].iParameter);
  }
}

extern "C" gboolean dasher_preferences_refresh_foreach_function(GtkTreeModel *pModel, GtkTreePath *pPath, GtkTreeIter *pIter, gpointer pUserData) {
  gpointer *pPointers = (gpointer *)pUserData;

  gchar *szTarget = (gchar *)pPointers[0];
  gchar *szComparison;

  gtk_tree_model_get(pModel, pIter, 2, &szComparison, -1);

  if(!strcmp(szTarget, szComparison)) {
    g_free(szComparison);
    // Todo: set selection here?
    gtk_tree_view_set_cursor((GtkTreeView *)pPointers[1], pPath, NULL, false);

    gtk_tree_view_scroll_to_cell((GtkTreeView *)pPointers[1], pPath, NULL, true, 0.5, 0.0);

    return true;
  }

  g_free(szComparison);
  
  return false;
}


void dasher_preferences_dialogue_refresh_widget(DasherPreferencesDialogue *pSelf, gint iParameter) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

#ifndef WITH_MAEMO
  int iNumBoolEntries = sizeof(sBoolTranslationTable) / sizeof(BoolTranslation);
  for(int i(0); i < iNumBoolEntries; ++i) {
    if((iParameter == -1) || (sBoolTranslationTable[i].iParameter == iParameter)) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sBoolTranslationTable[i].pWidget), dasher_app_settings_get_bool(pPrivate->pAppSettings, sBoolTranslationTable[i].iParameter));
    }
  }
#endif

  // TODO: I believe that this is being called initially before the
  // widgets are realised, so the selection isn't being correctly
  // brought into view
  int iNumStringEntries = sizeof(sStringTranslationTable) / sizeof(StringTranslation);
  for(int i(0); i < iNumStringEntries; ++i) {
    if((iParameter == -1) || (sStringTranslationTable[i].iParameter == iParameter)) {
      GtkTreeModel *pModel = gtk_tree_view_get_model(GTK_TREE_VIEW(sStringTranslationTable[i].pWidget));

      // TODO: tidy up in a struct
      const void *pUserData[3];
      pUserData[0] = dasher_app_settings_get_string(pPrivate->pAppSettings, sStringTranslationTable[i].iParameter);
      pUserData[1] = GTK_TREE_VIEW(sStringTranslationTable[i].pWidget);
      pUserData[2] = pSelf;
      
      if(sStringTranslationTable[i].pWidget && GTK_WIDGET_REALIZED(sStringTranslationTable[i].pWidget))
        gtk_tree_model_foreach(pModel, dasher_preferences_refresh_foreach_function, pUserData);
    }
  }
}

static void dasher_preferences_dialogue_refresh_parameter(DasherPreferencesDialogue *pSelf, GtkWidget *pWidget, gpointer pUserData) {
#ifndef WITH_MAEMO
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  
  int iNumBoolEntries = sizeof(sBoolTranslationTable) / sizeof(BoolTranslation);
  
  for(int i(0); i < iNumBoolEntries; ++i) {
    if((pWidget == NULL) || (sBoolTranslationTable[i].pWidget == pWidget)) {

      if(GTK_TOGGLE_BUTTON(sBoolTranslationTable[i].pWidget)->active != dasher_app_settings_get_bool(pPrivate->pAppSettings, sBoolTranslationTable[i].iParameter)) {
        
        dasher_app_settings_set_bool(pPrivate->pAppSettings, sBoolTranslationTable[i].iParameter, GTK_TOGGLE_BUTTON(sBoolTranslationTable[i].pWidget)->active);
      }
    }
  }
#endif
}

// TODO: Is this function actually useful? (conversely, is the other call to RefreshFoo elsewhere any use?)
extern "C" void on_widget_realize(GtkWidget *pWidget, gpointer pUserData) {
  gint *pParameter = (gint *)pUserData;
  dasher_preferences_dialogue_refresh_widget(g_pPreferencesDialogue, *pParameter); // TODO: Fix NULL pointer
}

// --- Generic boolean options ---

extern "C" void generic_bool_changed(GtkWidget *widget, gpointer user_data) {
  dasher_preferences_dialogue_refresh_parameter(g_pPreferencesDialogue, widget, user_data);// TODO: fix NULL
}

// --- Generic string options ---

void dasher_preferences_dialogue_populate_list(DasherPreferencesDialogue *pSelf, GtkTreeView *pView, int iParameter, GtkWidget *pHelper) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  
  // TODO: Need to kill helpers on list depopulation

  const gchar *szCurrentValue(dasher_app_settings_get_string(pPrivate->pAppSettings, iParameter));

  GArray *pFilterArray = dasher_app_settings_get_allowed_values(pPrivate->pAppSettings, iParameter);

  GtkListStore *pStore = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER);
  gtk_tree_view_set_model(pView, GTK_TREE_MODEL(pStore));

  GtkCellRenderer *pRenderer;
  GtkTreeViewColumn *pColumn;
  
  pRenderer = gtk_cell_renderer_text_new();
  pColumn = gtk_tree_view_column_new_with_attributes(_("Action"), pRenderer, "text", 2, NULL);
  gtk_tree_view_append_column(pView, pColumn);

  GtkTreeIter oIter;
  //  GtkTreeIter oSelectedIter;
  GtkTreeSelection *pSelection = gtk_tree_view_get_selection(pView);

  GtkWidget **pHelperWindowRef = new GtkWidget *;

  for(unsigned int i(0); i < pFilterArray->len; ++i) {
    const gchar *szCurrentFilter = g_array_index(pFilterArray, gchar *, i);
    gchar *szName = new gchar[strlen(szCurrentFilter) + 1];
    strcpy(szName, szCurrentFilter);

    gtk_list_store_append(pStore, &oIter);
    GtkWidget *pHelperWindow;

    if(pHelper) {
      pHelperWindow = dasher_preferences_dialogue_get_helper(pSelf, iParameter, szName);
      g_signal_connect(G_OBJECT(pHelper), "clicked", G_CALLBACK(show_helper_window), pHelperWindowRef);
    }
    else
      pHelperWindow = NULL;

    // This is potentially horrible - maybe rethink in the future;
    gtk_list_store_set(pStore, &oIter, 0, iParameter, 1, pHelper, 2, szName, 3, szName, 4, pHelperWindow, 5, pHelperWindowRef, -1);

    delete[] szName;

    if(!strcmp(szCurrentFilter, szCurrentValue)) {
      gtk_tree_selection_select_iter(pSelection, &oIter);
      if(pHelper) {
        gtk_widget_set_sensitive(GTK_WIDGET(pHelper), pHelperWindow != NULL);
        *pHelperWindowRef = pHelperWindow;
      }
    }
  }

  g_signal_connect(pSelection, "changed", (GCallback)on_list_selection, 0);
}

// TODO: In class
extern "C" void on_list_selection(GtkTreeSelection *pSelection, gpointer pUserData) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix null

  GtkTreeIter oIter;
  GtkTreeModel *pModel;
  
  if(gtk_tree_selection_get_selected(pSelection, &pModel, &oIter)) {
    int iParameter;
    gpointer pHelper;
    gpointer pHelperWindow;
    gpointer pHelperWindowRef;
    gchar *szValue;
    gtk_tree_model_get(pModel, &oIter, 0, &iParameter, 1, &pHelper, 2, &szValue, 4, &pHelperWindow, 5, &pHelperWindowRef, -1);
    
    dasher_app_settings_set_string(pPrivate->pAppSettings, iParameter, szValue);
    g_free(szValue);

    if(pHelper) {
      gtk_widget_set_sensitive(GTK_WIDGET(pHelper), pHelperWindow != NULL);
      *((GtkWidget **)pHelperWindowRef) = (GtkWidget *)pHelperWindow;
    }
  }
}

GtkWidget *dasher_preferences_dialogue_get_helper(DasherPreferencesDialogue *pSelf, int iParameter, const gchar *szValue) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  SModuleSettings *pSettings;
  int iCount;

  if(!dasher_app_settings_get_module_settings(pPrivate->pAppSettings, szValue, &pSettings, &iCount))
    return NULL;

  return module_settings_window_new(pPrivate->pAppSettings, szValue, pSettings, iCount);
}

// --- Special Cases ---

static void dasher_preferences_dialogue_populate_special_speed(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  double dNewValue = dasher_app_settings_get_long(pPrivate->pAppSettings, LP_MAX_BITRATE) / 100.0;
  gtk_range_set_value(pPrivate->pSpeedSlider, dNewValue);
}

static void dasher_preferences_dialogue_populate_special_mouse_start(DasherPreferencesDialogue *pSelf) {
#ifndef WITH_MAEMO
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  pPrivate->pMousePosButton = GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "mouseposbutton"));
  pPrivate->pMousePosStyle = GTK_COMBO_BOX(gtk_builder_get_object(pPrivate->pXML, "MousePosStyle"));

  if(dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_MOUSEPOS_MODE)) {
    gtk_combo_box_set_active(pPrivate->pMousePosStyle, 1);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "mouseposbutton")), true);
  }
  else if(dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_CIRCLE_START)) {
    gtk_combo_box_set_active(pPrivate->pMousePosStyle, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "mouseposbutton")), true);
  }
  else {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "mouseposbutton")), false);
  }
#endif 
}

static void dasher_preferences_dialogue_populate_special_orientation(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  pPrivate->pLRButton = GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton2"));
  pPrivate->pRLButton = GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton3"));
  pPrivate->pTBButton = GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton4"));
  pPrivate->pBTButton = GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton5"));

  switch (dasher_app_settings_get_long(pPrivate->pAppSettings, LP_ORIENTATION)) {
  case Dasher::Opts::Alphabet:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton1"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton1")), TRUE);
    
    pPrivate->bIgnoreSignals = true;

    switch (dasher_app_settings_get_long(pPrivate->pAppSettings, LP_REAL_ORIENTATION)) {
    case Dasher::Opts::LeftToRight:
      if(gtk_toggle_button_get_active(pPrivate->pLRButton) != TRUE)
         gtk_toggle_button_set_active(pPrivate->pLRButton, TRUE);
      break;
    case Dasher::Opts::RightToLeft:
      if(gtk_toggle_button_get_active(pPrivate->pRLButton) != TRUE)
         gtk_toggle_button_set_active(pPrivate->pRLButton, TRUE);
      break;
    case Dasher::Opts::TopToBottom:
      if(gtk_toggle_button_get_active(pPrivate->pTBButton) != TRUE)
         gtk_toggle_button_set_active(pPrivate->pTBButton, TRUE);
      break;
    case Dasher::Opts::BottomToTop:
      if(gtk_toggle_button_get_active(pPrivate->pTBButton) != TRUE)
         gtk_toggle_button_set_active(pPrivate->pTBButton, TRUE);
      break;
    }

    pPrivate->bIgnoreSignals = false;

    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pLRButton), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pRLButton), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pTBButton), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pBTButton), FALSE);
    break;

  case Dasher::Opts::LeftToRight:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton2"))) != TRUE)
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton2")), TRUE);
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton12"))) != TRUE)
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton12")), TRUE);

    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pLRButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pRLButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pTBButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pBTButton), TRUE);
    break;

  case Dasher::Opts::RightToLeft:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton3"))) != TRUE)
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton3")), TRUE);
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton12"))) != TRUE)
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton12")), TRUE);

    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pLRButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pRLButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pTBButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pBTButton), TRUE);
    break;

  case Dasher::Opts::TopToBottom:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton4"))) != TRUE)
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton4")), TRUE);  
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton12"))) != TRUE)
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton12")), TRUE); 

    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pLRButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pRLButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pTBButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pBTButton), TRUE);
    break;

  case Dasher::Opts::BottomToTop:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton5"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton5")), TRUE);
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton12"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton12")), TRUE);

    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pLRButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pRLButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pTBButton), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pBTButton), TRUE);
    break;
  }
}

static void dasher_preferences_dialogue_populate_special_appstyle(DasherPreferencesDialogue *pSelf) {
#ifndef WITH_MAEMO  
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  switch(dasher_app_settings_get_long(pPrivate->pAppSettings, APP_LP_STYLE)) {
  case 0:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "appstyle_classic")), TRUE);
    break;
  case 1: 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "appstyle_compose")), TRUE);
    break;
  case 2:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "appstyle_direct")), TRUE);
    break;  
  case 3:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "appstyle_fullscreen")), TRUE);
    break;
  }
#endif
}

static void dasher_preferences_dialogue_populate_special_linewidth(DasherPreferencesDialogue *pSelf) {
#ifndef WITH_MAEMO
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  if(dasher_app_settings_get_long(pPrivate->pAppSettings, LP_LINE_WIDTH) > 1)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "thicklinebutton")), true);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "thicklinebutton")), false);
#endif
}


static void dasher_preferences_dialogue_populate_special_lm(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  switch( dasher_app_settings_get_long(pPrivate->pAppSettings, LP_LANGUAGE_MODEL_ID )) {
  case 0:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton6"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton6")), TRUE);
    break;
  case 2: 
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton7"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton7")), TRUE);
    break;
  case 3:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton8"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton8")), TRUE);
    break;
  case 4:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton9"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "radiobutton9")), TRUE);
    break;
  default:
    break;
  }
}

static void dasher_preferences_dialogue_populate_special_uniform(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  gtk_range_set_value( GTK_RANGE(gtk_builder_get_object(pPrivate->pXML, "uniformhscale")), dasher_app_settings_get_long(pPrivate->pAppSettings, LP_UNIFORM)/10.0);
}

static void dasher_preferences_dialogue_populate_special_colour(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "manual_colour")), !dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_PALETTE_CHANGE));
  gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(pPrivate->pXML, "ColorTree")), !dasher_app_settings_get_bool(pPrivate->pAppSettings, BP_PALETTE_CHANGE));
}

static void dasher_preferences_dialogue_populate_special_dasher_font(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  GObject *pDasherFontButton = gtk_builder_get_object(pPrivate->pXML, "dasher_fontbutton");

  gtk_font_button_set_font_name(GTK_FONT_BUTTON(pDasherFontButton), 
                                dasher_app_settings_get_string(pPrivate->pAppSettings, SP_DASHER_FONT));
}

static void dasher_preferences_dialogue_populate_special_edit_font(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  GObject *pEditFontButton = gtk_builder_get_object(pPrivate->pXML, "edit_fontbutton");

  gtk_font_button_set_font_name(GTK_FONT_BUTTON(pEditFontButton), 
                                dasher_app_settings_get_string(pPrivate->pAppSettings, APP_SP_EDIT_FONT));
}
 
static void dasher_preferences_dialogue_populate_special_fontsize(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);

  int iValue = dasher_app_settings_get_long(pPrivate->pAppSettings, LP_DASHER_FONTSIZE);
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "fontsizenormal")), iValue == Opts::Normal);
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "fontsizelarge")), iValue == Opts::Big);
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "fontsizevlarge")), iValue == Opts::VBig);
}

static void dasher_preferences_dialogue_update_special(DasherPreferencesDialogue *pSelf, int iID) {
  for(unsigned int i(0); i < (sizeof(sSpecialControlTable) / sizeof(SpecialControl)); ++i) {
    if(((iID == -1) && sSpecialControlTable[i].bPrimary) || (sSpecialControlTable[i].iID == iID)) {
      (sSpecialControlTable[i].pPopulate)(pSelf);
    }
  }
}

// --- Callbacks for 'special case' controls ---

// TODO: Give these a systematic naming convention
// TODO: Think about trying to combine OnMousePosStyleChanged and startonmousepos

extern "C" void OnMousePosStyleChanged(GtkWidget *widget, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  // FIXME - duplicate code from extern "C" void startonmousepos
  if((pPrivate->pMousePosButton)->active) {
    int iIndex;
    iIndex = gtk_combo_box_get_active(pPrivate->pMousePosStyle);
    
    if(iIndex == 1) {
      dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_MOUSEPOS_MODE, true);
      dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_CIRCLE_START, false);
    }
    else {
      dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_MOUSEPOS_MODE, false);
      dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_CIRCLE_START, true);
    }
  }
}

extern "C" void startonmousepos(GtkWidget *widget, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  if(GTK_TOGGLE_BUTTON(widget)->active) {
    int iIndex;
    iIndex = gtk_combo_box_get_active(pPrivate->pMousePosStyle);

    if(iIndex == 1) {
      dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_MOUSEPOS_MODE, true);
      dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_CIRCLE_START, false);
    }
    else {
      dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_MOUSEPOS_MODE, false);
      dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_CIRCLE_START, true);
    }
  }
  else {
    dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_MOUSEPOS_MODE, false);
    dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_CIRCLE_START, false);
  }
}

extern "C" void PrefsSpeedSliderChanged(GtkHScale *hscale, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL
  
  long iNewValue = long(round(gtk_range_get_value(GTK_RANGE(hscale)) * 100));
  dasher_app_settings_set_long(pPrivate->pAppSettings, LP_MAX_BITRATE, iNewValue);
}

extern "C" void orientation(GtkRadioButton *widget, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  if(pPrivate->bIgnoreSignals)
    return;

  // Again, this could be neater.
  if(GTK_TOGGLE_BUTTON(widget)->active == TRUE) {
    if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton1")) {
      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_ORIENTATION, Dasher::Opts::Alphabet);
      
      // FIXME - get rid of global variables here.

      gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pLRButton), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pRLButton), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pTBButton), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pBTButton), FALSE);

      pPrivate->bIgnoreSignals = true;
      
      switch (dasher_app_settings_get_long(pPrivate->pAppSettings, LP_REAL_ORIENTATION)) {
      case Dasher::Opts::LeftToRight:
        if(gtk_toggle_button_get_active(pPrivate->pLRButton) != TRUE)
           gtk_toggle_button_set_active(pPrivate->pLRButton, TRUE);
        break;
      case Dasher::Opts::RightToLeft:
        if(gtk_toggle_button_get_active(pPrivate->pRLButton) != TRUE)
           gtk_toggle_button_set_active(pPrivate->pRLButton, TRUE);
        break;
      case Dasher::Opts::TopToBottom:
        if(gtk_toggle_button_get_active(pPrivate->pTBButton) != TRUE)
           gtk_toggle_button_set_active(pPrivate->pTBButton, TRUE);
        break;
      case Dasher::Opts::BottomToTop:
        if(gtk_toggle_button_get_active(pPrivate->pTBButton) != TRUE)
           gtk_toggle_button_set_active(pPrivate->pTBButton, TRUE);
        break;
      }

      pPrivate->bIgnoreSignals = false;
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton12")) {
      gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pLRButton), TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pRLButton), TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pTBButton), TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(pPrivate->pBTButton), TRUE);

      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_ORIENTATION,
          dasher_app_settings_get_long(pPrivate->pAppSettings, LP_REAL_ORIENTATION));
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton2")) {
      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_ORIENTATION, Dasher::Opts::LeftToRight);
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton3")) {
      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_ORIENTATION, Dasher::Opts::RightToLeft);
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton4")) {
      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_ORIENTATION, Dasher::Opts::TopToBottom);
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton5")) {
      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_ORIENTATION, Dasher::Opts::BottomToTop);
    }
  }
}

extern "C" void ThickLineClicked(GtkWidget *widget, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  if(GTK_TOGGLE_BUTTON(widget)->active)
    dasher_app_settings_set_long(pPrivate->pAppSettings, LP_LINE_WIDTH, 3);
  else
    dasher_app_settings_set_long(pPrivate->pAppSettings, LP_LINE_WIDTH, 1);
}

extern "C" void autocolour_clicked(GtkWidget *widget, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  dasher_app_settings_set_bool(pPrivate->pAppSettings, BP_PALETTE_CHANGE, !GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void mouseposstart_y_changed(GtkRange *widget, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  int mouseposstartdist=int(widget->adjustment->value);
  dasher_app_settings_set_long(pPrivate->pAppSettings, LP_MOUSEPOSDIST, mouseposstartdist);
}

extern "C" void languagemodel(GtkRadioButton *widget, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  if (GTK_TOGGLE_BUTTON(widget)->active==TRUE) {
    if( !strcmp( gtk_widget_get_name( GTK_WIDGET(widget) ), "radiobutton6" ) ) {
      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_LANGUAGE_MODEL_ID, 0 );
    } else if (!strcmp( gtk_widget_get_name( GTK_WIDGET(widget) ), "radiobutton7" )) {
      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_LANGUAGE_MODEL_ID, 2 );
    } else if (!strcmp( gtk_widget_get_name( GTK_WIDGET(widget) ), "radiobutton8" )) {
      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_LANGUAGE_MODEL_ID, 3 );
    } else if (!strcmp( gtk_widget_get_name( GTK_WIDGET(widget) ), "radiobutton9" )) {
      dasher_app_settings_set_long(pPrivate->pAppSettings, LP_LANGUAGE_MODEL_ID, 4 );
    }
  }
}

extern "C" void uniform_changed(GtkHScale *hscale) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  
  int iValue = (int)(GTK_RANGE(hscale)->adjustment->value * 10);

  if(iValue < 50) {
    iValue = 50;
    gtk_range_set_value(GTK_RANGE(gtk_builder_get_object(pPrivate->pXML, "uniformhscale")), 5.0);
  }
  
  dasher_app_settings_set_long(pPrivate->pAppSettings, LP_UNIFORM, iValue);
}

extern "C" gboolean show_helper_window(GtkWidget *pWidget, gpointer *pUserData) {
  gtk_window_present(GTK_WINDOW(*pUserData));
  return FALSE;
}

extern "C" void on_appstyle_changed(GtkWidget *widget, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  if(GTK_TOGGLE_BUTTON(widget)->active) {
    if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "appstyle_classic"))
      dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_STYLE, 0);
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "appstyle_compose"))
      dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_STYLE, 1);
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "appstyle_direct"))
      dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_STYLE, 2);
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "appstyle_fullscreen"))
      dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_STYLE, 3);
  }
}

extern "C" void on_dasher_font_changed(GtkFontButton *pButton, gpointer pUserData) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue);

  dasher_app_settings_set_string(pPrivate->pAppSettings, 
                                 SP_DASHER_FONT, 
                                 gtk_font_button_get_font_name(pButton));
}

extern "C" void on_edit_font_changed(GtkFontButton *pButton, gpointer pUserData) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue);

  dasher_app_settings_set_string(pPrivate->pAppSettings, 
                                 APP_SP_EDIT_FONT, 
                                 gtk_font_button_get_font_name(pButton));
}

// --- Actions Selection ---

// Note - for now consider the actions configuration to be *really* a
// special case (more so than the systematic special cases), as it
// doesn't even make use of the integer IDs for parameters.

void dasher_preferences_dialogue_populate_actions(DasherPreferencesDialogue *pSelf) {
#ifndef WITH_MAEMO
  DasherPreferencesDialoguePrivate *pPrivate = (DasherPreferencesDialoguePrivate *)(pSelf->private_data);
  
  pPrivate->pListStore = gtk_list_store_new(ACTIONS_N_COLUMNS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);

//  GtkTreeIter oIter;

  // TODO: Reimplement this

//   dasher_editor_actions_start(pPrivate->pEditor);

//   while(dasher_editor_actions_more(pPrivate->pEditor)) {
//     gtk_list_store_append(pPrivate->pListStore, &oIter);

//     const gchar *szName;
//     gint iID;
//     gboolean bShow;
//     gboolean bControl;
//     gboolean bAuto;

//     dasher_editor_actions_get_next(pPrivate->pEditor, &szName, &iID, &bShow, &bControl, &bAuto),

//     gtk_list_store_set(pPrivate->pListStore, &oIter, 
//                        ACTIONS_ID_COLUMN, iID,
//                        ACTIONS_NAME_COLUMN, szName,
//                        ACTIONS_SHOW_COLUMN, bShow,
//                        ACTIONS_CONTROL_COLUMN, bControl,
//                        ACTIONS_AUTO_COLUMN, bAuto,
//                        -1);
//   }
  
  GtkCellRenderer *pRenderer;
  GtkTreeViewColumn *pColumn;
  
  // TODO: (small) memory leak here at the moment
  gint *pColumnIndex = new gint[3];
  pColumnIndex[0] = ACTIONS_SHOW_COLUMN;
  pColumnIndex[1] = ACTIONS_CONTROL_COLUMN;
  pColumnIndex[2] = ACTIONS_AUTO_COLUMN;

  pRenderer = gtk_cell_renderer_text_new();
  pColumn = gtk_tree_view_column_new_with_attributes(_("Action"), pRenderer, "text", ACTIONS_NAME_COLUMN, NULL);
  g_object_set(G_OBJECT(pColumn), "expand", true, NULL);
  gtk_tree_view_append_column(pPrivate->pActionTreeView, pColumn);

  pRenderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(pRenderer, "toggled", (GCallback)on_action_toggle, pColumnIndex);
  /* TRANSLATORS: Show a button for the selected action in the Dasher window. */
  pColumn = gtk_tree_view_column_new_with_attributes(_("Show Button"), pRenderer, "active", ACTIONS_SHOW_COLUMN, NULL);
  gtk_tree_view_append_column(pPrivate->pActionTreeView, pColumn);

  pRenderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(pRenderer, "toggled", (GCallback)on_action_toggle, pColumnIndex + 1);
  pColumn = gtk_tree_view_column_new_with_attributes(_("Control Mode"), pRenderer, "active", ACTIONS_CONTROL_COLUMN, NULL);
  gtk_tree_view_append_column(pPrivate->pActionTreeView, pColumn);

  pRenderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(pRenderer, "toggled", (GCallback)on_action_toggle, pColumnIndex + 2);
  /* TRANSLATORS: Automatically perform the selected action when Dasher is stopped. */
  pColumn = gtk_tree_view_column_new_with_attributes(_("Auto On Stop"), pRenderer, "active", ACTIONS_AUTO_COLUMN, NULL);
  gtk_tree_view_append_column(pPrivate->pActionTreeView, pColumn);

  gtk_tree_view_set_model(pPrivate->pActionTreeView, GTK_TREE_MODEL(pPrivate->pListStore));
#endif
}

extern "C" void on_action_toggle(GtkCellRendererToggle *pRenderer, gchar *szPath, gpointer pUserData) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  gint *pColumnIndex = (gint *)pUserData;

  GtkTreeIter oIter;
  gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(pPrivate->pListStore), &oIter, szPath);
  
  gboolean bSelected;
  gint iID;
  gtk_tree_model_get(GTK_TREE_MODEL(pPrivate->pListStore), &oIter, ACTIONS_ID_COLUMN, &iID, *pColumnIndex, &bSelected, -1);

  gtk_list_store_set(pPrivate->pListStore, &oIter, *pColumnIndex, !bSelected, -1);
  
  // TODO: reimplement

//   switch(*pColumnIndex) {
//   case ACTIONS_SHOW_COLUMN:
//     dasher_editor_internal_action_set_show(g_pEditor, iID, !bSelected);
//     break;
//   case ACTIONS_CONTROL_COLUMN:
//     dasher_editor_internal_action_set_control(g_pEditor, iID, !bSelected);
//     break;
//   case ACTIONS_AUTO_COLUMN:
//     dasher_editor_internal_action_set_auto(g_pEditor, iID, !bSelected);
//     break;
//   }
}

extern "C" void set_dasher_fontsize(GtkWidget *pWidget, gboolean pUserData) {
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue);

  int iValue = dasher_app_settings_get_long(pPrivate->pAppSettings, LP_DASHER_FONTSIZE);

  if((iValue != Opts::Normal) && gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "fontsizenormal"))))
    dasher_app_settings_set_long(pPrivate->pAppSettings, LP_DASHER_FONTSIZE, Opts::Normal);
  else if((iValue != Opts::Big) && gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "fontsizelarge"))))
    dasher_app_settings_set_long(pPrivate->pAppSettings, LP_DASHER_FONTSIZE, Opts::Big);
  else if((iValue != Opts::VBig) && gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(gtk_builder_get_object(pPrivate->pXML, "fontsizevlarge"))))
    dasher_app_settings_set_long(pPrivate->pAppSettings, LP_DASHER_FONTSIZE, Opts::VBig);
}


// --- General Callbacks ---

// For general purpose operation of the dialogue, rather than dealing
// with changing specific options

extern "C" gboolean preferences_hide(GtkWidget *widget, gpointer user_data) {
  //  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(pSelf);
  DasherPreferencesDialoguePrivate *pPrivate = DASHER_PREFERENCES_DIALOGUE_PRIVATE(g_pPreferencesDialogue); // TODO: Fix NULL

  gtk_widget_hide(GTK_WIDGET(pPrivate->pPreferencesWindow));
  return TRUE;
}


// --- TODOS:

#ifdef WITH_MAEMO
extern "C" void on_window_size_changed(GtkWidget *widget, gpointer user_data) {
  if(GTK_TOGGLE_BUTTON(widget)->active)
    dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_MAEMO_SIZE, 1);
  else
    dasher_app_settings_set_long(pPrivate->pAppSettings, APP_LP_MAEMO_SIZE, 0);
}
#endif

