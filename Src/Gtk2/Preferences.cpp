#include "../Common/Common.h"

#include "Preferences.h"
#include "Parameters.h"
#include "dasher.h"
#include "config.h"
#include "GtkDasherControl.h"

#include "DasherControl.h"

#include "DasherTypes.h"

#include <cstring>
#include <libintl.h>


GtkListStore *g_pStore;

// TODO: Get rid of this as soon as possible
GladeXML *widgets;

typedef struct _BoolTranslation BoolTranslation;

struct _BoolTranslation {
  gint iParameter;
  gchar *szWidgetName;
  GtkWidget *pWidget;
};

typedef struct _StringTranslation StringTranslation;

struct _StringTranslation {
  gint iParameter;
  gchar *szWidgetName;
  GtkWidget *pWidget;
  gchar *szHelperName;
  GtkWidget *pHelper;
};

// Checkbox widgets which map directly to boolean parameters

BoolTranslation sBoolTranslationTable[] = {
  {BP_DRAW_MOUSE_LINE, "showmouselinebutton", NULL},
  {BP_DRAW_MOUSE, "showmousebutton", NULL},
  {BP_SHOW_SLIDER, "speedsliderbutton", NULL},
  {APP_BP_SHOW_TOOLBAR, "toolbarbutton", NULL},
  {BP_OUTLINE_MODE, "outlinebutton", NULL},
  {BP_CONTROL_MODE, "control_controlmode", NULL},
  {APP_BP_TIME_STAMP, "timestampbutton", NULL},
  {BP_AUTOCALIBRATE, "autocalibrate", NULL},
  {BP_GLOBAL_KEYBOARD, "keyboardgrab", NULL}
};

// List widgets which map directly to string parameters

StringTranslation sStringTranslationTable[] = {
  {SP_ALPHABET_ID, "AlphabetTree", NULL, NULL, NULL},
  {SP_COLOUR_ID, "ColorTree", NULL, NULL, NULL},
  {SP_INPUT_FILTER, "input_filter_tree_view", NULL, "button13", NULL},
  {SP_INPUT_DEVICE, "input_tree_view", NULL, "button25", NULL}
};

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
void PopulateLMPage(GladeXML * pGladeWidgets);
void generate_preferences(GladeXML * pGladeWidgets);
void PopulateControlPage(GladeXML * pGladeWidgets);
void PopulateViewPage(GladeXML * pGladeWidgets);
void PopulateButtonsPage(GladeXML * pGladeWidgets);
void PopulateSocketPage(GladeXML * pGladeWidgets);
void PopulateAdvancedPage(GladeXML *pGladeWidgets);

//void SetAlphabetSelection(int i, GtkTreeIter *pAlphIter);
void dasher_preferences_populate_list(GtkTreeView *pView, int iParameter, GtkWidget *pHelper);

extern "C" void advanced_edited_callback(GtkCellRendererText * cell, gchar * path_string, gchar * new_text, gpointer user_data);
extern "C" void colour_select(GtkTreeSelection * selection, gpointer data);
extern "C" void alphabet_select(GtkTreeSelection * selection, gpointer data);
extern "C" void on_action_toggle(GtkCellRendererToggle *pRenderer, gchar *szPath, gpointer pUserData);
extern "C" void on_list_selection(GtkTreeSelection *pSelection, gpointer pUserData);
extern "C" void on_widget_realize(GtkWidget *pWidget, gpointer pUserData);

// Private member variables
GtkWidget *dasher_preferences_dialogue_get_helper(DasherPreferencesDialogue *pSelf, int iParameter, const gchar *szValue);

GtkTreeSelection *alphselection;
GtkWidget *alphabettreeview;
GtkListStore *alph_list_store;

GtkTreeModel *m_pAdvancedModel;

GtkWidget *preferences_window;
GtkWidget *train_dialogue;

GtkWidget *m_pLRButton;
GtkWidget *m_pRLButton;
GtkWidget *m_pTBButton;
GtkWidget *m_pBTButton;
GtkWidget *m_pSpeedSlider;

GtkWidget *m_pButtonSettings;
GtkWidget *m_pSocketSettings;
GtkWidget *m_pAdvancedSettings;

GtkWidget *m_pMousePosButton;
GtkWidget *m_pMousePosStyle;

// Set this to ignore signals (ie loops coming back from setting widgets in response to parameters having changed)

bool g_bIgnoreSignals(false);

#define _(_x) gettext(_x)

// Stuff to do with training threads

GThread *trainthread;

struct TrainingThreadData {
  GtkWidget *pDasherControl;
  GtkWidget *pTrainingDialogue;
  gchar *szAlphabet;
};


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

// Private methods not associated with class
void update_advanced(int iParameter);

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
  DasherPreferencesDialoguePrivate *pPrivate = (DasherPreferencesDialoguePrivate *)(pDasherControl->private_data);
}

static void dasher_preferences_dialogue_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

// Public methods

