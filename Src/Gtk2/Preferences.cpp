#include "Preferences.h"
#include "Parameters.h"
#include "dasher.h"
#include "GtkDasherControl.h"
#include "AppSettings.h"

#include "DasherControl.h"

#include "DasherTypes.h"

#include <cstring>

void PopulateLMPage(GladeXML * pGladeWidgets);
void generate_preferences(GladeXML * pGladeWidgets);
void PopulateControlPage(GladeXML * pGladeWidgets);
void PopulateViewPage(GladeXML * pGladeWidgets);
void PopulateButtonsPage(GladeXML * pGladeWidgets);
void PopulateAdvancedPage(GladeXML *pGladeWidgets);

void SetAlphabetSelection(int i, GtkTreeIter *pAlphIter);

extern "C" void advanced_edited_callback(GtkCellRendererText * cell, gchar * path_string, gchar * new_text, gpointer user_data);
extern "C" void colour_select(GtkTreeSelection * selection, gpointer data);
extern "C" void alphabet_select(GtkTreeSelection * selection, gpointer data);

GtkTreeSelection *alphselection, *colourselection;
GtkWidget *alphabettreeview, *colourtreeview;
GtkListStore *alph_list_store;
GtkListStore *colour_list_store;

GtkTreeModel *m_pAdvancedModel;

GtkWidget *preferences_window;
GtkWidget *train_dialogue;

GtkWidget *m_pLRButton;
GtkWidget *m_pRLButton;
GtkWidget *m_pTBButton;
GtkWidget *m_pBTButton;
GtkWidget *m_pSpeedSlider;

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

// This file contains callbacks for the controls in the preferences
// dialogue. Please keep the callbacks in the same order that they
// appear in the dialogue box

// TODO: In theory a lot of these could be replaced by a single
// function by using user_data to store the key.

void initialise_preferences_dialogue(GladeXML *pGladeWidgets) {
  preferences_window = glade_xml_get_widget(pGladeWidgets, "preferences");
  m_pSpeedSlider = glade_xml_get_widget(pGladeWidgets, "hscale1");

  generate_preferences(pGladeWidgets);
  PopulateControlPage(pGladeWidgets);
  PopulateViewPage(pGladeWidgets);
  PopulateLMPage(pGladeWidgets);
  PopulateButtonsPage(pGladeWidgets);
  PopulateAdvancedPage(pGladeWidgets);
}

void PopulateControlPage(GladeXML *pGladeWidgets) {
  double dNewValue = get_app_parameter_long(LP_MAX_BITRATE) / 100.0;
  gtk_range_set_value(GTK_RANGE(m_pSpeedSlider), dNewValue); 
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "onedbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_NUMBER_DIMENSIONS));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "eyetrackerbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_EYETRACKER_MODE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "clickmodebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_CLICK_MODE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "leftbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_START_MOUSE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "spacebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_START_SPACE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "mouseposbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_MOUSEPOS_MODE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "copyallstopbutton")), get_app_parameter_bool(APP_BP_COPY_ALL_ON_STOP));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "speakbutton")), get_app_parameter_bool(APP_BP_SPEECH_MODE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "adaptivebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_AUTO_SPEEDCONTROL));
}

void PopulateButtonsPage(GladeXML *pGladeWidgets) {
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "zoomstepsscale")), gtk_dasher_control_get_parameter_long( GTK_DASHER_CONTROL( pDasherWidget ), LP_ZOOMSTEPS));
}

void PopulateViewPage(GladeXML *pGladeWidgets) {

  m_pLRButton = glade_xml_get_widget(pGladeWidgets, "radiobutton2");
  m_pRLButton = glade_xml_get_widget(pGladeWidgets, "radiobutton3");
  m_pTBButton = glade_xml_get_widget(pGladeWidgets, "radiobutton4");
  m_pBTButton = glade_xml_get_widget(pGladeWidgets, "radiobutton5");

  switch (gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ORIENTATION)) {
  case Dasher::Opts::Alphabet:
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton1"))) != TRUE)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "radiobutton1")), TRUE); 
    
    g_bIgnoreSignals = true;

    switch (gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_REAL_ORIENTATION)) {
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

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "toolbarbutton")), get_app_parameter_bool( APP_BP_SHOW_TOOLBAR) );
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "speedsliderbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_SHOW_SLIDER));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "showmousebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_DRAW_MOUSE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "showmouselinebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_DRAW_MOUSE_LINE));

  if(gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_LINE_WIDTH) > 1)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "thicklinebutton")), true);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "thicklinebutton")), true);

