#include "GtkDasherSlider.h"


#include <gtk--/adjustment.h>

GtkDasherSlider::GtkDasherSlider( CDasherInterface *_interface )
  : HBox(), l("Speed:"), s( ), interface( _interface )
{
  Gtk::Adjustment *a = manage( new Gtk::Adjustment( 2.0, 1.0, 5.0 ));
  
  s.set_adjustment(a);
  s.set_update_policy(GTK_UPDATE_CONTINUOUS);

  pack_start( l );
  pack_start( s );

  //Gtk::Adjustment *adj2=manage(new Gtk::Adjustment(1.0, 1.0, 101.0));
  // Gtk::HScale *s=manage(new Gtk::HScale(*adj2));

  a->value_changed.connect(bind(slot(this,&GtkDasherSlider::update_speed),a));

  show_all();
}

void GtkDasherSlider::update_speed( Gtk::Adjustment *adj )
{
  cout << "Update: " << adj->get_value() << endl;

  interface->ChangeMaxBitRate( adj->get_value() );
}