DasherPreferencesDialogue *dasher_preferences_dialogue_new(GladeXML *pGladeWidgets, DasherEditor *pEditor) {

  g_message("Creating preferences dialogue");

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

  m_pButtonSettings = glade_xml_get_widget(pGladeWidgets, "window1");
  m_pSocketSettings = glade_xml_get_widget(pGladeWidgets, "window2"); 
  m_pAdvancedSettings = glade_xml_get_widget(pGladeWidgets, "window3");

  InitialiseTables(pGladeWidgets);
  RefreshWidget(-1);
  
  generate_preferences(pGladeWidgets);
  PopulateControlPage(pGladeWidgets);
  PopulateViewPage(pGladeWidgets);
  PopulateLMPage(pGladeWidgets);
  PopulateButtonsPage(pGladeWidgets);
  PopulateSocketPage(pGladeWidgets);
  PopulateAdvancedPage(pGladeWidgets);

// #ifndef GNOME_SPEECH
//   // This ought to be greyed out if not built with speech support
//   gtk_widget_set_sensitive(glade_xml_get_widget(pGladeWidgets, "speakbutton"), false);
// #endif

#ifndef JAPANESE
  gtk_widget_hide(glade_xml_get_widget(pGladeWidgets, "radiobutton9"));
#endif

  if(!dasher_app_settings_have_advanced(g_pDasherAppSettings)) {
    gtk_widget_set_sensitive(glade_xml_get_widget(pGladeWidgets, "button27"), dasher_app_settings_have_advanced(g_pDasherAppSettings));
    gtk_widget_show(glade_xml_get_widget(pGladeWidgets, "gconfwarning"));
  }
  else {
    gtk_widget_hide(glade_xml_get_widget(pGladeWidgets, "gconfwarning"));
  }

  return pDasherControl;
}

void dasher_preferences_dialogue_show(DasherPreferencesDialogue *pSelf) {
  DasherPreferencesDialoguePrivate *pPrivate = (DasherPreferencesDialoguePrivate *)(pSelf->private_data);
  // FIXME - REIMPLEMENT

  // Keep the preferences window in the correct position relative to the
  // main Dasher window
  //  gtk_window_set_transient_for(GTK_WINDOW(preferences_window),GTK_WINDOW(window));
  gtk_window_present(pPrivate->pWindow);
}


void dasher_preferences_dialogue_handle_parameter_change(DasherPreferencesDialogue *pSelf, int iParameter) {

  update_advanced(iParameter);

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
  else if(iParameter == LP_MAX_BITRATE) {
    double dNewValue = dasher_app_settings_get_long(g_pDasherAppSettings, LP_MAX_BITRATE) / 100.0;
    gtk_range_set_value(GTK_RANGE(m_pSpeedSlider), dNewValue);
  }
  else if(iParameter == BP_PALETTE_CHANGE) {
    gtk_widget_set_sensitive(glade_xml_get_widget(widgets, "ColorTree"), !dasher_app_settings_get_bool(g_pDasherAppSettings, BP_PALETTE_CHANGE));
  }
  else {
    RefreshWidget(iParameter);
  }
}


void dasher_preferences_dialogue_populate_actions(DasherPreferencesDialogue *pSelf) {
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
  gtk_tree_view_append_column(GTK_TREE_VIEW(pPrivate->pActionTreeView), pColumn);

  pRenderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(pRenderer, "toggled", (GCallback)on_action_toggle, pColumnIndex);
  pColumn = gtk_tree_view_column_new_with_attributes("Show", pRenderer, "active", ACTIONS_SHOW_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pPrivate->pActionTreeView), pColumn);

  pRenderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(pRenderer, "toggled", (GCallback)on_action_toggle, pColumnIndex + 1);
  pColumn = gtk_tree_view_column_new_with_attributes("Control", pRenderer, "active", ACTIONS_CONTROL_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pPrivate->pActionTreeView), pColumn);

  pRenderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(pRenderer, "toggled", (GCallback)on_action_toggle, pColumnIndex + 2);
  pColumn = gtk_tree_view_column_new_with_attributes("Auto", pRenderer, "active", ACTIONS_AUTO_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(pPrivate->pActionTreeView), pColumn);

  gtk_tree_view_set_model(GTK_TREE_VIEW(pPrivate->pActionTreeView), GTK_TREE_MODEL(g_pStore));

  // New input filter selection

//   dasher_preferences_populate_list(GTK_TREE_VIEW(glade_xml_get_widget(pGladeWidgets, "input_filter_tree_view")), SP_INPUT_FILTER);
//   dasher_preferences_populate_list(GTK_TREE_VIEW(glade_xml_get_widget(pGladeWidgets, "input_tree_view")), SP_INPUT_DEVICE);
}

/// Older stuff


// This file contains callbacks for the controls in the preferences
// dialogue. Please keep the callbacks in the same order that they
// appear in the dialogue box

// TODO: In theory a lot of these could be replaced by a single
// function by using user_data to store the key.

void PopulateControlPage(GladeXML *pGladeWidgets) {
  double dNewValue = dasher_app_settings_get_long(g_pDasherAppSettings, LP_MAX_BITRATE) / 100.0;
  gtk_range_set_value(GTK_RANGE(m_pSpeedSlider), dNewValue); 
//   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "onedbutton")), getBool(BP_NUMBER_DIMENSIONS));
//   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "eyetrackerbutton")), getBool(BP_EYETRACKER_MODE));
//   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "clickmodebutton")), getBool(BP_CLICK_MODE));
//  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "keyboardbutton")), getBool(BP_KEY_CONTROL));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "leftbutton")), getBool(BP_START_MOUSE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "spacebutton")), getBool(BP_START_SPACE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "adaptivebutton")), getBool(BP_AUTO_SPEEDCONTROL));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "control_controlmode")), getBool(BP_CONTROL_MODE));

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

}

void PopulateButtonsPage(GladeXML *pGladeWidgets) {
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "zoomstepsscale")), getLong(LP_ZOOMSTEPS));
  int rightzoom =  getLong(LP_RIGHTZOOM);
  rightzoom = int(rightzoom/1024);
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "rightzoomscale")), rightzoom);
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "numberofboxesscale")), getLong(LP_B));
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "sparameterscale")), getLong(LP_S));
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "zparameterscale")), getLong(LP_Z));
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "rparameterscale")), getLong(LP_R));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "pulsingmodebutton")), getBool(BP_BUTTONPULSING));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "steadymodebutton")), getBool(BP_BUTTONSTEADY));
}

