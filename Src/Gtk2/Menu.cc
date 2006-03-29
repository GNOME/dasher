#include "../Common/Common.h"

#include "Menu.h"
#include "dasher.h"
#include "edit.h"
#include "DasherTypes.h"
#include "fileops.h"

#include "GtkDasherControl.h"
#include "AppSettings.h"

#include <glib/gi18n.h>
#ifdef GNOME_LIBS
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#endif

#include <iostream>

extern GladeXML *widgets;

extern GtkWidget *open_filesel;
extern GtkWidget *save_filesel;
extern GtkWidget *save_and_quit_filesel;
extern GtkWidget *import_filesel;
extern GtkWidget *append_filesel;
extern GtkWidget *window;
extern GtkWidget *file_selector;

void PopulateMenus(GladeXML *pGladeWidgets) {
  switch(gtk_dasher_control_get_parameter_long( GTK_DASHER_CONTROL(pDasherWidget), LP_DASHER_FONTSIZE)) {
  case 1:
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(glade_xml_get_widget(pGladeWidgets, "fontsizenormal")), true);
    break;
  case 2:
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(glade_xml_get_widget(pGladeWidgets, "fontsizelarge")), true);
    break;
  case 4:
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(glade_xml_get_widget(pGladeWidgets, "fontsizevlarge")), true);
    break;
  }
}

extern "C" void select_save_file_as(GtkWidget * widget, gpointer user_data);

// 'File' Menu

#if GTK_CHECK_VERSION(2,3,0)

extern "C" void select_new_file(GtkWidget *widget, gpointer user_data) {
  //FIXME - confirm this. We should check whether the user wants to lose their work.

  choose_filename();
  clear_edit();
  //  gtk_dasher_control_set_context(GTK_DASHER_CONTROL(pDasherWidget), "");
  gtk_dasher_control_invalidate_context(GTK_DASHER_CONTROL(pDasherWidget));

  // Starting a new file indicates a new user trial in detailed logging
  gtk_dasher_user_log_new_trial(GTK_DASHER_CONTROL(pDasherWidget));

}

#endif

#if GTK_CHECK_VERSION(2,3,0)

extern "C" void select_open_file(GtkWidget *widget, gpointer user_data) {
  GtkWidget *filesel = gtk_file_chooser_dialog_new(_("Select File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

#ifdef GNOME_LIBS
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filesel), FALSE);
#endif

  if(gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
#ifdef GNOME_LIBS
    char *filename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filesel));
#else
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
#endif
    open_file(filename);
    g_free(filename);
  }
  gtk_widget_destroy(filesel);
}

#else

extern "C" void filesel_hide(GtkWidget *widget, gpointer user_data) {
  // FIXME - uh. Yes. This works, but is it in any way guaranteed to?
  // Of course, if glade let us set user_data stuff properly, this would
  // be a lot easier
  gtk_widget_hide(gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(widget))));
}

extern "C" void open_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
  open_file(filename);
}

extern "C" void select_open_file(GtkWidget *widget, gpointer user_data) {
  if(open_filesel == NULL) {
    open_filesel = glade_xml_get_widget(widgets, "open_fileselector");
    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(open_filesel)->ok_button), "clicked", G_CALLBACK(open_file_from_filesel), (gpointer) open_filesel);
  }
  gtk_window_set_transient_for(GTK_WINDOW(open_filesel), GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(open_filesel));
}

#endif

extern "C" void save_file(GtkWidget *widget, gpointer user_data) {
  if(filename != NULL) {
    save_file_as(filename,FALSE); // FIXME - REIMPLEMENT
  }
  else {
    select_save_file_as(NULL, NULL);
  }
}

#if GTK_CHECK_VERSION(2,3,0)

extern "C" void select_save_file_as(GtkWidget *widget, gpointer user_data) {
  GtkWidget *filesel = gtk_file_chooser_dialog_new(_("Select File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

#ifdef GNOME_LIBS
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filesel), FALSE);
#endif

  if(gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
#ifdef GNOME_LIBS
    char *filename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filesel));
#else
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
#endif
    save_file_as(filename,FALSE);
    g_free(filename);
  }
  gtk_widget_destroy(filesel);
}

#else

extern "C" void save_file_from_filesel_and_quit(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  if(save_file_as(filename, FALSE) == false) {

    // FIXME - do we really just want to fail silently if the save operation fails?

    return;
  }
  else {
    exiting = TRUE;
    gtk_main_quit();
  }
  return;
}

extern "C" void save_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
  save_file_as(filename, FALSE);
}

extern "C" void select_save_file_as(GtkWidget *widget, gpointer user_data) {
  if(save_filesel == NULL) {
    save_filesel = glade_xml_get_widget(widgets, "save_fileselector");
    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(save_filesel)->ok_button), "clicked", G_CALLBACK(save_file_from_filesel), (gpointer) save_filesel);
  }

  if(filename != NULL)
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(save_filesel), filename);
  gtk_window_set_transient_for(GTK_WINDOW(save_filesel), GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(save_filesel));
}

