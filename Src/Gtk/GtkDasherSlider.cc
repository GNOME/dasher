#include "GtkDasherSlider.h"


#include <gtk--/adjustment.h>

GtkDasherSlider::GtkDasherSlider( CDasherInterface *_interface )
  : s( ), interface( _interface ), Frame( "Max bitrate (speed)" )
{
  a = manage( new Gtk::Adjustment( 5.0, 1.0, 8.0, 1.0, 1.0 ));
  
  set_border_width(2);

  s.set_adjustment(a);
  s.set_update_policy(GTK_UPDATE_CONTINUOUS);

  //  hb.pack_start( l, false, false, 8 );
  // hb.pack_start( s, true, true, 8 );

  add( s );

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