void PopulateViewPage(GladeXML *pGladeWidgets) {

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
 
  //  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "toolbarbutton")), dasher_app_settings_get_bool(g_pDasherAppSettings,  APP_BP_SHOW_TOOLBAR) );
  //  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "speedsliderbutton")), getBool(BP_SHOW_SLIDER));
  //  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "showmousebutton")), getBool(BP_DRAW_MOUSE));
  //  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "showmouselinebutton")), getBool(BP_DRAW_MOUSE_LINE));

  if(getLong(LP_LINE_WIDTH) > 1)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "thicklinebutton")), true);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "thicklinebutton")), false);

  //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "keyboardbutton")), getBool(BP_KEYBOARD_MODE));
//  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "palettebutton")), getBool(BP_PALETTE_CHANGE));
//  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "outlinebutton")), getBool(BP_OUTLINE_MODE));
}


void PopulateSocketPage(GladeXML *pGladeWidgets) {
//   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "cb_socketenable")), getBool(BP_SOCKET_INPUT_ENABLE));
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(pGladeWidgets, "socketport")), (double) getLong(LP_SOCKET_PORT));

  gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(pGladeWidgets, "entrysocketxlabel")), getString(SP_SOCKET_INPUT_X_LABEL));
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(pGladeWidgets, "socketxmin")), ((double) getLong(LP_SOCKET_INPUT_X_MIN)) / 1000);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(pGladeWidgets, "socketxmax")), ((double) getLong(LP_SOCKET_INPUT_X_MAX)) / 1000);

  gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(pGladeWidgets, "entrysocketylabel")), getString(SP_SOCKET_INPUT_Y_LABEL));
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(pGladeWidgets, "socketymin")), ((double) getLong(LP_SOCKET_INPUT_Y_MIN)) / 1000);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(pGladeWidgets, "socketymax")), ((double) getLong(LP_SOCKET_INPUT_Y_MAX)) / 1000);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "cb_socketdebug")), getBool(BP_SOCKET_DEBUG));
}


// Renderer function for the advanced options list

void AdvancedCellDataFunction(GtkTreeViewColumn *pColumn, GtkCellRenderer *pRenderer, GtkTreeModel *pModel, GtkTreeIter *pIter, gpointer pUserData) {
  gpointer pData;
  gtk_tree_model_get(pModel, pIter, 1, &pData, -1);

  GValue *pValue((GValue *)pData);
  gchar szBuffer[256];

  if(G_VALUE_HOLDS_BOOLEAN(pValue)) {
    if(g_value_get_boolean(pValue))
      g_snprintf(szBuffer, sizeof(szBuffer), "Yes");
    else
      g_snprintf(szBuffer, sizeof(szBuffer), "No");
      
    g_object_set(pRenderer, "text", szBuffer, NULL);
  }
  else if(G_VALUE_HOLDS_INT(pValue)) {
    g_snprintf(szBuffer, sizeof(szBuffer), "%d", g_value_get_int(pValue));
    g_object_set(pRenderer, "text", szBuffer, NULL);
  }
  else if(G_VALUE_HOLDS_STRING(pValue)) {
    g_object_set(pRenderer, "text", g_value_get_string(pValue), NULL);
  }
}

void PopulateAdvancedPage(GladeXML *pGladeWidgets) {

  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "yaxisscale")), getLong(LP_YSCALE));
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "mouseposstartscale")), getLong(LP_MOUSEPOSDIST));

  // Now populate the generic advanced settings list
  
  GtkTreeViewColumn *column;
  GtkTreeIter advancediter;
  GtkTreeSelection *advancedselection;
  GtkWidget *advancedtreeview;
  GtkListStore *advanced_list_store;

  advancedtreeview = glade_xml_get_widget(pGladeWidgets, "advancedtree");

  // FIXME - we shouldn't need to do this - populate on realize (if not done already)
  
  gtk_widget_realize(advancedtreeview);

  advanced_list_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);
  gtk_tree_view_set_model(GTK_TREE_VIEW(advancedtreeview), GTK_TREE_MODEL(advanced_list_store));

  m_pAdvancedModel = GTK_TREE_MODEL(advanced_list_store);
  
  advancedselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(advancedtreeview));
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(advancedselection), GTK_SELECTION_BROWSE);

  column = gtk_tree_view_column_new_with_attributes("Setting", gtk_cell_renderer_text_new(), "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(advancedtreeview), column);

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer, "editable", TRUE, NULL);
  column = gtk_tree_view_column_new();
  gtk_tree_view_column_pack_start(column, renderer, TRUE);
  gtk_tree_view_column_set_cell_data_func( column, renderer, AdvancedCellDataFunction, NULL, NULL);

  g_signal_connect(renderer, "edited", (GCallback) advanced_edited_callback, advanced_list_store);
  gtk_tree_view_append_column(GTK_TREE_VIEW(advancedtreeview), column);
  
  // Clear the contents of the lmsettings list
  gtk_list_store_clear(advanced_list_store);

  // TODO - store pointers to settings objects directly in list store?
  
  for(int i(0); i < dasher_app_settings_get_count(g_pDasherAppSettings); ++i ) {
    GValue *pValue = g_new0(GValue, 1);

    switch(dasher_app_settings_get_parameter_type(g_pDasherAppSettings, i)) {
    case DASHER_TYPE_BOOL:
      g_value_init( pValue, G_TYPE_BOOLEAN );
      g_value_set_boolean(pValue, dasher_app_settings_get_bool(g_pDasherAppSettings, i));
      gtk_list_store_append(advanced_list_store, &advancediter);
      gtk_list_store_set(advanced_list_store, &advancediter, 0, dasher_app_settings_get_reg_name(g_pDasherAppSettings, i), 1, pValue, 2, i,  -1);
      break;
    case DASHER_TYPE_LONG:
      g_value_init( pValue, G_TYPE_INT );
      g_value_set_int(pValue, dasher_app_settings_get_long(g_pDasherAppSettings, i));
      gtk_list_store_append(advanced_list_store, &advancediter);
      gtk_list_store_set(advanced_list_store, &advancediter, 0, dasher_app_settings_get_reg_name(g_pDasherAppSettings, i), 1, pValue, 2, i, -1);
      break;
    case DASHER_TYPE_STRING:
      g_value_init( pValue, G_TYPE_STRING );
      g_value_set_string(pValue, dasher_app_settings_get_string(g_pDasherAppSettings, i));
      gtk_list_store_append(advanced_list_store, &advancediter);
      gtk_list_store_set(advanced_list_store, &advancediter, 0, dasher_app_settings_get_reg_name(g_pDasherAppSettings, i), 1, pValue, 2, i, -1);
      break;
    }
  }
}