#endif

#if GTK_CHECK_VERSION(2,3,0)

extern "C" void select_append_file(GtkWidget *widget, gpointer user_data) {
  GtkWidget *filesel = gtk_file_chooser_dialog_new(_("Select File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

#ifdef GNOME_LIBS
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filesel), FALSE);
#endif

  if(gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
#ifdef GNOME_LIBS
    char *filename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filesel));
#else
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
#endif
    //    save_file_as(filename,TRUE);// FIXME - REIMPLEMENT
    g_free(filename);
  }
  gtk_widget_destroy(filesel);
}

#else

extern "C" void append_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));

  save_file_as(filename, TRUE);

  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
}

extern "C" void select_append_file(GtkWidget *widget, gpointer user_data) {
  if(append_filesel == NULL) {
    append_filesel = glade_xml_get_widget(widgets, "append_fileselector");
    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(append_filesel)->ok_button), "clicked", G_CALLBACK(append_file_from_filesel), (gpointer) append_filesel);
  }

  gtk_window_set_transient_for(GTK_WINDOW(append_filesel), GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(append_filesel));
}

#endif

#if GTK_CHECK_VERSION(2,3,0)

extern "C" void select_import_file(GtkWidget *widget, gpointer user_data) {
  GtkWidget *filesel = gtk_file_chooser_dialog_new(_("Select File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

#ifdef GNOME_LIBS
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filesel), FALSE);
#endif

  if(gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_ACCEPT) {
#ifdef GNOME_LIBS
    char *filename = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filesel));
#else
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filesel));
#endif

    gtk_dasher_control_train(GTK_DASHER_CONTROL(pDasherWidget), filename);

    g_free(filename);
  }

  gtk_widget_destroy(filesel);
}

#else

extern "C" void import_file_from_filesel(GtkWidget *selector2, GtkFileSelection *selector) {
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  gtk_dasher_control_train(GTK_DASHER_CONTROL(pDasherWidget), filename);
  filesel_hide(GTK_WIDGET(selector->ok_button), NULL);
}

extern "C" void select_import_file(GtkWidget *widget, gpointer user_data) {
  if(import_filesel == NULL) {
    import_filesel = glade_xml_get_widget(widgets, "import_fileselector");

    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(import_filesel)->ok_button), "clicked", G_CALLBACK(import_file_from_filesel), (gpointer) import_filesel);
  }

  gtk_window_set_transient_for(GTK_WINDOW(import_filesel), GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(import_filesel));
}

#endif

#if !GTK_CHECK_VERSION(2,3,0)

// FIXME - I believe that this doesn't ever get called

extern "C" void select_save_file_as_and_quit(GtkWidget *widget, gpointer user_data) {
  if(save_and_quit_filesel == NULL) {
    save_and_quit_filesel = glade_xml_get_widget(widgets, "save_and_quit_fileselector");
    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(save_and_quit_filesel)->ok_button), "clicked", G_CALLBACK(save_file_from_filesel_and_quit), (gpointer) save_and_quit_filesel);
  }

  if(filename != NULL)
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(save_and_quit_filesel), filename);
  gtk_window_set_transient_for(GTK_WINDOW(save_and_quit_filesel), GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(save_and_quit_filesel));
}

#endif

extern "C" void save_file_and_quit(GtkWidget *widget, gpointer user_data) {
  if(filename != NULL) {
    if (save_file_as(filename,FALSE)==true) {
    //  exiting=TRUE;
    SaveWindowState();
    gtk_main_quit();
    } else {
      return;
    }
  }
  else {
    select_save_file_as(NULL, NULL);
    SaveWindowState();
    gtk_main_quit();
  }
}

extern "C" bool ask_save_before_exit(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog = NULL;

  if(file_modified != FALSE) {
    // Ask whether to save the modified file, insert filename if it exists.
    if(filename != NULL) {
      dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, _("Do you want to save your changes to %s?\n\nYour changes will be lost if you don't save them."), filename);
    }
    else if(filename == NULL) {
      dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, _("Do you want to save your changes?\n\nYour changes will be lost if you don't save them."));
    }

    gtk_dialog_add_buttons(GTK_DIALOG(dialog), _("Don't save"), GTK_RESPONSE_REJECT, _("Don't quit"), GTK_RESPONSE_CANCEL, _("Save and quit"), GTK_RESPONSE_ACCEPT, NULL);
    switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
    case GTK_RESPONSE_REJECT:
      //      write_to_file(); // FIXME - REIMPLEMENT
      SaveWindowState();
      gtk_main_quit();
      break;
    case GTK_RESPONSE_CANCEL:
      gtk_widget_destroy(GTK_WIDGET(dialog));
      return true;
      break;
    case GTK_RESPONSE_ACCEPT:
      gtk_widget_destroy(GTK_WIDGET(dialog));
      //      write_to_file(); // FIXME - REIMPLEMENT
      save_file_and_quit(NULL, NULL);
    }
  }
  else {
    // It should be noted that write_to_file merely saves the new text to the training
    // file rather than saving it to a file of the user's choice

    // FIXME - REIMPLEMENT

    //    write_to_file();
    SaveWindowState();
    gtk_main_quit();
  }
  return false;
}

