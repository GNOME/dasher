#include "dasher.h"
#include "FontDialogues.h"
#include "edit.h"
#include "GtkDasherControl.h"
#include "../DasherCore/Parameters.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glade/glade.h>

#include <string>

// Dasher font selector

GtkFontSelectionDialog *dasher_fontselector, *edit_fontselector;

void InitialiseFontDialogues(GladeXML *pGladeWidgets) {
  dasher_fontselector = GTK_FONT_SELECTION_DIALOG(glade_xml_get_widget(pGladeWidgets, "dasher_fontselector"));
  edit_fontselector = GTK_FONT_SELECTION_DIALOG(glade_xml_get_widget(pGladeWidgets, "edit_fontselector"));
}

extern "C" gboolean fontsel_hide(GtkWidget *widget, gpointer user_data) {
  gtk_widget_hide(GTK_WIDGET(dasher_fontselector));
  return FALSE;
}

extern "C" void get_font_from_dialog(GtkWidget *one, GtkWidget *two) {
  char *font_name;
  font_name = gtk_font_selection_dialog_get_font_name(dasher_fontselector);
  if(font_name) {
    gtk_dasher_control_set_parameter_string(GTK_DASHER_CONTROL(pDasherWidget), SP_DASHER_FONT, font_name);
  }
  fontsel_hide(NULL, NULL);
  //  dasher_redraw();
}

extern "C" void set_dasher_font(GtkWidget *widget, gpointer user_data) {
  g_signal_connect(dasher_fontselector->ok_button, "clicked", G_CALLBACK(get_font_from_dialog), (gpointer) dasher_fontselector);
  // FIXME - REIMPLEMENT
  // gtk_window_set_transient_for(GTK_WINDOW(dasher_fontselector),GTK_WINDOW(window));
  // gtk_font_selection_dialog_set_font_name(dasher_fontselector,dasherfont.c_str());
  gtk_window_present(GTK_WINDOW(dasher_fontselector));
}

// Edit box font selector

extern "C" gboolean edit_fontsel_hide(GtkWidget *widget, gpointer user_data) {
  gtk_widget_hide(GTK_WIDGET(edit_fontselector));
  return FALSE;
}

extern "C" void get_edit_font_from_dialog(GtkWidget *one, GtkWidget *two) {
  char *font_name;
  font_name = gtk_font_selection_dialog_get_font_name(edit_fontselector);
  if(font_name) {

    // FIXME - REIMPLEMENT

    //    dasher_set_parameter_string( STRING_EDITFONT, font_name );
    set_editbox_font(font_name);
    //    editfont=font_name;
  }
  edit_fontsel_hide(NULL, NULL);
}

extern "C" void set_edit_font(GtkWidget *widget, gpointer user_data) {
  g_signal_connect(edit_fontselector->ok_button, "clicked", G_CALLBACK(get_edit_font_from_dialog), (gpointer) edit_fontselector);

  // FIXME - REIMPLEMENT

  //  gtk_window_set_transient_for(GTK_WINDOW(edit_fontselector),GTK_WINDOW(window));
  //  gtk_font_selection_dialog_set_font_name(edit_fontselector,editfont.c_str());
  //  GtkWidget *cancel_butto3 = glade_xml_get_widget(widgets,"cancel_butto3");
  //  gtk_widget_hide(cancel_butto3);
  gtk_window_present(GTK_WINDOW(edit_fontselector));
}