void PopulateLMPage(GladeXML *pGladeWidgets) {

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
  

  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "uniformhscale")), getLong(LP_UNIFORM)/10.0);
	  
  // LM parameters are now obsolete - will eventually be part of the 'advanced' page


}

void generate_preferences(GladeXML *pGladeWidgets) {
  // We need to populate the lists of alphabets and colours
//  dasher_preferences_populate_list(GTK_TREE_VIEW(glade_xml_get_widget(pGladeWidgets, "AlphabetTree")), SP_ALPHABET_ID);
//  dasher_preferences_populate_list(GTK_TREE_VIEW(glade_xml_get_widget(pGladeWidgets, "ColorTree")), SP_COLOUR_ID);

//   GtkTreeIter alphiter, colouriter;

//   // Build the alphabet tree - this is nasty
//   alphabettreeview = glade_xml_get_widget(pGladeWidgets, "AlphabetTree");
//   alph_list_store = gtk_list_store_new(1, G_TYPE_STRING);
//   gtk_tree_view_set_model(GTK_TREE_VIEW(alphabettreeview), GTK_TREE_MODEL(alph_list_store));
//   alphselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(alphabettreeview));
//   gtk_tree_selection_set_mode(GTK_TREE_SELECTION(alphselection), GTK_SELECTION_BROWSE);
//   GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Alphabet", gtk_cell_renderer_text_new(), "text", 0, NULL);
//   gtk_tree_view_append_column(GTK_TREE_VIEW(alphabettreeview), column);

//   // Clear the contents of the alphabet list
//   gtk_list_store_clear(alph_list_store);

//   GArray *pAlphabetArray;

//   pAlphabetArray = gtk_dasher_control_get_allowed_values(GTK_DASHER_CONTROL(pDasherWidget), SP_ALPHABET_ID);

//   for(unsigned int i(0); i < pAlphabetArray->len; ++i) {

//     const gchar *pCurrentAlphabet(g_array_index(pAlphabetArray, gchar *, i));

//     gtk_list_store_append(alph_list_store, &alphiter);
//     gtk_list_store_set(alph_list_store, &alphiter, 0, pCurrentAlphabet, -1);

//     if(!strcmp(pCurrentAlphabet, getString(SP_ALPHABET_ID)))
//       SetAlphabetSelection(i, &alphiter);
    
//   }

//   g_array_free(pAlphabetArray, true);

//   // Connect up a signal so we can select a new alphabet
//   g_signal_connect_after(G_OBJECT(alphselection), "changed", GTK_SIGNAL_FUNC(alphabet_select), NULL);

//   // Do the same for colours
//   colourtreeview = glade_xml_get_widget(pGladeWidgets, "ColorTree");

//   // Make sure that the colour tree is realized now as we'll need to do
//   // stuff with it before it's actually displayed
//   gtk_widget_realize(colourtreeview);

//   colour_list_store = gtk_list_store_new(1, G_TYPE_STRING);
//   gtk_tree_view_set_model(GTK_TREE_VIEW(colourtreeview), GTK_TREE_MODEL(colour_list_store));
//   colourselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(colourtreeview));
//   gtk_tree_selection_set_mode(GTK_TREE_SELECTION(colourselection), GTK_SELECTION_BROWSE);
//   column = gtk_tree_view_column_new_with_attributes("Colour", gtk_cell_renderer_text_new(), "text", 0, NULL);
//   gtk_tree_view_append_column(GTK_TREE_VIEW(colourtreeview), column);

//   // Clear the contents of the colour list
//   gtk_list_store_clear(colour_list_store);

//   GArray *pColourArray;

//   pColourArray = gtk_dasher_control_get_allowed_values(GTK_DASHER_CONTROL(pDasherWidget), SP_COLOUR_ID);

//   for(unsigned int i(0); i < pColourArray->len; ++i) {

//     const gchar *pCurrentColour(g_array_index(pColourArray, gchar *, i));

//     gtk_list_store_append(colour_list_store, &colouriter);
//     gtk_list_store_set(colour_list_store, &colouriter, 0, pCurrentColour, -1);

//     if(!strcmp(pCurrentColour, getString(SP_COLOUR_ID))) {
//       gchar ugly_path_hack[100];
//       sprintf(ugly_path_hack, "%d", i);
//       gtk_tree_selection_select_iter(colourselection, &colouriter);
//       gtk_tree_view_set_cursor(GTK_TREE_VIEW(colourtreeview), gtk_tree_path_new_from_string(ugly_path_hack), NULL, false);
//     }
//   }

//   g_array_free(pColourArray, true);

//   // Connect up a signal so we can select a new colour scheme
//   g_signal_connect_after(G_OBJECT(colourselection), "changed", GTK_SIGNAL_FUNC(colour_select), NULL);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "manual_colour")), !getBool(BP_PALETTE_CHANGE)); 
  gtk_widget_set_sensitive(glade_xml_get_widget(pGladeWidgets, "ColorTree"), !getBool(BP_PALETTE_CHANGE));
}

