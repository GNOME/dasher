#include "../Common/Common.h"
#include "../../config.h"

#include "Preferences.h"
#include "Parameters.h"
#include "dasher.h"
#include "config.h"
#include "GtkDasherControl.h"
#include "Menu.h"

#include "DasherControl.h"

#include "DasherTypes.h"
#include "module_settings_window.h"

#include <cstring>
#include <libintl.h>

// TODO: Reintroduce advanced settings and start on mouse position offset

typedef struct _BoolTranslation BoolTranslation;

struct _BoolTranslation {
  gint iParameter;
  gchar *szWidgetName;
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
  gchar *szWidgetName;
  GtkWidget *pWidget;
  gchar *szHelperName;
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

void RefreshWidget(gint iParameter);
extern "C" void RefreshParameter(GtkWidget *pWidget, gpointer pUserData);
void InitialiseTables(GladeXML *pGladeWidgets);
void dasher_preferences_populate_list(GtkTreeView *pView, int iParameter, GtkWidget *pHelper);
extern "C" void advanced_edited_callback(GtkCellRendererText * cell, gchar * path_string, gchar * new_text, gpointer user_data);
extern "C" void colour_select(GtkTreeSelection * selection, gpointer data);
extern "C" void on_action_toggle(GtkCellRendererToggle *pRenderer, gchar *szPath, gpointer pUserData);
extern "C" void on_list_selection(GtkTreeSelection *pSelection, gpointer pUserData);
extern "C" void on_widget_realize(GtkWidget *pWidget, gpointer pUserData);
extern "C" gboolean show_helper_window(GtkWidget *pWidget, gpointer *pUserData);

// Private member variables
GtkWidget *dasher_preferences_dialogue_get_helper(DasherPreferencesDialogue *pSelf, int iParameter, const gchar *szValue);

void populate_special_speed(GladeXML *pGladeWidgets);
void populate_special_mouse_start(GladeXML *pGladeWidgets);
void populate_special_orientation(GladeXML *pGladeWidgets);
void populate_special_appstyle(GladeXML *pGladeWidgets);
void populate_special_linewidth(GladeXML *pGladeWidgets);
void populate_special_lm(GladeXML *pGladeWidgets);
void populate_special_uniform(GladeXML *pGladeWidgets);
void populate_special_colour(GladeXML *pGladeWidgets);
void populate_special_dasher_font(GladeXML *pGladeWidgets);
void populate_special_edit_font(GladeXML *pGladeWidgets);
void populate_special_fontsize(GladeXML *pGladeWidgets);

typedef struct _SpecialControl SpecialControl;

struct _SpecialControl {
  int iID;
  void (*pPopulate)(GladeXML *);
  gboolean bPrimary;
};

SpecialControl sSpecialControlTable[] = {
  {LP_MAX_BITRATE, populate_special_speed, true},
  {BP_MOUSEPOS_MODE, populate_special_mouse_start, true},
  {BP_CIRCLE_START, populate_special_mouse_start, false},
  {LP_ORIENTATION, populate_special_orientation, true},
  {LP_REAL_ORIENTATION, populate_special_orientation, false},
  {APP_LP_STYLE, populate_special_appstyle, true},
  {LP_LINE_WIDTH, populate_special_linewidth, true},
  {LP_LANGUAGE_MODEL_ID, populate_special_lm, true},
  {LP_UNIFORM, populate_special_uniform, true},
  {BP_PALETTE_CHANGE, populate_special_colour, true},
  {SP_DASHER_FONT, populate_special_dasher_font, true},
  {APP_SP_EDIT_FONT, populate_special_edit_font, true}, 
  {LP_DASHER_FONTSIZE, populate_special_fontsize, true}
};

void update_special(int iID);


static GtkListStore *g_pStore;
static GladeXML *widgets;
static GtkWidget *preferences_window;
static GtkWidget *m_pLRButton;
static GtkWidget *m_pRLButton;
static GtkWidget *m_pTBButton;
static GtkWidget *m_pBTButton;
static GtkWidget *m_pSpeedSlider;
static GtkWidget *m_pMousePosButton;
static GtkWidget *m_pMousePosStyle;

// Set this to ignore signals (ie loops coming back from setting widgets in response to parameters having changed)
static bool g_bIgnoreSignals(false);

#define _(_x) gettext(_x)

// TODO: Look at coding convention stuff for gobjets

/// Newer, object based stuff

struct _DasherPreferencesDialoguePrivate {
  GtkWindow *pWindow;
  DasherEditor *pEditor;
  GtkWidget *pActionTreeView;
};

typedef struct _DasherPreferencesDialoguePrivate DasherPreferencesDialoguePrivate;

// Private member functions
static void dasher_preferences_dialogue_class_init(DasherPreferencesDialogueClass *pClass);
static void dasher_preferences_dialogue_init(DasherPreferencesDialogue *pMain);
static void dasher_preferences_dialogue_destroy(GObject *pObject);

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
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

// Public methods

DasherPreferencesDialogue *dasher_preferences_dialogue_new(GladeXML *pGladeWidgets, DasherEditor *pEditor) {
  DasherPreferencesDialogue *pDasherControl;
  pDasherControl = (DasherPreferencesDialogue *)(g_object_new(dasher_preferences_dialogue_get_type(), NULL));
  DasherPreferencesDialoguePrivate *pPrivate = (DasherPreferencesDialoguePrivate *)(pDasherControl->private_data);

  pPrivate->pEditor = pEditor;

  widgets = pGladeWidgets;

  preferences_window = glade_xml_get_widget(pGladeWidgets, "preferences");
  pPrivate->pWindow = GTK_WINDOW(preferences_window);

  pPrivate->pActionTreeView =glade_xml_get_widget(pGladeWidgets, "action_tree_view");

  gtk_window_set_transient_for(GTK_WINDOW(preferences_window), GTK_WINDOW(window));

  m_pSpeedSlider = glade_xml_get_widget(pGladeWidgets, "hscale1");
  InitialiseTables(pGladeWidgets);
  RefreshWidget(-1);
  update_special(-1); // TODO: Make this a class member

#ifndef JAPANESE
  gtk_widget_hide(glade_xml_get_widget(pGladeWidgets, "radiobutton9"));
#endif

  return pDasherControl;
}

void dasher_preferences_dialogue_show(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = (DasherPreferencesDialoguePrivate *)(pSelf->private_data);
  // FIXME - REIMPLEMENT

  // Keep the preferences window in the correct position relative to the
  // main Dasher window
  
  gtk_window_set_transient_for(pPrivate->pWindow,GTK_WINDOW(window));
#ifdef WITH_MAEMO
#ifndef WITH_MAEMOFULLSCREEN
  gtk_window_set_keep_above((pPrivate->pWindow), true);
#endif
#endif
  gtk_window_set_keep_above((pPrivate->pWindow), dasher_main_topmost(g_pDasherMain));
  gtk_window_present(pPrivate->pWindow);
}


void dasher_preferences_dialogue_handle_parameter_change(DasherPreferencesDialogue *pSelf, int iParameter) {
  RefreshWidget(iParameter);
  update_special(iParameter);
}


// --- Generic Options ---

void InitialiseTables(GladeXML *pGladeWidgets) {
#ifndef WITH_MAEMO
  int iNumBoolEntries = sizeof(sBoolTranslationTable) / sizeof(BoolTranslation);
  for(int i(0); i < iNumBoolEntries; ++i) {
    sBoolTranslationTable[i].pWidget = glade_xml_get_widget(pGladeWidgets, sBoolTranslationTable[i].szWidgetName);
  }
#endif

  int iNumStringEntries = sizeof(sStringTranslationTable) / sizeof(StringTranslation);
  for(int i(0); i < iNumStringEntries; ++i) {
    sStringTranslationTable[i].pWidget = glade_xml_get_widget(pGladeWidgets, sStringTranslationTable[i].szWidgetName);
    if(sStringTranslationTable[i].szHelperName)
      sStringTranslationTable[i].pHelper = glade_xml_get_widget(pGladeWidgets, sStringTranslationTable[i].szHelperName);

    dasher_preferences_populate_list(GTK_TREE_VIEW(sStringTranslationTable[i].pWidget), sStringTranslationTable[i].iParameter, sStringTranslationTable[i].pHelper);
    g_signal_connect(sStringTranslationTable[i].pWidget, "realize", (GCallback)on_widget_realize, &sStringTranslationTable[i].iParameter);
  }
}

extern "C" gboolean refresh_foreach_function(GtkTreeModel *pModel, GtkTreePath *pPath, GtkTreeIter *pIter, gpointer pUserData) {
  
  gpointer *pPointers = (gpointer *)pUserData;

  gchar *szTarget = (gchar *)pPointers[0];
  gchar *szComparison;
  gtk_tree_model_get(pModel, pIter, 2, &szComparison, -1);

  if(!strcmp(szTarget, szComparison)) {
    // Todo: set selection here?
    gtk_tree_view_set_cursor((GtkTreeView *)pPointers[1], pPath, NULL, false);

    gtk_tree_view_scroll_to_cell((GtkTreeView *)pPointers[1], pPath, NULL, true, 0.5, 0.0);

    return true;
  }
  
  return false;
}

void RefreshWidget(gint iParameter) {
#ifndef WITH_MAEMO
  int iNumBoolEntries = sizeof(sBoolTranslationTable) / sizeof(BoolTranslation);
  for(int i(0); i < iNumBoolEntries; ++i) {
    if((iParameter == -1) || (sBoolTranslationTable[i].iParameter == iParameter)) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sBoolTranslationTable[i].pWidget), dasher_app_settings_get_bool(g_pDasherAppSettings, sBoolTranslationTable[i].iParameter));
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

