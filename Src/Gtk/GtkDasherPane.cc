#include <iostream.h>
#include <string>
#include <vector>

#include <gtk--/text.h>
#include <gtk--/main.h>
#include <gdk/gdk.h>
#include <gtk--/frame.h>

#include "GtkDasherPane.h"
#include "GtkDasherCanvas.h"
#include "GtkDasherStore.h"
#include "GtkDasherEdit.h"
#include "GtkDasherSlider.h"

#include "DasherInterface.h"
#include "SettingsStore.h"
#include "DasherSettingsInterface.h"

#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#include <iostream.h>
#include <fstream.h>

GtkDasherPane::GtkDasherPane( Dasher::CDasherSettingsInterface *setif )
  : VBox( false, 0 ), paused( true ), started( false ), vp()
{ 

  store = new GtkDasherStore;

  char *HomeDir;

  HomeDir = getenv( "HOME" );

  char *UserDataDir;

  UserDataDir = new char[ strlen( HomeDir ) + 10 ];
  sprintf( UserDataDir, "%s/.dasher/", HomeDir );
  
  char *SystemDataDir;

  SystemDataDir = "/etc/dasher/";

  interface = new CDasherInterface;

  interface->SetSystemLocation(SystemDataDir);
  interface->SetUserLocation(UserDataDir);
  interface->SetSettingsStore( store );

  text = new GtkDasherEdit( interface );
  text->set_usize(128, 128);

  slider = new GtkDasherSlider( interface );
  
  canvas = new GtkDasherCanvas( 360, 360, interface );

  vp.pack1( *text, true, true );
  vp.pack2( *canvas, true, true );

  pack_start( vp, true, true );
  pack_start( *slider, false, false );
  show_all();

  interface->ChangeLanguageModel(0);
  interface->ChangeView(0);

  std::vector< std::string > alphabetlist;
  interface->GetAlphabets( &alphabetlist );  

  abox.AddAlphabet( alphabetlist );

  interface->ChangeAlphabet( alphabetlist[0] );

  interface->ChangeEdit( text );
  interface->ChangeScreen( canvas->get_wrapper() );

  Gtk::Main::timeout.connect(slot(this,&GtkDasherPane::timer_callback),50);

  canvas->button_press_event.connect(slot(this, &GtkDasherPane::toggle_pause));


  abox.get_ok_button()->clicked.connect(slot(this, &GtkDasherPane::handle_alphabet));
  abox.get_cancel_button()->clicked.connect(slot(this, &GtkDasherPane::handle_alphabet_cancel));
}

void GtkDasherPane::clear()
{
  canvas->clear();
  interface->Redraw();
}

void GtkDasherPane::reset()
{
  text->Clear();
  interface->Start();
  paused = true;
  started = false;
  clear();
  // text->Clear();
}

void GtkDasherPane::set_settings_ui( Dasher::CDasherSettingsInterface *settingsif )
{
  interface->SetSettingsUI( settingsif );
}

void GtkDasherPane::save()
{
}

void GtkDasherPane::show_alphabet_box()
{
  abox.show();
}

bool GtkDasherPane::open( std::string filename )
{
  return(text->Open(filename));
}

void GtkDasherPane::import( std::string filename )
{
  cout << "Import: " << filename << endl;
  interface -> TrainFile( filename );
}

bool GtkDasherPane::save_as( string filename )
{
  //  cout << "In Save: " << filename << endl;

  return(text->SaveAs( filename, false ));
}

bool GtkDasherPane::append( string filename )
{
  return(text->SaveAs( filename, true ));
}

void GtkDasherPane::select_all()
{
  text->SelectAll();
}

void GtkDasherPane::cut()
{
  text->Cut();
}

void GtkDasherPane::copy()
{
  text->Copy();
}

void GtkDasherPane::paste()
{
  text->Paste();
}

void GtkDasherPane::copy_all()
{
  text->CopyAll();
}

void GtkDasherPane::orientation( Opts::ScreenOrientations o )
{
  interface->ChangeOrientation( o );
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

      interface->TapOn(x,y, get_time() );
    }

  return( 1 );
}

void GtkDasherPane::set_dasher_font( string fontname )
{
  interface->SetDasherFont( fontname );
}

void GtkDasherPane::set_edit_font( string fontname, long size )
{
  interface->SetEditFont( fontname, size );
}

void GtkDasherPane::show_speed_slider( bool value )
{
  interface->ShowSpeedSlider( value );
}

void GtkDasherPane::show_toolbar( bool value )
{
  interface->ShowToolbar( value );
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
    interface->Unpause( get_time() );
  
  paused = !paused;
  return( true );
}

int GtkDasherPane::visibility_event_impl(GdkEventAny *event)
{
  cout << "In visibility event handler" << endl;

  return( true );
}

void GtkDasherPane::show_slider( bool s )
{
  if( s )
    slider->show();
  else
    slider->hide();
}

void GtkDasherPane::move_slider( double position )
{
  slider->move( position );
}

void GtkDasherPane::copy_all_on_pause( bool s )
{
  interface->CopyAllOnStop( s );
}

void GtkDasherPane::fix_pane( bool value )
{
  interface->FixLayout( value );
}

void GtkDasherPane::fix( bool value )
{
  // vp.set_sensitive( !value );  // This doesn't work
}

void GtkDasherPane::timestamp( bool value )
{
  interface->TimeStampNewFiles( value );
}

long GtkDasherPane::get_time()
{
  long s_now;
  long ms_now;
  
  struct timeval tv;
  struct timezone tz;
  
  gettimeofday( &tv, &tz );
  
  s_now = tv.tv_sec;
  ms_now = tv.tv_usec / 1000;
  
  return( s_now * 1000 + ms_now );
}

void GtkDasherPane::handle_alphabet()
{
  abox.hide();

  std::string foo;

  foo = abox.get_selection();

  cout << "Selection is " << foo << endl;

  text->kill_flush();
  
  interface->ChangeAlphabet( foo );
}

void GtkDasherPane::handle_alphabet_cancel()
{
  abox.hide();
}