// void SetAlphabetSelection(int i, GtkTreeIter *pAlphIter) {
//  //  gchar ugly_path_hack[100];
// //   sprintf(ugly_path_hack, "%d", i);
//   gtk_tree_selection_select_iter(alphselection, pAlphIter);

//   // GtkTreePath *pPath(gtk_tree_path_new_from_string(ugly_path_hack));

//   GtkTreePath *pPath(gtk_tree_model_get_path(GTK_TREE_MODEL(alph_list_store), pAlphIter));
  
//   gtk_tree_view_set_cursor(GTK_TREE_VIEW(alphabettreeview), pPath, NULL, false);
//   gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(alphabettreeview), pPath, NULL, false, 0.5, 0.0);
  
//   gtk_tree_path_free(pPath);
// }


void update_colours() {

  // FIXME - REIMPLEMENT

//   if (training==true) {
//     // We can go back and do this after training, but doing it now would
//     // break stuff
//     return;
//   }

//   colourscheme=dasher_get_current_colours();
//   const int colourlist_size=512;
//   const char *colourlist[ colourlist_size ];
//   int colour_count = dasher_get_colours( colourlist, colourlist_size );
//   for (int i=0; i<colour_count; i++) {
//     if (colourscheme==colourlist[i]) {
//       // We need to build a path - GTK 2.2 lets us do this nicely, but we
//       // want to support 2.0
//       gchar ugly_path_hack[100];
//       sprintf(ugly_path_hack,"%d",i);
//       gtk_tree_selection_select_path(colourselection,gtk_tree_path_new_from_string(ugly_path_hack));
//       gtk_tree_view_set_cursor(GTK_TREE_VIEW(colourtreeview),gtk_tree_path_new_from_string(ugly_path_hack),NULL,false);
//     }
//   }
}

// General Callbacks

extern "C" gboolean preferences_hide(GtkWidget *widget, gpointer user_data) {
  gtk_widget_hide(preferences_window);
  return TRUE;
}

// 'Alphabet' Page

// FIXME - maybe have a separate 'training thread' file

gpointer change_alphabet(gpointer alph) {

   std::cout << "Starting training thread" << std::endl;

  struct TrainingThreadData *pThreadData((struct TrainingThreadData *)alph);

  gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), SP_ALPHABET_ID, pThreadData->szAlphabet);

  std::cout << "Finished training" << std::endl;

  gtk_widget_destroy(pThreadData->pTrainingDialogue);

  g_free(pThreadData->szAlphabet);
  delete pThreadData;

  std::cout << "Finished training thread" << std::endl;

  g_thread_exit(NULL);

  return NULL;
}

extern "C" void alphabet_select(GtkTreeSelection *selection, gpointer data) {

  std::cout << "Thread status: " << g_thread_supported() << std::endl;

  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *alph;

  if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gtk_tree_model_get(model, &iter, 0, &alph, -1);

    // FIXME - Reimplement this check

    // There's no point in training if the alphabet is already selected
    //   if (alph!=alphabet) {
    //     alphabet=alph;
#ifndef WITH_GPE
    // Don't let them select another alphabet while we're training the first one
    //  if (training==true) {
    //  return;
    //  }

    // Note that we're training - this is needed in order to avoid
    // doing anything that would conflict with the other thread
    //  training=TRUE;
    //      trainqueue=g_async_queue_new();

    struct TrainingThreadData *pThreadData(new struct TrainingThreadData);

    std::cout << "*" << _("Training Dasher... please wait") << "*" << std::endl;

    train_dialogue = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, _("Training Dasher... please wait"));
    gtk_window_set_resizable(GTK_WINDOW(train_dialogue), FALSE);
    gtk_window_present(GTK_WINDOW(train_dialogue));

    pThreadData->szAlphabet = alph;
    pThreadData->pTrainingDialogue = train_dialogue;
    pThreadData->pDasherControl = pDasherWidget;

    // Clear the queue of GTK events to make sure the dialogue gets displayed before we start training.

    while(gtk_events_pending())
      gtk_main_iteration();

    gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), SP_ALPHABET_ID, pThreadData->szAlphabet);
 
    gtk_widget_hide(pThreadData->pTrainingDialogue);
    gtk_widget_destroy(pThreadData->pTrainingDialogue);
    
    g_free(pThreadData->szAlphabet);
    delete pThreadData;

    //    trainthread = g_thread_create(change_alphabet, pThreadData, false, NULL);

#else
    // For GPE, we're not so fussed at the moment
    //      dasher_set_parameter_string( STRING_ALPHABET, (gchar*)alph );
    gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), SP_ALPHABET_ID, alph);
#endif
    //      g_free(alph);
    //  } else {
//       g_free(alph);
//     }
  }
}

// 'Color' Page