      const void *pUserData[2];
      pUserData[0] = dasher_app_settings_get_string(g_pDasherAppSettings, sStringTranslationTable[i].iParameter);
      pUserData[1] = GTK_TREE_VIEW(sStringTranslationTable[i].pWidget);
      
      if(sStringTranslationTable[i].pWidget && GTK_WIDGET_REALIZED(sStringTranslationTable[i].pWidget))
	gtk_tree_model_foreach(pModel, refresh_foreach_function, pUserData);
    }
  }
}

extern "C" void RefreshParameter(GtkWidget *pWidget, gpointer pUserData) {
#ifndef WITH_MAEMO
  int iNumBoolEntries = sizeof(sBoolTranslationTable) / sizeof(BoolTranslation);
  
  for(int i(0); i < iNumBoolEntries; ++i) {
    if((pWidget == NULL) || (sBoolTranslationTable[i].pWidget == pWidget)) {
      if(GTK_TOGGLE_BUTTON(sBoolTranslationTable[i].pWidget)->active != dasher_app_settings_get_bool(g_pDasherAppSettings, sBoolTranslationTable[i].iParameter)) {
	dasher_app_settings_set_bool(g_pDasherAppSettings, sBoolTranslationTable[i].iParameter, GTK_TOGGLE_BUTTON(sBoolTranslationTable[i].pWidget)->active);
      }
    }
  }
#endif
}

