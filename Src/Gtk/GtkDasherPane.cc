// GtkDasherPane.cc
// (c) 2002 Philip Cowans

#include <iostream>
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

#include <fstream>

#include <iconv.h>

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

  SystemDataDir = PROGDATA"/"; // Set by configure

  interface = new CDasherInterface;

  interface->SetSystemLocation(SystemDataDir);
  interface->SetUserLocation(UserDataDir);
  interface->SetSettingsStore( store );

  text = new GtkDasherEdit( interface );
  text->set_usize(128, 128);

  slider = new GtkDasherSlider( interface );
  
  canvas = new GtkDasherCanvas( 360, 360, interface );

  canvasframe.set_border_width( 2 );
  canvasframe.set_shadow_type( GTK_SHADOW_IN );
  canvasframe.add( *canvas );

  editframe.set_border_width( 1 );
  editframe.set_shadow_type( GTK_SHADOW_NONE );
  editframe.add( *text );

  vp.pack1( editframe, true, true );

  vp.pack2( canvasframe, true, true );

  pack_start( vp, true, true );
  pack_start( *slider, false, false );
  show_all();

  interface->ChangeScreen( canvas->get_wrapper() );

  //  interface->ChangeLanguageModel(0); // The core seems to deal with this
  interface->ChangeView(0);

  std::vector< std::string > alphabetlist;
  interface->GetAlphabets( &alphabetlist );  

  abox.AddAlphabet( alphabetlist );

  //  interface->ChangeAlphabet( alphabetlist[0] );

  interface->ChangeEdit( text );

  Gtk::Main::timeout.connect(slot(this,&GtkDasherPane::timer_callback),50);

  canvas->button_press_event.connect(slot(this, &GtkDasherPane::toggle_pause));


  abox.get_ok_button()->clicked.connect(slot(this, &GtkDasherPane::handle_alphabet));
  abox.get_cancel_button()->clicked.connect(slot(this, &GtkDasherPane::handle_alphabet_cancel));
  abox.delete_event.connect( SigC::slot(this, &GtkDasherPane::abox_close_sel) );

}

void GtkDasherPane::clear()
{
  canvas->clear();
  interface->Redraw();
  GdkCursor* cursor = gdk_cursor_new(GDK_CROSSHAIR);
  gdk_window_set_cursor(canvas->get_window(), cursor);
  gdk_cursor_destroy(cursor);
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
  interface -> TrainFile( filename );
}

bool GtkDasherPane::save_as( string filename )
{
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

void GtkDasherPane::encoding( Opts::FileEncodingFormats e )
{
  interface->SetFileEncoding( e );
}

GtkDasherPane::~GtkDasherPane()
{
   delete( text );
   delete( canvas );
   delete( interface );
}

gint GtkDasherPane::timer_callback()
{

  if( !canvas->Paused() )
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

void GtkDasherPane::set_dasher_font_size( FontSize size )
{
  interface->SetDasherFontSize( size );
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
  if( !(canvas->Paused()) )
    {
      int x;
      int y;
      
      gdk_window_get_pointer(canvas->get_window(), &x, &y, NULL);
      interface->PauseAt(x,y);
    }
  else
    interface->Unpause( get_time() );
  
  canvas->toggle_paused();
  return( true );
}

int GtkDasherPane::visibility_event_impl(GdkEventAny *event)
{
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

  std::string _alphabet( abox.get_selection() );

  if( _alphabet != "" )
    interface->ChangeAlphabet( _alphabet );
}


void GtkDasherPane::handle_alphabet_cancel()
{
  abox.hide();
}

void GtkDasherPane::select_encoding()
{
  // Tries to guess an encoding based on the range of unicode
  // characters

  int maxcount(0);
  int maxpage(-1);

  for( int i(1); i < 15; ++i )
    {
      // Cycle through the pages

      char encstr[256];

      snprintf( encstr, 255, "ISO-8859-%d", i );

      iconv_t cdesc;
     
      cdesc = iconv_open( encstr, "UTF-8" );

      int count(0);

      for( int a(1); a < interface->GetNumberSymbols(); ++a )
	{
	  string symbol;
	  symbol = interface->GetDisplayText(a,false);
	  
	  char *convbuffer = new char[256];
	  char *inbuffer = new char[256];
	  
	  char *cb( convbuffer );
	  char *ib( inbuffer );
	  
	  strncpy( inbuffer, symbol.c_str(), 255 );
	  
	  size_t inb = symbol.length();
	  
	  size_t outb = 256;
	  
	  iconv( cdesc, &inbuffer, &inb, &convbuffer, &outb );
	  
	  string csymbol( cb, 256-outb );
	  
	  delete cb;
	  delete ib;

	  if( outb < 256 )
	    ++count;
	}

      if( count > maxcount )
	{
	  maxcount = count;
	  maxpage = i;
	}

      iconv_close( cdesc );
    }
  canvas->set_encoding( maxpage );
  text->set_display_encoding( maxpage );

}

void GtkDasherPane::change_alphabet( std::string _alphabet )
{
  text->kill_flush();
  select_encoding();

  abox.set_selection( _alphabet );
}

gint GtkDasherPane::abox_close_sel( GdkEventAny *e )
{
  abox.hide();
  return( true );
}

gint GtkDasherPane::key_press_event_impl( GdkEventKey *e )
{
  std::cout << "Received keypress" << std::endl;
}

std::string GtkDasherPane::get_current_filename()
{
  return( text->get_current_filename() );
}

bool GtkDasherPane::is_dirty()
{
  return( text->is_dirty() );
}