//   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "keyboardbutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_KEYBOARD_MODE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "palettebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_PALETTE_CHANGE));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "outlinebutton")), gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_OUTLINE_MODE));
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
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "timestampbutton")), get_app_parameter_bool( APP_BP_TIME_STAMP ) );

  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "yaxisscale")), gtk_dasher_control_get_parameter_long( GTK_DASHER_CONTROL( pDasherWidget ), LP_YSCALE));
  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "mouseposscale")), gtk_dasher_control_get_parameter_long( GTK_DASHER_CONTROL( pDasherWidget ), LP_MOUSEPOSDIST));

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
  
  for(int i(0); i < GetParameterCount(); ++i ) {
    GValue *pValue = g_new0(GValue, 1);

    switch(GetParameterType(i)) {
    case DASHER_TYPE_BOOL:
      g_value_init( pValue, G_TYPE_BOOLEAN );
      g_value_set_boolean(pValue, get_app_parameter_bool(i));
      gtk_list_store_append(advanced_list_store, &advancediter);
      gtk_list_store_set(advanced_list_store, &advancediter, 0, GetParameterRegName(i), 1, pValue, 2, i,  -1);
      break;
    case DASHER_TYPE_LONG:
      g_value_init( pValue, G_TYPE_INT );
      g_value_set_int(pValue, get_app_parameter_long(i));
      gtk_list_store_append(advanced_list_store, &advancediter);
      gtk_list_store_set(advanced_list_store, &advancediter, 0, GetParameterRegName(i), 1, pValue, 2, i, -1);
      break;
    case DASHER_TYPE_STRING:
      g_value_init( pValue, G_TYPE_STRING );
      g_value_set_string(pValue, get_app_parameter_string(i));
      gtk_list_store_append(advanced_list_store, &advancediter);
      gtk_list_store_set(advanced_list_store, &advancediter, 0, GetParameterRegName(i), 1, pValue, 2, i, -1);
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
  

  gtk_range_set_value( GTK_RANGE(glade_xml_get_widget(pGladeWidgets, "uniformhscale")), gtk_dasher_control_get_parameter_long( GTK_DASHER_CONTROL( pDasherWidget ), LP_UNIFORM)/10.0);
	  
  // LM parameters are now obsolete - will eventually be part of the 'advanced' page


}

void generate_preferences(GladeXML *pGladeWidgets) {
  // We need to populate the lists of alphabets and colours

  GtkTreeIter alphiter, colouriter;

  // Build the alphabet tree - this is nasty
  alphabettreeview = glade_xml_get_widget(pGladeWidgets, "AlphabetTree");
  alph_list_store = gtk_list_store_new(1, G_TYPE_STRING);
  gtk_tree_view_set_model(GTK_TREE_VIEW(alphabettreeview), GTK_TREE_MODEL(alph_list_store));
  alphselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(alphabettreeview));
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(alphselection), GTK_SELECTION_BROWSE);
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Alphabet", gtk_cell_renderer_text_new(), "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(alphabettreeview), column);

  // Clear the contents of the alphabet list
  gtk_list_store_clear(alph_list_store);

  GArray *pAlphabetArray;

  pAlphabetArray = gtk_dasher_control_get_allowed_values(GTK_DASHER_CONTROL(pDasherWidget), SP_ALPHABET_ID);

  for(unsigned int i(0); i < pAlphabetArray->len; ++i) {

    const gchar *pCurrentAlphabet(g_array_index(pAlphabetArray, gchar *, i));

    gtk_list_store_append(alph_list_store, &alphiter);
    gtk_list_store_set(alph_list_store, &alphiter, 0, pCurrentAlphabet, -1);

    if(!strcmp(pCurrentAlphabet, gtk_dasher_control_get_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), SP_ALPHABET_ID)))
      SetAlphabetSelection(i, &alphiter);
    
  }

  g_array_free(pAlphabetArray, true);

  // Connect up a signal so we can select a new alphabet
  g_signal_connect_after(G_OBJECT(alphselection), "changed", GTK_SIGNAL_FUNC(alphabet_select), NULL);

  // Do the same for colours
  colourtreeview = glade_xml_get_widget(pGladeWidgets, "ColorTree");

  // Make sure that the colour tree is realized now as we'll need to do
  // stuff with it before it's actually displayed
  gtk_widget_realize(colourtreeview);

  colour_list_store = gtk_list_store_new(1, G_TYPE_STRING);
  gtk_tree_view_set_model(GTK_TREE_VIEW(colourtreeview), GTK_TREE_MODEL(colour_list_store));
  colourselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(colourtreeview));
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(colourselection), GTK_SELECTION_BROWSE);
  column = gtk_tree_view_column_new_with_attributes("Colour", gtk_cell_renderer_text_new(), "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(colourtreeview), column);

  // Clear the contents of the colour list
  gtk_list_store_clear(colour_list_store);

  GArray *pColourArray;

  pColourArray = gtk_dasher_control_get_allowed_values(GTK_DASHER_CONTROL(pDasherWidget), SP_COLOUR_ID);

  for(unsigned int i(0); i < pColourArray->len; ++i) {

    const gchar *pCurrentColour(g_array_index(pColourArray, gchar *, i));

    gtk_list_store_append(colour_list_store, &colouriter);
    gtk_list_store_set(colour_list_store, &colouriter, 0, pCurrentColour, -1);

    if(!strcmp(pCurrentColour, gtk_dasher_control_get_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), SP_COLOUR_ID))) {
      gchar ugly_path_hack[100];
      sprintf(ugly_path_hack, "%d", i);
      gtk_tree_selection_select_iter(colourselection, &colouriter);
      gtk_tree_view_set_cursor(GTK_TREE_VIEW(colourtreeview), gtk_tree_path_new_from_string(ugly_path_hack), NULL, false);
    }
  }

  g_array_free(pColourArray, true);

  // Connect up a signal so we can select a new colour scheme
  g_signal_connect_after(G_OBJECT(colourselection), "changed", GTK_SIGNAL_FUNC(colour_select), NULL);

  if(gtk_dasher_control_get_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_PALETTE_CHANGE)) {
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "autocolour"))))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "autocolour")), TRUE); 
    gtk_widget_set_sensitive(colourtreeview, FALSE);
  }
  else {
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "manualcolour"))))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(pGladeWidgets, "manualcolour")), TRUE); 
    gtk_widget_set_sensitive(colourtreeview, TRUE);
  }
}