// TODO: Is this function actually useful? (conversely, is the other call to RefreshFoo elsewhere any use?)
extern "C" void on_widget_realize(GtkWidget *pWidget, gpointer pUserData) {
  gint *pParameter = (gint *)pUserData;
  RefreshWidget(*pParameter);
}

// --- Generic boolean options ---

extern "C" void generic_bool_changed(GtkWidget *widget, gpointer user_data) {
  RefreshParameter(widget, user_data);
}

// --- Generic string options ---

void dasher_preferences_populate_list(GtkTreeView *pView, int iParameter, GtkWidget *pHelper) {
  // TODO: Need to kill helpers on list depopulation

  const gchar *szCurrentValue(gtk_dasher_control_get_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter));

  GArray *pFilterArray = gtk_dasher_control_get_allowed_values(GTK_DASHER_CONTROL(pDasherWidget), iParameter);

  GtkListStore *pStore = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER);
  gtk_tree_view_set_model(pView, GTK_TREE_MODEL(pStore));

  GtkCellRenderer *pRenderer;
  GtkTreeViewColumn *pColumn;
  
  pRenderer = gtk_cell_renderer_text_new();
  pColumn = gtk_tree_view_column_new_with_attributes("Action", pRenderer, "text", 2, NULL);
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
      pHelperWindow = dasher_preferences_dialogue_get_helper(g_pPreferencesDialogue, iParameter, szName);
      g_signal_connect(G_OBJECT(pHelper), "clicked", G_CALLBACK(show_helper_window), pHelperWindowRef);
    }
    else
      pHelperWindow = NULL;

    // This is potentially horrible - maybe rethink in the future;
    gtk_list_store_set(pStore, &oIter, 0, iParameter, 1, pHelper, 2, szName, 3, szName, 4, pHelperWindow, 5, pHelperWindowRef, -1);
 
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