extern "C" void colour_select(GtkTreeSelection *selection, gpointer data) {

  // FIXME - REIMPLEMENT

   GtkTreeIter iter;
   GtkTreeModel *model;
   gchar *colour;

   if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
     gtk_tree_model_get(model, &iter, 0, &colour, -1);

     gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), SP_COLOUR_ID, colour);    

     // Reset the colour selection as well


     g_free(colour);
   }
}

// 'Control' Page

// extern "C" void SetDimension(GtkWidget *widget, gpointer user_data) {
//   gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_NUMBER_DIMENSIONS, GTK_TOGGLE_BUTTON(widget)->active);
// }

// extern "C" void SetEyetracker(GtkWidget *widget, gpointer user_data) {
//   gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_EYETRACKER_MODE, GTK_TOGGLE_BUTTON(widget)->active);
// }

// extern "C" void SetClick(GtkWidget *widget, gpointer user_data) {
//   gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_CLICK_MODE, GTK_TOGGLE_BUTTON(widget)->active);
// }

extern "C" void startonleft(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_START_MOUSE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void startonspace(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_START_SPACE, GTK_TOGGLE_BUTTON(widget)->active);
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

extern "C" void copy_all_on_stop(GtkWidget *widget, gpointer user_data) {
  dasher_app_settings_set_bool(g_pDasherAppSettings, APP_BP_COPY_ALL_ON_STOP, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void windowpause(GtkWidget *widget, gpointer user_data) {
  // FIXME - REIMPLEMENT

  //  dasher_set_parameter_bool( BOOL_WINDOWPAUSE, GTK_TOGGLE_BUTTON(widget)->active );
}

extern "C" void on_controlmode_changed(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_CONTROL_MODE, GTK_TOGGLE_BUTTON(widget)->active);
}


extern "C" void speak(GtkWidget *widget, gpointer user_data) {
  dasher_app_settings_set_bool(g_pDasherAppSettings, APP_BP_SPEECH_MODE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void PrefsSpeedSliderChanged(GtkHScale *hscale, gpointer user_data) {
    long iNewValue = long(round(gtk_range_get_value(GTK_RANGE(hscale)) * 100));
    dasher_app_settings_set_long(g_pDasherAppSettings, LP_MAX_BITRATE, iNewValue);

}
extern "C" void adaptive(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_AUTO_SPEEDCONTROL, GTK_TOGGLE_BUTTON(widget)->active);
}

// 'View' Page

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

extern "C" void generic_bool_changed(GtkWidget *widget, gpointer user_data) {
  RefreshParameter(widget, user_data);
}

extern "C" void ThickLineClicked(GtkWidget *widget, gpointer user_data) {
  if(GTK_TOGGLE_BUTTON(widget)->active)
    gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_LINE_WIDTH, 3);
  else
    gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_LINE_WIDTH, 1);
}

// extern "C" void palettechange(GtkWidget *widget, gpointer user_data) {
//   gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_PALETTE_CHANGE, GTK_TOGGLE_BUTTON(widget)->active);
// }

extern "C" void autocolour_clicked(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_PALETTE_CHANGE, !GTK_TOGGLE_BUTTON(widget)->active);
}
// 'Advanced' Page

extern "C" void mouseposstart_y_changed(GtkRange *widget, gpointer user_data) {
  int mouseposstartdist=int(widget->adjustment->value);
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_MOUSEPOSDIST, mouseposstartdist);
}

extern "C" void y_scale_changed(GtkRange *widget, gpointer user_data) {
  int yscale=int(widget->adjustment->value);
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_YSCALE, yscale);
}

// 'Buttons' Page

extern "C" void zoomsteps_changed(GtkHScale *hscale) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ZOOMSTEPS, int(GTK_RANGE(hscale)->adjustment->value));
}

extern "C" void zoomfactor_changed(GtkHScale *hscale) {
  //gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ZOOMFACTOR, int(GTK_RANGE(hscale)->adjustment->value));
} 

extern "C" void droptime_changed(GtkHScale *hscale) {
  //gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_DROPTIME,  int(GTK_RANGE(hscale)->adjustment->value));
} 

extern "C" void numberofboxes_changed(GtkHScale *hscale) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_B, int(GTK_RANGE(hscale)->adjustment->value));
}

extern "C" void sparameter_changed(GtkHScale *hscale) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_S, int(GTK_RANGE(hscale)->adjustment->value));
}

extern "C" void rightzoom_changed(GtkHScale *hscale) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_RIGHTZOOM, long(1024 * GTK_RANGE(hscale)->adjustment->value));
}

extern "C" void zparameter_changed(GtkHScale *hscale) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_Z, int(GTK_RANGE(hscale)->adjustment->value) );
}

extern "C" void rparameter_changed(GtkHScale *hscale) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_R, int(GTK_RANGE(hscale)->adjustment->value) );
}

// 'Language Model' Page

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

extern "C" void Adaptive(GtkWidget *widget, gpointer user_data) {
  // FIXME - Not yet implemented
}

extern "C" void uniform_changed(GtkHScale *hscale) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_UNIFORM, int (GTK_RANGE(hscale)->adjustment->value * 10));
}


// 'Socket' Page

extern "C" void socketenable_toggled(GtkToggleButton *widget) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_SOCKET_INPUT_ENABLE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void socketport_changed(GtkSpinButton *widget) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_SOCKET_PORT, gtk_spin_button_get_value_as_int(widget));
}

extern "C" void socketxlabel_changed(GtkEntry *widget) {
  gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), SP_SOCKET_INPUT_X_LABEL,gtk_entry_get_text(widget));
}

