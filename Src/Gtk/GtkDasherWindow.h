#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <gtk--/window.h>
#include <gtk--/toolbar.h>
#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/menubar.h>
#include <gtk--/menu.h>
#include <gtk--/image.h>

//#include <gnome--/app.h>

#include "GtkDasherSave.h"
#include "GtkDasherPane.h"

#define TB_NEW "new"
#define TB_OPEN "open"
#define TB_SAVE "save"
#define TB_CUT "cut"
#define TB_COPY "copy"
#define TB_COPY_ALL "copy_all"
#define TB_PASTE "paste"

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

  void toolbar_button_cb(char *);
};

#endif