extern "C" void on_list_selection(GtkTreeSelection *pSelection, gpointer pUserData) {
  GtkTreeIter oIter;
  GtkTreeModel *pModel;
  
  if(gtk_tree_selection_get_selected(pSelection, &pModel, &oIter)) {
    int iParameter;
    gpointer pHelper;
    gpointer pHelperWindow;
    gpointer pHelperWindowRef;
    gchar *szValue;
    gtk_tree_model_get(pModel, &oIter, 0, &iParameter, 1, &pHelper, 2, &szValue, 4, &pHelperWindow, 5, &pHelperWindowRef, -1);
    
    gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter, szValue);
    
    if(pHelper) {
      gtk_widget_set_sensitive(GTK_WIDGET(pHelper), pHelperWindow != NULL);
      *((GtkWidget **)pHelperWindowRef) = (GtkWidget *)pHelperWindow;
    }
  }
}

GtkWidget *dasher_preferences_dialogue_get_helper(DasherPreferencesDialogue *pSelf, int iParameter, const gchar *szValue) {
  SModuleSettings *pSettings;
  int iCount;

  if(!gtk_dasher_control_get_module_settings(GTK_DASHER_CONTROL(pDasherWidget), szValue, &pSettings, &iCount))
    return NULL;

  return module_settings_window_new(g_pDasherAppSettings, szValue, pSettings, iCount);
}

// --- Special Cases ---

void populate_special_speed(GladeXML *pGladeWidgets) {
  double dNewValue = dasher_app_settings_get_long(g_pDasherAppSettings, LP_MAX_BITRATE) / 100.0;
  gtk_range_set_value(GTK_RANGE(m_pSpeedSlider), dNewValue); 
}

void populate_special_mouse_start(GladeXML *pGladeWidgets) {
#ifndef WITH_MAEMO
  m_pMousePosButton = glade_xml_get_widget(pGladeWidgets, "mouseposbutton");
  m_pMousePosStyle = glade_xml_get_widget(pGladeWidgets, "MousePosStyle");

  if(dasher_app_settings_get_bool(g_pDasherAppSettings, BP_MOUSEPOS_MODE)) {
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_pMousePosStyle), 1);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "mouseposbutton")), true);
  }
  else if(dasher_app_settings_get_bool(g_pDasherAppSettings, BP_CIRCLE_START)) {
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_pMousePosStyle), 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "mouseposbutton")), true);
  }
  else {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "mouseposbutton")), false);
  }
#endif 
}

