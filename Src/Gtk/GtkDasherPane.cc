#include <iostream.h>
#include <string>
#include <vector>

#include <gtk--/text.h>
#include <gtk--/main.h>
#include <gdk/gdk.h>

#include "GtkDasherPane.h"
#include "GtkDasherCanvas.h"
#include "GtkDasherStore.h"
#include "GtkDasherEdit.h"

#include "DasherInterface.h"
#include "SettingsStore.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <iostream.h>
#include <fstream.h>

GtkDasherPane::GtkDasherPane()
  : VBox( false, 0 ), paused( true ), started( false )
{ 

  store = new GtkDasherStore;

  char *HomeDir;

  HomeDir = getenv( "HOME" );

  char *UserDataDir;

  UserDataDir = new char[ strlen( HomeDir ) + 10 ];
  sprintf( UserDataDir, "%s/.dasher/", HomeDir );
  
  char *SystemDataDir;

  SystemDataDir = "/etc/dasher/";

  cout << "User configuration directory is " << UserDataDir << endl;
  cout << "System configuration directory is " << SystemDataDir << endl;

  interface = new CDasherInterface;

  interface->SetSystemLocation(SystemDataDir);
  interface->SetUserLocation(UserDataDir);
  interface->SetSettingsStore( store );


  text = new GtkDasherEdit( interface );
  text->set_usize(128, 128);
  text->thaw();
 
  canvas = new GtkDasherCanvas( 480, 480, interface );

  pack_start( *text );
  pack_start( *canvas );
  show_all();

  interface->ChangeLanguageModel(0);
  interface->ChangeView(0);

  std::vector< std::string > alphabetlist;
  interface->GetAlphabets( &alphabetlist );  

  interface->ChangeAlphabet( alphabetlist[0] );

 
   // Load in training data here

  char training_file[ strlen( SystemDataDir) + 10 ];

  sprintf( training_file, "%strain.txt", SystemDataDir );

  //  ifstream t( "/etc/dasher/train.txt" );

  ifstream t( training_file );

  if( !t.bad() )
    {
      cout << "Training ... " << flush;

      char b[256];
      
      while( !t.eof() )
      {
      int i(0);

      while( (i < 255) && ( !t.eof() ) )
	{
	  t.read( &b[i], 1 );
	  ++i;
	}

      b[i] = 0;

      string s( b );

      if( !t.eof() )
	interface->Train( &s, true );
      else
	interface->Train( &s, false );
    }

      cout << "done." << endl;
    }

  // interface->Redraw();
  //  interface->Start();

 interface->ChangeEdit( text );
  interface->ChangeScreen( canvas );

  Gtk::Main::timeout.connect(slot(this,&GtkDasherPane::timer_callback),50);

  canvas->button_press_event.connect(slot(this, &GtkDasherPane::toggle_pause));
}

void GtkDasherPane::clear()
{
  canvas->clear();
  interface->Redraw();
}

void GtkDasherPane::reset()
{
  cout << "In Reset" << endl;
  interface->Start();
  paused = true;
  // text->

  // FIXME - need to reset the contents of the edit box here too

  clear();
}

void GtkDasherPane::save( string filename )
{
  //  cout << "In Save: " << filename << endl;

  //  text->save( filename );
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

      //      cout << "x: " << x << " y: " << y << endl;
      
      interface->TapOn(x,y,50);
      
      // We need to return a nonzero value so that the timer repeats
    }
  return( 1 );
}

int GtkDasherPane::toggle_pause( GdkEventButton *e )
{
  cout << "In toggle_pause" << endl;

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



