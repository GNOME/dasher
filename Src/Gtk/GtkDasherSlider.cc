// GtkDasherSlider.cc
// (c) 2002 Philip Cowans

#include "GtkDasherSlider.h"


#include <gtk--/adjustment.h>

GtkDasherSlider::GtkDasherSlider( CDasherInterface *_interface )
  : s( ), interface( _interface ), Frame( "Maximum Speed (Bit Rate)" ), f()
{
  a = manage( new Gtk::Adjustment( 5.0, 1.0, 8.0, 1.0, 1.0 ));
  
  set_border_width(2);

  s.set_adjustment(a);
  s.set_update_policy(GTK_UPDATE_DISCONTINUOUS);

  f.add( s );
  
  f.set_border_width(2);
  f.set_shadow_type(GTK_SHADOW_NONE);

  add(f);

  a->value_changed.connect(bind(slot(this,&GtkDasherSlider::update_speed),a));

  show_all();
}

void GtkDasherSlider::update_speed( Gtk::Adjustment *adj )
{
  interface->ChangeMaxBitRate( adj->get_value() );
}

void GtkDasherSlider::move( double position )
{
  // Note - we should only move the slider if it isn't already in the
  // right position - otherwise we risk humorous loops

  if( a->get_value() != position )
    a->set_value(position);
}
