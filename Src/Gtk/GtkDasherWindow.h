#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <gtk--/window.h>
#include <gtk--/toolbar.h>
#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/menubar.h>
#include <gtk--/menu.h>
#include <gtk--/image.h>
#include <gtk--/dialog.h>
#include <gtk--/fontselection.h>


//#include <gnome--/app.h>

#include "GtkDasherSave.h"
#include "GtkDasherPane.h"

#include "DasherSettingsInterface.h"

#define TB_NEW 0
#define TB_OPEN 1
#define TB_SAVE 2
#define TB_CUT 3
#define TB_COPY 4
#define TB_COPY_ALL 5
#define TB_PASTE 6

#define MENU_NEW 0
#define MENU_OPEN 1
#define MENU_SAVE 2
#define MENU_SAVEAS 3
#define MENU_APPEND 4
#define MENU_IMPORT 5
#define MENU_EXIT 6

#define MENU_CUT 100
#define MENU_COPY 101
#define MENU_PASTE 102
#define MENU_COPYALL 103
#define MENU_SELECTALL 104

#define MENU_TOOLBAR 200
#define MENU_SLIDER 201
#define MENU_FIX 202

#define MENU_ODEFAULT 210
#define MENU_OLR 211
#define MENU_ORL 212
#define MENU_OTB 213
#define MENU_OBT 214

#define MENU_TIMESTAMP 300
#define MENU_CAOS 301
#define MENU_EFONT 302
#define MENU_DFONT 303
#define MENU_RFONT 304

#define MENU_ABOUT 400

class GtkDasherWindow : public Gtk::Window, Dasher::CDasherSettingsInterface
{
public:

  // Construtor

  GtkDasherWindow();

  // Destructor
  
  ~GtkDasherWindow();
  
  // Event handlers

  int destroy_event_impl(GdkEventAny *event);
  int delete_event_impl(GdkEventAny *event);

  // Methods inherited from CDasherSettingsInterface

  void ChangeAlphabet(const std::string& NewAlphabetID) { }; // Not implemented
  void ChangeMaxBitRate(double NewMaxBitRate);
  void ChangeLanguageModel(unsigned int NewLanguageModelID) {}; // Not implemented
  void ChangeView(unsigned int NewViewID) {}; // Not implemented
  void ChangeOrientation(Opts::ScreenOrientations Orientation);
  void SetFileEncoding(Opts::FileEncodingFormats Encoding) {}; // Not implemented
  void SetScreenSize(long Width, long Height) {}; // Not implemented
	
  void ShowToolbar(bool Value); 
  void ShowToolbarText(bool Value) {}; // This will never be implemented
  void ShowToolbarLargeIcons(bool Value) {}; // Nor will this
  void ShowSpeedSlider(bool Value);	
  void FixLayout(bool Value);
  void TimeStampNewFiles(bool Value);
  void CopyAllOnStop(bool Value);
  void SetEditFont(std::string Name, long Size) {}; // UI doesn't need to know
  void SetDasherFont(std::string Name) {}; // UI doesn't need to know
  void SetEditHeight(long Value) {}; // Not implemented

protected:

  Gtk::Menu_Helpers::MenuList *list_opts;
  Gtk::Menu_Helpers::MenuList *list_view;


  GtkDasherPane dasher_pane;
  GtkDasherSave save_dialogue;
  Gtk::Toolbar toolbar;
  Gtk::VBox main_vbox; 
  Gtk::MenuBar menubar;

  Gtk::Button button;
  Gtk::Label label;

  Gtk::FontSelectionDialog dfontsel;
  Gtk::FontSelectionDialog efontsel;

  Gtk::FileSelection ofilesel;
  Gtk::FileSelection ifilesel;
  Gtk::FileSelection afilesel;

  Gtk::Pixmap first_pixmap;

  Gtk::Dialog aboutbox;

  void toolbar_button_cb(int c);
  void menu_button_cb(int c);
  void file_ok_sel();
  void dfont_ok_sel();
  void dfont_cancel_sel();
  void efont_ok_sel();
  void efont_cancel_sel();

  void ofile_ok_sel();
  void ofile_cancel_sel();

  void ifile_ok_sel();
  void ifile_cancel_sel();

  void afile_ok_sel();
  void afile_cancel_sel();

  void about_close_sel();

  void reset();
  void save();
  void save_as();
  void exit();

  void select_all();
  void cut();
  void copy();
  void paste();
  void copy_all();

  void open();

  void orientation( Opts::ScreenOrientations o );

  void reset_fonts();

  bool slider_shown;
  bool copy_all_on_pause;
  bool toolbar_shown;
  bool fix_pane;
  bool timestamp;

  void toggle_slider();
  void toggle_copy_all();
  void toggle_toolbar();
  void toggle_fix();
  void toggle_timestamp();
};

#endif







