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
  void save( string filename );

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