void populate_special_orientation(GladeXML *pGladeWidgets) {
  m_pLRButton = glade_xml_get_widget(pGladeWidgets, "radiobutton2");
  m_pRLButton = glade_xml_get_widget(pGladeWidgets, "radiobutton3");
  m_pTBButton = glade_xml_get_widget(pGladeWidgets, "radiobutton4");
  m_pBTButton = glade_xml_get_widget(pGladeWidgets, "radiobutton5");

  switch (getLong(LP_ORIENTATION)) {
  case Dasher::Opts::Alphabet:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton1"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton1")), TRUE); 
    
    g_bIgnoreSignals = true;

    switch (getLong(LP_REAL_ORIENTATION)) {
    case Dasher::Opts::LeftToRight:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pLRButton)) != TRUE)
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pLRButton), TRUE);
      break;
    case Dasher::Opts::RightToLeft:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pRLButton)) != TRUE)
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pRLButton), TRUE);
      break;
    case Dasher::Opts::TopToBottom:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pTBButton)) != TRUE)
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pTBButton), TRUE);
      break;
    case Dasher::Opts::BottomToTop:
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pTBButton)) != TRUE)
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pTBButton), TRUE);
      break;
    }

    g_bIgnoreSignals = false;

    gtk_widget_set_sensitive(m_pLRButton, FALSE);
    gtk_widget_set_sensitive(m_pRLButton, FALSE);
    gtk_widget_set_sensitive(m_pTBButton, FALSE);
    gtk_widget_set_sensitive(m_pBTButton, FALSE);


    break;
  case Dasher::Opts::LeftToRight:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton2"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton2")), TRUE); 
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton12"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton12")), TRUE); 

    gtk_widget_set_sensitive(m_pLRButton, TRUE);
    gtk_widget_set_sensitive(m_pRLButton, TRUE);
    gtk_widget_set_sensitive(m_pTBButton, TRUE);
    gtk_widget_set_sensitive(m_pBTButton, TRUE);
    break;
  case Dasher::Opts::RightToLeft:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton3"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton3")), TRUE); 
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton12"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton12")), TRUE); 

    gtk_widget_set_sensitive(m_pLRButton, TRUE);
    gtk_widget_set_sensitive(m_pRLButton, TRUE);
    gtk_widget_set_sensitive(m_pTBButton, TRUE);
    gtk_widget_set_sensitive(m_pBTButton, TRUE);
    break;
  case Dasher::Opts::TopToBottom:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton4"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton4")), TRUE);  
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton12"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton12")), TRUE); 

    gtk_widget_set_sensitive(m_pLRButton, TRUE);
    gtk_widget_set_sensitive(m_pRLButton, TRUE);
    gtk_widget_set_sensitive(m_pTBButton, TRUE);
    gtk_widget_set_sensitive(m_pBTButton, TRUE);
    break;
  case Dasher::Opts::BottomToTop:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton5"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton5")), TRUE); 
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton12"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton12")), TRUE); 

    gtk_widget_set_sensitive(m_pLRButton, TRUE);
    gtk_widget_set_sensitive(m_pRLButton, TRUE);
    gtk_widget_set_sensitive(m_pTBButton, TRUE);
    gtk_widget_set_sensitive(m_pBTButton, TRUE);
    break;
  }
}

void populate_special_appstyle(GladeXML *pGladeWidgets) {
#ifndef WITH_MAEMO  
  switch(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE)) {
  case 0:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "appstyle_classic")), TRUE);
    break;
  case 1: 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "appstyle_compose")), TRUE);
    break;
  case 2:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "appstyle_direct")), TRUE);
    break;   
  case 3:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "appstyle_fullscreen")), TRUE);
    break;
  }
#endif
}

void populate_special_linewidth(GladeXML *pGladeWidgets) {
#ifndef WITH_MAEMO
  if(getLong(LP_LINE_WIDTH) > 1)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "thicklinebutton")), true);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "thicklinebutton")), false);
#endif
}


void populate_special_lm(GladeXML *pGladeWidgets) {
  switch( gtk_dasher_control_get_parameter_long( GTK_DASHER_CONTROL(pDasherWidget), LP_LANGUAGE_MODEL_ID )) {
  case 0:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton6"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton6")), TRUE);
    break;
  case 2: 
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton7"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton7")), TRUE);
    break;
  case 3:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton8"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton8")), TRUE);
    break;
  case 4:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton9"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton9")), TRUE);
    break;
  default:
    break;
  }
}

void populate_special_uniform(GladeXML *pGladeWidgets) {
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "uniformhscale")), getLong(LP_UNIFORM)/10.0);
}

void populate_special_colour(GladeXML *pGladeWidgets) {
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "manual_colour")), !getBool(BP_PALETTE_CHANGE)); 
  gtk_widget_set_sensitive(glade_xml_get_widget(pGladeWidgets, "ColorTree"), !getBool(BP_PALETTE_CHANGE));
}


void populate_special_dasher_font(GladeXML *pGladeWidgets) {
  GtkWidget *pDasherFontButton = glade_xml_get_widget(pGladeWidgets, "dasherfontbutton");
  PangoFontDescription *pFont = pango_font_description_from_string(dasher_app_settings_get_string(g_pDasherAppSettings, SP_DASHER_FONT));
  gtk_widget_modify_font(pDasherFontButton, pFont);
  gtk_button_set_label(GTK_BUTTON(pDasherFontButton), dasher_app_settings_get_string(g_pDasherAppSettings, SP_DASHER_FONT));
}