void SetAlphabetSelection(int i, GtkTreeIter *pAlphIter) {
 //  gchar ugly_path_hack[100];
//   sprintf(ugly_path_hack, "%d", i);
  gtk_tree_selection_select_iter(alphselection, pAlphIter);

  // GtkTreePath *pPath(gtk_tree_path_new_from_string(ugly_path_hack));

  GtkTreePath *pPath(gtk_tree_model_get_path(GTK_TREE_MODEL(alph_list_store), pAlphIter));
  
  gtk_tree_view_set_cursor(GTK_TREE_VIEW(alphabettreeview), pPath, NULL, false);
  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(alphabettreeview), pPath, NULL, false, 0.5, 0.0);
  
  gtk_tree_path_free(pPath);
}


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

extern "C" void preferences_display(GtkWidget *widget, gpointer user_data) {

  // FIXME - REIMPLEMENT

  // Keep the preferences window in the correct position relative to the
  // main Dasher window
  //  gtk_window_set_transient_for(GTK_WINDOW(preferences_window),GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(preferences_window));
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

    train_dialogue = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, _("Training Dasher, please wait"));
    gtk_window_set_resizable(GTK_WINDOW(train_dialogue), FALSE);
    gtk_window_present(GTK_WINDOW(train_dialogue));

    pThreadData->szAlphabet = alph;
    pThreadData->pTrainingDialogue = train_dialogue;
    pThreadData->pDasherControl = pDasherWidget;

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

