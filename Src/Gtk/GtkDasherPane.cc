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

#include <time.h>

GtkDasherPane::GtkDasherPane()
  : VBox( false, 0 ), paused( false )
{
  text = new GtkDasherEdit;
  text->set_usize(128, 128);
  text->thaw();

  canvas = new GtkDasherCanvas( 480, 480 );

  pack_start( *text );
  pack_start( *canvas );
  show_all();

  interface = new CDasherInterface;

  interface->ChangeLanguageModel(0);
  interface->ChangeView(0);

  std::vector< std::string > alphabetlist;
  interface->GetAlphabets( &alphabetlist );  

  interface->ChangeAlphabet( alphabetlist[0] );

  interface->ChangeEdit( text );
  interface->ChangeScreen( canvas );
 
  interface->Start();

  Gtk::Main::timeout.connect(slot(this,&GtkDasherPane::timer_callback),50);

  canvas->button_press_event.connect(slot(this, &GtkDasherPane::toggle_pause));
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
  if( !paused )
    {
      int x;
      int y;
      
      gdk_window_get_pointer(canvas->get_window(), &x, &y, NULL);
      interface->PauseAt(x,y);
    }
  else
    interface->Unpause( 0 );  // FIXME - need to specify a time here
  
  paused = !paused;

  return( true );
}

int GtkDasherPane::visibility_event_impl(GdkEventAny *event)
{
  cout << "In visibility event handler" << endl;

  return( true );
}



