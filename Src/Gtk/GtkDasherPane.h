#ifndef DASHER_PANE_HH
#define DASHER_PANE_HH

#include <gtk--/box.h>
#include <gtk--/text.h>
#include <gtk--/paned.h>

#include "GtkDasherCanvas.h"
#include "GtkDasherEdit.h"
#include "GtkDasherStore.h"
#include "GtkDasherSlider.h"

#include "DasherInterface.h"
#include "SettingsStore.h"
#include "DasherSettingsInterface.h"

#include <time.h>
#include <string.h>

using namespace Dasher;

class GtkDasherPane : public Gtk::VBox
{
public:
  GtkDasherPane( Dasher::CDasherSettingsInterface *setif );
  ~GtkDasherPane();

  void clear();

  void reset();
  void save();
  bool save_as( string filename );
  bool open( string filename );
  bool append( string filename );
  void import( string filename );

  void select_all();
  void cut();
  void copy();
  void paste();
  void copy_all();

  void orientation( Opts::ScreenOrientations o );
  void set_settings_ui( Dasher::CDasherSettingsInterface *setingsif );

  void set_dasher_font( string fontname );
  void set_edit_font( string fontname, long size );
  void show_speed_slider( bool value );  // Tell interface we wish to show slider
  void show_slider( bool s ); // Actually do it
  void move_slider( double position );

  void show_toolbar( bool value );

  void fix_pane( bool value ); // Notify of fix

  void fix( bool value ); // Actually do it

  void timestamp( bool value );
  
  void copy_all_on_pause( bool s );
private:
  GtkDasherEdit *text;
  GtkDasherCanvas *canvas;
  
  //  CSettingsStore *store;
  GtkDasherStore *store;

  Gtk::VPaned vp;

  CDasherInterface *interface;

  GtkDasherSlider *slider;

  gint timer_callback();

  int toggle_pause( GdkEventButton *e);

  int visibility_event_impl(GdkEventAny *event);
  long get_time();
  bool paused;
  bool started;

  long s_pause;
  long ms_pause;

};

#endif