extern "C" void socketxmin_changed(GtkSpinButton *widget) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_SOCKET_INPUT_X_MIN, (long) (gtk_spin_button_get_value(widget)*1000.0));
}

extern "C" void socketxmax_changed(GtkSpinButton *widget) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_SOCKET_INPUT_X_MAX, (long) (gtk_spin_button_get_value(widget)*1000.0));
}

extern "C" void socketylabel_changed(GtkEntry *widget) {
  gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), SP_SOCKET_INPUT_Y_LABEL,gtk_entry_get_text(widget));
}

extern "C" void socketymin_changed(GtkSpinButton *widget) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_SOCKET_INPUT_Y_MIN, (long) (gtk_spin_button_get_value(widget)*1000.0));
}
extern "C" void socketymax_changed(GtkSpinButton *widget) {
  gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_SOCKET_INPUT_Y_MAX, (long) (gtk_spin_button_get_value(widget)*1000.0));
}

extern "C" void socketdebug_toggled(GtkToggleButton *widget) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_SOCKET_DEBUG, GTK_TOGGLE_BUTTON(widget)->active);
}



// Advanced 2 page (reorganise!)

extern "C" void advanced_edited_callback(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer pUserData) {

  // TODO - store integer values in model

  GtkTreeModel *pModel((GtkTreeModel*)pUserData);
  
  GtkTreeIter iter;
  gtk_tree_model_get_iter_from_string( pModel, &iter, path_string );
  
  gint iKey;
  gtk_tree_model_get( pModel, &iter, 2, &iKey, -1 );

  gpointer pData;
  gtk_tree_model_get( pModel, &iter, 1, &pData, -1 );
  GValue *pValue((GValue *)pData);
  
  if(G_VALUE_HOLDS_BOOLEAN(pValue)) {
    if(!strcmp(new_text, "Yes"))
      dasher_app_settings_set_bool(g_pDasherAppSettings, iKey, TRUE);
    else if(!strcmp(new_text, "No"))
      dasher_app_settings_set_bool(g_pDasherAppSettings, iKey, FALSE);
  }
  else if(G_VALUE_HOLDS_INT(pValue)) {

    // TODO - use strtol here so we can detect errors

    dasher_app_settings_set_long(g_pDasherAppSettings, iKey, atoi(new_text));
  }
  else if(G_VALUE_HOLDS_STRING(pValue)) {
    dasher_app_settings_set_string(g_pDasherAppSettings, iKey, new_text);
  }   
}

void update_advanced(int iParameter) {

  // Don't do anything if we haven't created the tree model yet.

  if(!GTK_IS_TREE_MODEL(m_pAdvancedModel))
    return;

  GtkTreeIter sIter;

  bool bMore(gtk_tree_model_get_iter_first(m_pAdvancedModel, &sIter));

  while(bMore) {

    gint iKey;
    gtk_tree_model_get( m_pAdvancedModel, &sIter, 2, &iKey, -1 );

    if(iKey == iParameter) {
      gpointer pData;
      gtk_tree_model_get( m_pAdvancedModel, &sIter, 1, &pData, -1 );
      GValue *pValue((GValue *)pData);

      if(G_VALUE_HOLDS_BOOLEAN(pValue))
	g_value_set_boolean(pValue, dasher_app_settings_get_bool(g_pDasherAppSettings, iParameter));
      else if(G_VALUE_HOLDS_INT(pValue))
	g_value_set_int(pValue, dasher_app_settings_get_long(g_pDasherAppSettings, iParameter));
      else if(G_VALUE_HOLDS_STRING(pValue))
	g_value_set_string(pValue, dasher_app_settings_get_string(g_pDasherAppSettings, iParameter));

      gtk_tree_model_row_changed(m_pAdvancedModel, gtk_tree_model_get_path(m_pAdvancedModel, &sIter), &sIter);

      return;
    }

    bMore = gtk_tree_model_iter_next(m_pAdvancedModel, &sIter);
  }

  return;
}

// TODO: The following two methods should be combined
extern "C" gboolean button_preferences_show(GtkWidget *widget, gpointer user_data) {
  int iParameter = SP_INPUT_FILTER;
  gtk_window_present(GTK_WINDOW(dasher_preferences_dialogue_get_helper(g_pPreferencesDialogue, iParameter, dasher_app_settings_get_string(g_pDasherAppSettings, iParameter))));
  return FALSE;
}

extern "C" gboolean socket_preferences_show(GtkWidget *widget, gpointer user_data) {
  int iParameter = SP_INPUT_DEVICE;
  gtk_window_present(GTK_WINDOW(dasher_preferences_dialogue_get_helper(g_pPreferencesDialogue, iParameter, dasher_app_settings_get_string(g_pDasherAppSettings, iParameter))));
  return FALSE;
}



extern "C" gboolean button_preferences_hide(GtkWidget *widget, gpointer user_data) {
  gtk_widget_hide(m_pButtonSettings);
  return FALSE;
}


extern "C" gboolean socket_preferences_hide(GtkWidget *widget, gpointer user_data) {
  gtk_widget_hide(m_pSocketSettings);
  return FALSE;
}

extern "C" gboolean advanced_preferences_show(GtkWidget *widget, gpointer user_data) {
  //  gtk_window_present(GTK_WINDOW(m_pAdvancedSettings));
  dasher_app_settings_launch_advanced(g_pDasherAppSettings);
  return FALSE;
}

extern "C" gboolean advanced_preferences_hide(GtkWidget *widget, gpointer user_data) {
  gtk_widget_hide(m_pAdvancedSettings);
  return FALSE;
}

