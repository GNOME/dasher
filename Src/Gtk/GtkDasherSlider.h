#ifndef DASHER_SLIDER_H
#define DASHER_SLIDER_H

#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/scale.h>

#include "DasherInterface.h"

class GtkDasherSlider : public Gtk::HBox
{
 public:
  GtkDasherSlider(CDasherInterface *_interface );

  void move( double position );

 private:
  Gtk::Label l;
  Gtk::HScale s;
  
  void update_speed( Gtk::Adjustment *adj );

  CDasherInterface *interface;
  Gtk::Adjustment *a;
};

#endif