extern "C" void SetDimension(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_NUMBER_DIMENSIONS, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void SetEyetracker(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_EYETRACKER_MODE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void SetClick(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_CLICK_MODE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void startonleft(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_START_MOUSE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void startonspace(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_START_SPACE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void startonmousepos(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_MOUSEPOS_MODE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void copy_all_on_stop(GtkWidget *widget, gpointer user_data) {
 set_app_parameter_bool(APP_BP_COPY_ALL_ON_STOP, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void windowpause(GtkWidget *widget, gpointer user_data) {
  // FIXME - REIMPLEMENT

  //  dasher_set_parameter_bool( BOOL_WINDOWPAUSE, GTK_TOGGLE_BUTTON(widget)->active );
}

extern "C" void speak(GtkWidget *widget, gpointer user_data) {
  set_app_parameter_bool(APP_BP_SPEECH_MODE, GTK_TOGGLE_BUTTON(widget)->active);
}

void preferences_handle_parameter_change(int iParameter)
{
  switch(iParameter) {
  case LP_MAX_BITRATE:
    {    
      double dNewValue = get_app_parameter_long(LP_MAX_BITRATE) / 100.0;
      gtk_range_set_value(GTK_RANGE(m_pSpeedSlider), dNewValue);
      break;
    }

  }
      
}

extern "C" void PrefsSpeedSliderChanged(GtkHScale *hscale, gpointer user_data) {
    long iNewValue = round(gtk_range_get_value(GTK_RANGE(hscale)) * 100);
    set_app_parameter_long(LP_MAX_BITRATE, iNewValue);

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
      
      switch (gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_REAL_ORIENTATION)) {
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

      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_ORIENTATION,  gtk_dasher_control_get_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_REAL_ORIENTATION));
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

extern "C" void show_toolbar(GtkWidget *widget, gpointer user_data) {
  set_app_parameter_bool( APP_BP_SHOW_TOOLBAR, GTK_TOGGLE_BUTTON(widget)->active );
}

extern "C" void show_slider(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_SHOW_SLIDER, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void DrawMouse(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_DRAW_MOUSE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void DrawMouseLine(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_DRAW_MOUSE_LINE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void ThickLineClicked(GtkWidget *widget, gpointer user_data) {
  if(GTK_TOGGLE_BUTTON(widget)->active)
    gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_LINE_WIDTH, 3);
  else
    gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_LINE_WIDTH, 1);
}

extern "C" void outlineboxes(GtkWidget *widget, gpointer user_data) {
  // drawoutline=GTK_TOGGLE_BUTTON(widget)->active; // FIXME - REIMPLEMENT
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_OUTLINE_MODE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void palettechange(GtkWidget *widget, gpointer user_data) {
  gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_PALETTE_CHANGE, GTK_TOGGLE_BUTTON(widget)->active);
}

extern "C" void autocolour_clicked(GtkWidget *widget, gpointer user_data) {
  if(GTK_TOGGLE_BUTTON(widget)->active == TRUE) {
    if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "autocolour")) { 
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_PALETTE_CHANGE, TRUE);
      gtk_widget_set_sensitive(colourtreeview, FALSE);
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "manualcolour")) { 
      gtk_dasher_control_set_parameter_bool(GTK_DASHER_CONTROL(pDasherWidget), BP_PALETTE_CHANGE, FALSE);
      gtk_widget_set_sensitive(colourtreeview, TRUE);
    }
  }
}

// 'Advanced' Page

extern "C" void timestamp_files(GtkWidget *widget, gpointer user_data) {
  set_app_parameter_bool( APP_BP_TIME_STAMP, GTK_TOGGLE_BUTTON(widget)->active );
}

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


// 'Langauge Model' Page

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
      set_app_parameter_bool(iKey, TRUE);
    else if(!strcmp(new_text, "No"))
      set_app_parameter_bool(iKey, FALSE);
  }
  else if(G_VALUE_HOLDS_INT(pValue)) {

    // TODO - use strtol here so we can detect errors

    set_app_parameter_long(iKey, atoi(new_text));
  }
  else if(G_VALUE_HOLDS_STRING(pValue)) {
    set_app_parameter_string(iKey, new_text);
  }   
}

void update_advanced(int iParameter) {
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
	g_value_set_boolean(pValue, get_app_parameter_bool(iParameter));
      else if(G_VALUE_HOLDS_INT(pValue))
	g_value_set_int(pValue, get_app_parameter_long(iParameter));
      else if(G_VALUE_HOLDS_STRING(pValue))
	g_value_set_string(pValue, get_app_parameter_string(iParameter));

      gtk_tree_model_row_changed(m_pAdvancedModel, gtk_tree_model_get_path(m_pAdvancedModel, &sIter), &sIter);

      return;
    }

    bMore = gtk_tree_model_iter_next(m_pAdvancedModel, &sIter);
  }

  return;
}
