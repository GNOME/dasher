#ifndef DASHER_PANE_HH
#define DASHER_PANE_HH

#include <gtk--/box.h>
#include <gtk--/text.h>

#include "GtkDasherCanvas.h"
#include "GtkDasherEdit.h"
#include "GtkDasherStore.h"

#include "DasherInterface.h"
#include "SettingsStore.h"

#include <time.h>
#include <string.h>

using namespace Dasher;

class GtkDasherPane : public Gtk::VBox
{
public:
  GtkDasherPane();
  ~GtkDasherPane();

  void clear();

  void reset();
  void save();
  void save_as( string filename );

  void select_all();
  void cut();
  void copy();
  void paste();
  void copy_all();

  void orientation( Opts::ScreenOrientations o );

  void set_dasher_font( string fontname );
  void set_edit_font( string fontname, long size );

private:
  GtkDasherEdit *text;
  GtkDasherCanvas *canvas;
  
  //  CSettingsStore *store;
  GtkDasherStore *store;

  CDasherInterface *interface;

  gint timer_callback();

  int toggle_pause( GdkEventButton *e);

  int visibility_event_impl(GdkEventAny *event);

  bool paused;
  bool started;
};

#endif