extern "C" void keycontrol(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_KEY_CONTROL, GTK_TOGGLE_BUTTON(widget)->active );
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

void InitialiseTables(GladeXML *pGladeWidgets) {
  int iNumBoolEntries = sizeof(sBoolTranslationTable) / sizeof(BoolTranslation);
  for(int i(0); i < iNumBoolEntries; ++i) {
    sBoolTranslationTable[i].pWidget = glade_xml_get_widget(pGladeWidgets, sBoolTranslationTable[i].szWidgetName);
  }

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
    gtk_tree_view_set_cursor((GtkTreeView *)pPointers[1], pPath, NULL, false);
    gtk_tree_view_scroll_to_cell((GtkTreeView *)pPointers[1], pPath, NULL, false, 0.5, 0.0);
    return true;
  }
  
  return false;
}

void RefreshWidget(gint iParameter) {
  int iNumBoolEntries = sizeof(sBoolTranslationTable) / sizeof(BoolTranslation);
  for(int i(0); i < iNumBoolEntries; ++i) {
    if((iParameter == -1) || (sBoolTranslationTable[i].iParameter == iParameter)) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sBoolTranslationTable[i].pWidget), dasher_app_settings_get_bool(g_pDasherAppSettings, sBoolTranslationTable[i].iParameter));
    }
  }

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

      gtk_tree_model_foreach(pModel, refresh_foreach_function, pUserData);
    }
  }
}

extern "C" void RefreshParameter(GtkWidget *pWidget, gpointer pUserData) {
  int iNumBoolEntries = sizeof(sBoolTranslationTable) / sizeof(BoolTranslation);
  
  for(int i(0); i < iNumBoolEntries; ++i) {
    if((pWidget == NULL) || (sBoolTranslationTable[i].pWidget == pWidget)) {
      if(GTK_TOGGLE_BUTTON(sBoolTranslationTable[i].pWidget)->active != dasher_app_settings_get_bool(g_pDasherAppSettings, sBoolTranslationTable[i].iParameter)) {
	dasher_app_settings_set_bool(g_pDasherAppSettings, sBoolTranslationTable[i].iParameter, GTK_TOGGLE_BUTTON(sBoolTranslationTable[i].pWidget)->active);
      }
    }
  }
}

void dasher_preferences_populate_list(GtkTreeView *pView, int iParameter, GtkWidget *pHelper) {
  const gchar *szCurrentValue(gtk_dasher_control_get_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter));

  GArray *pFilterArray = gtk_dasher_control_get_allowed_values(GTK_DASHER_CONTROL(pDasherWidget), iParameter);

  GtkListStore *pStore = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);
  gtk_tree_view_set_model(pView, GTK_TREE_MODEL(pStore));

  GtkTreeIter oIter;
  GtkTreeSelection *pSelection = gtk_tree_view_get_selection(pView);

  for(unsigned int i(0); i < pFilterArray->len; ++i) {
    const gchar *szCurrentFilter = g_array_index(pFilterArray, gchar *, i);
    gchar *szName = new gchar[strlen(szCurrentFilter) + 1];
    strcpy(szName, szCurrentFilter);

    gtk_list_store_append(pStore, &oIter);
    gtk_list_store_set(pStore, &oIter, 0, iParameter, 1, pHelper, 2, szName, 3, szName, -1);
 
    if(!strcmp(szCurrentFilter, szCurrentValue)) {
      g_message("Making selection: %s", szCurrentValue);
      gtk_tree_selection_select_iter(pSelection, &oIter);
    }
  }

  GtkCellRenderer *pRenderer;
  GtkTreeViewColumn *pColumn;
  
  pRenderer = gtk_cell_renderer_text_new();
  pColumn = gtk_tree_view_column_new_with_attributes("Action", pRenderer, "text", 2, NULL);
  gtk_tree_view_append_column(pView, pColumn);

  g_signal_connect(pSelection, "changed", (GCallback)on_list_selection, 0);
}

extern "C" void on_list_selection(GtkTreeSelection *pSelection, gpointer pUserData) {
  GtkTreeIter oIter;
  GtkTreeModel *pModel;
  
  if(gtk_tree_selection_get_selected(pSelection, &pModel, &oIter)) {
    int iParameter;
    gpointer pHelper;
    gchar *szValue;
    gtk_tree_model_get(pModel, &oIter, 0, &iParameter, 1, &pHelper, 2, &szValue, -1);
    
    g_message("Changing: %d to %s", iParameter, szValue);
    gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), iParameter, szValue);
    
    if(pHelper)
      gtk_widget_set_sensitive(GTK_WIDGET(pHelper), (dasher_preferences_dialogue_get_helper(g_pPreferencesDialogue, iParameter, szValue) != NULL));
  }
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

extern "C" void on_widget_realize(GtkWidget *pWidget, gpointer pUserData) {
  // TODO: This doesn't seem to be working
  gint *pParameter = (gint *)pUserData;
  RefreshWidget(*pParameter);
}

GtkWidget *dasher_preferences_dialogue_get_helper(DasherPreferencesDialogue *pSelf, int iParameter, const gchar *szValue) {
  switch(iParameter) {
  case SP_INPUT_FILTER:
    if(!strcmp(szValue, "Menu Mode") ||
       !strcmp(szValue, "Direct Mode") ||
       !strcmp(szValue, "Alternating Direct Mode") ||
       !strcmp(szValue, "Compass Mode"))
      return m_pButtonSettings;
    break;
  case SP_INPUT_DEVICE:
    if(!strcmp(szValue, "Socket Input"))
      return m_pSocketSettings;
    break;
  }

  return NULL;
}