void populate_special_edit_font(GladeXML *pGladeWidgets) {
  GtkWidget *pEditFontButton = glade_xml_get_widget(pGladeWidgets, "editfontbutton");
  gtk_button_set_label(GTK_BUTTON(pEditFontButton), dasher_app_settings_get_string(g_pDasherAppSettings, APP_SP_EDIT_FONT));
}
 
void populate_special_fontsize(GladeXML *pGladeWidgets) {
  int iValue = gtk_dasher_control_get_parameter_long( GTK_DASHER_CONTROL(pDasherWidget), LP_DASHER_FONTSIZE);
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "fontsizenormal")), iValue == Opts::Normal);
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "fontsizelarge")), iValue == Opts::Big);
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "fontsizevlarge")), iValue == Opts::VBig);
}

void update_special(int iID) {
  for(unsigned int i(0); i < (sizeof(sSpecialControlTable) / sizeof(SpecialControl)); ++i) {
    if(((iID == -1) && sSpecialControlTable[i].bPrimary) || (sSpecialControlTable[i].iID == iID)) {
      (sSpecialControlTable[i].pPopulate)(widgets);
    }
  }
}

// --- Callbacks for 'special case' controls ---

// TODO: Give these a systematic naming convention
// TODO: Think about trying to combine OnMousePosStyleChanged and startonmousepos

extern "C" void OnMousePosStyleChanged(GtkWidget *widget, gpointer user_data) {
  // FIXME - duplicate code from extern "C" void startonmousepos
  if(GTK_TOGGLE_BUTTON(m_pMousePosButton)->active) {
    int iIndex;
    iIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(m_pMousePosStyle));
    
    if(iIndex == 1) {
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_MOUSEPOS_MODE, true);
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_CIRCLE_START, false);
    }
    else {
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_MOUSEPOS_MODE, false);
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_CIRCLE_START, true);
    }
  }
}

extern "C" void startonmousepos(GtkWidget *widget, gpointer user_data) {
  if(GTK_TOGGLE_BUTTON(widget)->active) {
    int iIndex;
    iIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(m_pMousePosStyle));

    if(iIndex == 1) {
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_MOUSEPOS_MODE, true);
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_CIRCLE_START, false);
    }
    else {
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_MOUSEPOS_MODE, false);
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_CIRCLE_START, true);
    }
  }
  else {
    gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_MOUSEPOS_MODE, false);
    gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_CIRCLE_START, false);
  }
}

extern "C" void PrefsSpeedSliderChanged(GtkHScale *hscale, gpointer user_data) {
    long iNewValue = long(round(gtk_range_get_value(GTK_RANGE(hscale)) * 100));
    dasher_app_settings_set_long(g_pDasherAppSettings, LP_MAX_BITRATE, iNewValue);
}

extern "C" void orientation(GtkRadioButton *widget, gpointer user_data) {

  if(g_bIgnoreSignals)
    return;

  // Again, this could be neater.
  if(GTK_TOGGLE_BUTTON(widget)->active == TRUE) {
    if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton1")) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ORIENTATION, Dasher::Opts::Alphabet);
      
      // FIXME - get rid of global variables here.

      gtk_widget_set_sensitive(m_pLRButton, FALSE);
      gtk_widget_set_sensitive(m_pRLButton, FALSE);
      gtk_widget_set_sensitive(m_pTBButton, FALSE);
      gtk_widget_set_sensitive(m_pBTButton, FALSE);


      g_bIgnoreSignals = true;
      
      switch (getLong(LP_REAL_ORIENTATION)) {
      case Dasher::Opts::LeftToRight:
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pLRButton)) != TRUE)
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pLRButton), TRUE);
	break;
      case Dasher::Opts::RightToLeft:
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pRLButton)) != TRUE)
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pRLButton), TRUE);
	break;
      case Dasher::Opts::TopToBottom:
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pTBButton)) != TRUE)
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pTBButton), TRUE);
	break;
      case Dasher::Opts::BottomToTop:
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pTBButton)) != TRUE)
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pTBButton), TRUE);
	break;
      }

      g_bIgnoreSignals = false;
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton12")) {
      gtk_widget_set_sensitive(m_pLRButton, TRUE);
      gtk_widget_set_sensitive(m_pRLButton, TRUE);
      gtk_widget_set_sensitive(m_pTBButton, TRUE);
      gtk_widget_set_sensitive(m_pBTButton, TRUE);

      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ORIENTATION,  getLong(LP_REAL_ORIENTATION));
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton2")) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ORIENTATION, Dasher::Opts::LeftToRight);
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton3")) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ORIENTATION, Dasher::Opts::RightToLeft);
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton4")) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ORIENTATION, Dasher::Opts::TopToBottom);
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "radiobutton5")) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ORIENTATION, Dasher::Opts::BottomToTop);
    }
  }
}

