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

//#include <gnome--/app.h>

#include "GtkDasherSave.h"
#include "GtkDasherPane.h"

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

#define MENU_ODEFAULT 210
#define MENU_OLR 211
#define MENU_ORL 212
#define MENU_OTB 213
#define MENU_OBT 214

#define MENU_ABOUT 400

class GtkDasherWindow : public Gtk::Window
{
public:
  GtkDasherWindow();
  ~GtkDasherWindow();
  int destroy_event_impl(GdkEventAny *event);
  int delete_event_impl(GdkEventAny *event);

protected:
  GtkDasherPane dasher_pane;
  GtkDasherSave save_dialogue;
  Gtk::Toolbar toolbar;
  Gtk::VBox main_vbox; 
  Gtk::MenuBar menubar;

  Gtk::Pixmap first_pixmap;

  Gtk::Dialog aboutbox;

  void toolbar_button_cb(int c);
  void menu_button_cb(int c);
  void file_ok_sel();

  void reset();
  void save();
  void save_as();
  void exit();

  void select_all();
  void cut();
  void copy();
  void paste();
  void copy_all();

  void orientation( Opts::ScreenOrientations o );
};

#endif