// 'Edit' menu

extern "C" void clipboard_cut(void) {
  dasher_editor_clipboard(g_pEditor, CLIPBOARD_CUT);       // FIXME - give this a better name
}

extern "C" void clipboard_copy(void) {
  dasher_editor_clipboard(g_pEditor, CLIPBOARD_COPY);
}

extern "C" void clipboard_paste(void) {
  dasher_editor_clipboard(g_pEditor, CLIPBOARD_PASTE);
}

extern "C" void clipboard_copy_all(void) {
  dasher_editor_clipboard(g_pEditor, CLIPBOARD_COPYALL);
}

// Not actually on the menu, but should be?

extern "C" void clipboard_select_all(void) {
  dasher_editor_clipboard(g_pEditor, CLIPBOARD_SELECTALL);
}

// 'Options' Menu

// 'Preferences' item is in Preferences.cpp

// Font selector options will be in their own file

extern "C" void set_dasher_fontsize(GtkWidget *widget, gpointer user_data) {
  if(GTK_CHECK_MENU_ITEM(widget)->active == TRUE) {
    if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "fontsizenormal")) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_DASHER_FONTSIZE, Dasher::Opts::Normal);
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "fontsizelarge")) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_DASHER_FONTSIZE, Dasher::Opts::Big);
    }
    else if(!strcmp(gtk_widget_get_name(GTK_WIDGET(widget)), "fontsizevlarge")) {
      gtk_dasher_control_set_parameter_long(GTK_DASHER_CONTROL(pDasherWidget), LP_DASHER_FONTSIZE, Dasher::Opts::VBig);
    }
  }
}

extern "C" void reset_fonts(GtkWidget *widget, gpointer user_data) {
  dasher_app_settings_reset(g_pDasherAppSettings, SP_DASHER_FONT);
  dasher_app_settings_reset(g_pDasherAppSettings, APP_SP_EDIT_FONT);
}

// 'Help' Menu

extern "C" void about_dasher(GtkWidget *widget, gpointer user_data) {

#ifdef GNOME_LIBS

  // In alphabetical order
  const gchar *authors[] = {
    "Chris Ball",
    "Phil Cowans",
    "Frederik Eaton",
    "Behdad Esfahbod",
    "Matthew Garrett",
    "Chris Hack",
    "David MacKay",
    "Iain Murray",
    "Takashi Kaburagi",
    "Keith Vertanen",
    "Hanna Wallach",
    "David Ward",
    "Brian Williams",
    "Seb Wills",
    NULL
  };

  // Yeah, should really do some Gnome documentation for it...
  const gchar *documenters[] = {
    "Chris Ball",
    "Matthew Garrett",
    "David MacKay",
    NULL
  };

  gtk_show_about_dialog(GTK_WINDOW(window), 
			"copyright", "Copyright The Dasher Project", 
			"comments", _("Dasher is a predictive text entry application"), 
			"authors", (const char **)authors,
			"documenters", (const char **)documenters,
			"translator-credits", _("translator-credits"),
			"website", "http://www.dasher.org.uk/",
			"logo-icon-name", "dasher",
			"version", PACKAGE_VERSION,
			NULL);
  
#else
  // EAT UGLY ABOUT BOX, PHILISTINE
  GtkWidget *label, *button;
  char *tmp;

  GtkWidget *about = gtk_dialog_new();

  gtk_dialog_set_has_separator(GTK_DIALOG(about), FALSE);
  gtk_window_set_title(GTK_WINDOW(about), "About Dasher");

  tmp = g_strdup_printf("Dasher Version %s ", VERSION);
  label = gtk_label_new(tmp);
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about)->vbox), label, FALSE, FALSE, 0);

  label = gtk_label_new("http://www.inference.phy.cam.ac.uk/dasher/");
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about)->vbox), label, FALSE, FALSE, 0);

  label = gtk_label_new("Copyright The Dasher Project");
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about)->vbox), label, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock(GTK_STOCK_OK);
  gtk_widget_show(button);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about)->vbox), button, FALSE, FALSE, 0);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(about));

  gtk_widget_show(about);
#endif
}

// FIXME - I think this is never called?

extern "C" void show_help(GtkWidget *widget, gpointer user_data) {
#ifdef GNOME_LIBS
  gnome_help_display_desktop(NULL, "dasher", "dasher", NULL, NULL);
#endif
}