extern "C" void ThickLineClicked(GtkWidget *widget, gpointer user_data) {
  if(GTK_TOGGLE_BUTTON(widget)->active)
    gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_LINE_WIDTH, 3);
  else
    gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_LINE_WIDTH, 1);
}

extern "C" void autocolour_clicked(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_PALETTE_CHANGE, !GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void mouseposstart_y_changed(GtkRange *widget, gpointer user_data) {
  int mouseposstartdist=int(widget->adjustment->value);
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_MOUSEPOSDIST, mouseposstartdist);
}

extern "C" void languagemodel(GtkRadioButton *widget, gpointer user_data) {
  if (GTK_TOGGLE_BUTTON(widget)->active==TRUE) {
    if( !strcmp( gtk_widget_get_name( GTK_WIDGET(widget) ), "radiobutton6" ) ) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget),  LP_LANGUAGE_MODEL_ID, 0 );
    } else if (!strcmp( gtk_widget_get_name( GTK_WIDGET(widget) ), "radiobutton7" )) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget),  LP_LANGUAGE_MODEL_ID, 2 );
    } else if (!strcmp( gtk_widget_get_name( GTK_WIDGET(widget) ), "radiobutton8" )) {
      gtk_dasher_control_set_parameter_long( GTK_DASHER_CONTROL(pDasherWidget), LP_LANGUAGE_MODEL_ID, 3 );
    } else if (!strcmp( gtk_widget_get_name( GTK_WIDGET(widget) ), "radiobutton9" )) {
      gtk_dasher_control_set_parameter_long( GTK_DASHER_CONTROL(pDasherWidget), LP_LANGUAGE_MODEL_ID, 4 );
    }
  }
}

extern "C" void uniform_changed(GtkHScale *hscale) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_UNIFORM, int (GTK_RANGE(hscale)->adjustment->value * 10));
}

extern "C" gboolean show_helper_window(GtkWidget *pWidget, gpointer *pUserData) {
  gtk_window_present(GTK_WINDOW(*pUserData));
  return FALSE;
}

extern "C" void on_appstyle_changed(GtkWidget *widget, gpointer user_data) {
  if(GTK_TOGGLE_BUTTON(widget)->active) {
    if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "appstyle_classic"))
      dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_STYLE, 0);
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "appstyle_compose"))
      dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_STYLE, 1);
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "appstyle_direct"))
      dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_STYLE, 2);
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "appstyle_fullscreen"))
      dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_STYLE, 3);
  }
}

// --- Actions Selection ---

// Note - for now consider the actions configuration to be *really* a
// special case (more so than the systematic special cases), as it
// doesn't even make use of the integer IDs for parameters.

