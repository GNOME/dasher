//TODO: Make this obsolete - GTK FONT BUTTONs exist

#include "../Common/Common.h"

//#include "dasher.h"
#include "FontDialogues.h"
//#include "GtkDasherControl.h"
//#include "../DasherCore/Parameters.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <string>

// TODO: This is clearly not going to work
static DasherAppSettings *g_pDasherAppSettings = NULL;

// Dasher font selector

GtkFontSelectionDialog *dasher_fontselector, *edit_fontselector;

void InitialiseFontDialogues(GladeXML *pGladeWidgets, DasherAppSettings *pAppSettings) {
  g_pDasherAppSettings = pAppSettings;

  dasher_fontselector = GTK_FONT_SELECTION_DIALOG(glade_xml_get_widget(pGladeWidgets, "dasher_fontselector"));
  edit_fontselector = GTK_FONT_SELECTION_DIALOG(glade_xml_get_widget(pGladeWidgets, "edit_fontselector"));
}

extern "C" gboolean dasher_font_cancel_cb(GtkWidget *widget, gpointer user_data) {
  gtk_widget_hide(GTK_WIDGET(dasher_fontselector));
  return FALSE;
}

extern "C" void dasher_font_ok_cb(GtkWidget *one, GtkWidget *two) {
  char *font_name;
  font_name = gtk_font_selection_dialog_get_font_name(dasher_fontselector);
  if(font_name) {
    if(g_pDasherAppSettings)
      dasher_app_settings_set_string(g_pDasherAppSettings, SP_DASHER_FONT, font_name);
  }
  gtk_widget_hide(GTK_WIDGET(dasher_fontselector));
}

extern "C" void dasher_font_apply_cb(GtkWidget *one, GtkWidget *two) {
  char *font_name;
  font_name = gtk_font_selection_dialog_get_font_name(dasher_fontselector);
  if(font_name) {
    if(g_pDasherAppSettings)
      dasher_app_settings_set_string(g_pDasherAppSettings, SP_DASHER_FONT, font_name);
  }
}


extern "C" void set_dasher_font(GtkWidget *widget, gpointer user_data) {
  // TODO: Do through app settings
  //  gtk_font_selection_dialog_set_font_name(dasher_fontselector,gtk_dasher_control_get_parameter_string(GTK_DASHER_CONTROL(pDasherWidget),SP_DASHER_FONT));
  gtk_window_present(GTK_WINDOW(dasher_fontselector));
}

// Edit box font selector

extern "C" gboolean edit_font_cancel_cb(GtkWidget *widget, gpointer user_data) {
  gtk_widget_hide(GTK_WIDGET(edit_fontselector));
  return FALSE;
}

extern "C" void edit_font_ok_cb(GtkWidget *one, GtkWidget *two) {
  char *font_name;
  font_name = gtk_font_selection_dialog_get_font_name(edit_fontselector);
  if(font_name) {
    if(g_pDasherAppSettings)
      dasher_app_settings_set_string(g_pDasherAppSettings, APP_SP_EDIT_FONT, font_name);
  }
  gtk_widget_hide(GTK_WIDGET(edit_fontselector));
}

extern "C" void edit_font_apply_cb(GtkWidget *one, GtkWidget *two) {
  char *font_name;
  font_name = gtk_font_selection_dialog_get_font_name(edit_fontselector);
  if(font_name) {
    if(g_pDasherAppSettings)
      dasher_app_settings_set_string(g_pDasherAppSettings, APP_SP_EDIT_FONT, font_name);
  }
}


extern "C" void set_edit_font(GtkWidget *widget, gpointer user_data) {
  gtk_font_selection_dialog_set_font_name(edit_fontselector, dasher_app_settings_get_string(g_pDasherAppSettings, APP_SP_EDIT_FONT));
  gtk_window_present(GTK_WINDOW(edit_fontselector));
}
