#include <iostream.h>
#include <string>
#include <vector>

#include <gtk--/text.h>
#include <gtk--/main.h>
#include <gdk/gdk.h>

#include "GtkDasherPane.h"
#include "GtkDasherCanvas.h"
#include "GtkDasherEdit.h"

#include "DasherInterface.h"
#include "SettingsStore.h"

GtkDasherPane::GtkDasherPane()
  : VBox( false, 0 ), paused( false )
{
  text = new GtkDasherEdit;
  text->set_usize(128, 128);
  text->thaw();

  canvas = new GtkDasherCanvas( 480, 480 );

  pack_start( *text );
  pack_start( *canvas );

  interface = new CDasherInterface;

  cout << "Canvas is " << canvas << endl;

  std::vector< std::string > alphabetlist;
  
  interface->GetAlphabets( &alphabetlist );

  cout << "Alphabet count " << alphabetlist.size() << " " << alphabetlist[0] << endl;

  interface->ChangeLanguageModel(0);
  interface->ChangeView(0);

  interface->ChangeAlphabet( alphabetlist[0] );

  // interface->ChangeScreen( canvas );
  interface->ChangeEdit( text );
   interface->ChangeScreen( canvas );
  interface->Start();
  interface->Unpause(0);
  cout << "Foo" << endl;
  interface->Redraw();

  show_all();

  //  SigC::Slot0<gint> my_slot = bind(slot(this,&GtkDasherPane::timer_callback),0);

  // now connect the slot to Gtk::Main::timeout
  Gtk::Connection conn = Gtk::Main::timeout.connect(slot(this,&GtkDasherPane::timer_callback),50);


  button_press_event.connect(slot(this, &GtkDasherPane::toggle_pause));
}

void GtkDasherPane::clear()
{
  canvas->clear();
  interface->Redraw();
}

GtkDasherPane::~GtkDasherPane()
{
   delete( text );
   delete( canvas );
   delete( interface );
}

gint GtkDasherPane::timer_callback()
{

  if( !paused )
    {
  int x;
  int y;

   gdk_window_get_pointer(canvas->get_window(), &x, &y, NULL);

  interface->TapOn(x,y,50);

  // We need to return a nonzero value so that the timer repeats
    }
  return( 1 );
}

int GtkDasherPane::toggle_pause( GdkEventButton *e )
{
  cout << "In toggle_pause" << endl;

  //  interface->PauseAt(0,0);
  paused = !paused;
}