void dasher_preferences_dialogue_populate_actions(DasherPreferencesDialogue *pSelf) {
#ifndef WITH_MAEMO
  DasherPreferencesDialoguePrivate *pPrivate = (DasherPreferencesDialoguePrivate *)(pSelf->private_data);
  
  g_pStore = gtk_list_store_new(ACTIONS_N_COLUMNS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);

  GtkTreeIter oIter;

  dasher_editor_actions_start(pPrivate->pEditor);

  while(dasher_editor_actions_more(pPrivate->pEditor)) {
    gtk_list_store_append(g_pStore, &oIter);

    const gchar *szName;
    gint iID;
    gboolean bShow;
    gboolean bControl;
    gboolean bAuto;

    dasher_editor_actions_get_next(pPrivate->pEditor, &szName, &iID, &bShow, &bControl, &bAuto),

    gtk_list_store_set(g_pStore, &oIter, 
		       ACTIONS_ID_COLUMN, iID,
		       ACTIONS_NAME_COLUMN, szName,
		       ACTIONS_SHOW_COLUMN, bShow,
		       ACTIONS_CONTROL_COLUMN, bControl,
		       ACTIONS_AUTO_COLUMN, bAuto,
		       -1);
  }
  
  GtkCellRenderer *pRenderer;
  GtkTreeViewColumn *pColumn;
  
  // TODO: (small) memory leak here at the moment
  gint *pColumnIndex = new gint[3];
  pColumnIndex[0] = ACTIONS_SHOW_COLUMN;
  pColumnIndex[1] = ACTIONS_CONTROL_COLUMN;
  pColumnIndex[2] = ACTIONS_AUTO_COLUMN;

  pRenderer = gtk_cell_renderer_text_new();
  pColumn = gtk_tree_view_column_new_with_attributes("Action", pRenderer, "text", ACTIONS_NAME_COLUMN, NULL);
  g_object_set(G_OBJECT(pColumn), "expand", true, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pPrivate->pActionTreeView), pColumn);

  pRenderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(pRenderer, "toggled", (GCallback)on_action_toggle, pColumnIndex);
  pColumn = gtk_tree_view_column_new_with_attributes("Show Button", pRenderer, "active", ACTIONS_SHOW_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pPrivate->pActionTreeView), pColumn);

  pRenderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(pRenderer, "toggled", (GCallback)on_action_toggle, pColumnIndex + 1);
  pColumn = gtk_tree_view_column_new_with_attributes("Control Mode", pRenderer, "active", ACTIONS_CONTROL_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pPrivate->pActionTreeView), pColumn);

  pRenderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(pRenderer, "toggled", (GCallback)on_action_toggle, pColumnIndex + 2);
  pColumn = gtk_tree_view_column_new_with_attributes("Auto On Stop", pRenderer, "active", ACTIONS_AUTO_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pPrivate->pActionTreeView), pColumn);

  gtk_tree_view_set_model(GTK_TREE_VIEW(pPrivate->pActionTreeView), GTK_TREE_MODEL(g_pStore));
#endif
}

extern "C" void on_action_toggle(GtkCellRendererToggle *pRenderer, gchar *szPath, gpointer pUserData) {
  gint *pColumnIndex = (gint *)pUserData;

  GtkTreeIter oIter;
  gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(g_pStore), &oIter, szPath);
  
  gboolean bSelected;
  gint iID;
  gtk_tree_model_get(GTK_TREE_MODEL(g_pStore), &oIter, ACTIONS_ID_COLUMN, &iID, *pColumnIndex, &bSelected, -1);

  gtk_list_store_set(g_pStore, &oIter, *pColumnIndex, !bSelected, -1);
  
  switch(*pColumnIndex) {
  case ACTIONS_SHOW_COLUMN:
    dasher_editor_action_set_show(g_pEditor, iID, !bSelected);
    break;
  case ACTIONS_CONTROL_COLUMN:
    dasher_editor_action_set_control(g_pEditor, iID, !bSelected);
    break;
  case ACTIONS_AUTO_COLUMN:
    dasher_editor_action_set_auto(g_pEditor, iID, !bSelected);
    break;
  }
}


// --- General Callbacks ---

// For general purpose operation of the dialogue, rather than dealing
// with changing specific options

extern "C" gboolean preferences_hide(GtkWidget *widget, gpointer user_data) {
  gtk_widget_hide(preferences_window);
  return TRUE;
}


// --- TODOS:

#ifdef WITH_MAEMO
extern "C" void on_window_size_changed(GtkWidget *widget, gpointer user_data) {
  if(GTK_TOGGLE_BUTTON(widget)->active)
    dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_MAEMO_SIZE, 1);
  else
    dasher_app_settings_set_long(g_pDasherAppSettings, APP_LP_MAEMO_SIZE, 0);
}
#endif

