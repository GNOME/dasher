#include "DasherInterface.h"
#include "SettingsStore.h"
#include "DashEdit.h"
#include "DasherTypes.h"

#include "libdasher.h"
#include "libdasher_private.h"

CDasherInterface *interface;
dasher_ui *dui;
dasher_screen *dsc;
//dasher_settings_store *dss;
CSettingsStore *dss;
dasher_edit *ded;

string default_string("");

// Function pointers for the callbacks

void (*string_callback)( string_param, const char * ) = NULL;
void (*double_callback)( double_param, double )= NULL;
void (*int_callback)( int_param, long int ) = NULL;
void (*bool_callback)( bool_param, bool ) = NULL;

void (*blank_callback)() = NULL;
void (*display_callback)() = NULL;
void (*draw_rectangle_callback)(int, int, int, int, int, Opts::ColorSchemes) = NULL;
void (*draw_polyline_callback)(Dasher::CDasherScreen::point*, int) = NULL;
void (*draw_text_callback)(symbol, int, int, int) = NULL;
void (*text_size_callback)(symbol, int*, int*, int) = NULL;

void (*edit_output_callback)(symbol) = NULL;
void (*edit_flush_callback)(symbol) = NULL;
void (*edit_unflush_callback)() = NULL;

void (*clipboard_callback)( clipboard_action ) = NULL;

bool (*get_bool_option_callback)(const std::string&) = NULL;
long (*get_long_option_callback)(const std::string&) = NULL;
string& (*get_string_option_callback)(const std::string& Key) = NULL;
  
void (*set_bool_option_callback)(const std::string&, bool) = NULL;
void (*set_long_option_callback)(const std::string&, long) = NULL;
void (*set_string_option_callback)(const std::string&, const std::string&) = NULL;
  
void (*set_bool_default_callback)(const std::string&, bool) = NULL;
void (*set_long_default_callback)(const std::string&, long) = NULL;
void (*set_string_default_callback)(const std::string&, const std::string&) = NULL;

void handle_parameter_string( string_param p, const string & value )
{
  if( string_callback != NULL )
    string_callback( p, value.c_str() );
};

void handle_parameter_double( double_param p, double value )
{
  if( double_callback != NULL )
    double_callback( p, value );
};

void handle_parameter_int( int_param p, long int value )
{
  if( int_callback != NULL )
    int_callback( p, value );
};

void handle_parameter_bool( bool_param p, bool value )
{
  if( bool_callback != NULL )
    bool_callback( p, value );
}

// Handlers for drawing callbacks

void handle_blank()
{
  if( blank_callback != NULL )
    blank_callback();
}

void handle_display()
{
  if( display_callback != NULL )
    display_callback();
}


void handle_draw_rectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme)
{
  if( draw_rectangle_callback != NULL )
    draw_rectangle_callback( x1, y1, x2, y2, Color, ColorScheme );
}

void handle_draw_polyline(Dasher::CDasherScreen::point* Points, int Number)
{
  if( draw_polyline_callback != NULL )
    draw_polyline_callback( Points, Number );
}

void handle_draw_text(symbol Character, int x1, int y1, int size)
{
  if( draw_text_callback != NULL )
    draw_text_callback( Character, x1, y1, size );
}
void handle_text_size(symbol Character, int* Width, int* Height, int Size)
{
  if( text_size_callback != NULL )
    text_size_callback(Character, Width, Height, Size);
}

void handle_edit_output(symbol Character)
{
  if( edit_output_callback != NULL )
    edit_output_callback( Character );
}

void handle_edit_flush(symbol Character)
{
  if( edit_flush_callback != NULL )
    edit_flush_callback( Character );
}

void handle_edit_unflush()
{
  if( edit_unflush_callback != NULL )
    edit_unflush_callback();
}

void handle_clipboard( clipboard_action act )
{
  if( clipboard_callback != NULL )
    clipboard_callback( act );
}


bool handle_get_bool_option(const std::string& Key)
{
  if( get_bool_option_callback != NULL )
    return( get_bool_option_callback( Key ) );
  else
    return( false );
}

long handle_get_long_option(const std::string& Key)
{
  if( get_long_option_callback != NULL )
    return( get_long_option_callback( Key ) );
  else
    return(0);
}

std::string& handle_get_string_option(const std::string& Key)
{
  if( get_string_option_callback != NULL )
    return( get_string_option_callback( Key ) );
  else
    return( default_string );
}

void handle_set_bool_option(const std::string& Key, bool Value)
{
  if( set_bool_option_callback != NULL )
    set_bool_option_callback( Key, Value );
}

void handle_set_long_option(const std::string& Key, long Value)
{
  if( set_long_option_callback != NULL )
    set_long_option_callback( Key, Value );
}

void handle_set_string_option(const std::string& Key, const std::string& Value)
{
  if( set_string_option_callback != NULL )
    set_string_option_callback( Key, Value );
}
  
void handle_set_bool_default(const std::string& Key, bool Value)
{
  if( set_bool_default_callback != NULL )
    set_bool_default_callback( Key, Value );
}

void handle_set_long_default(const std::string& Key, long Value)
{
  if( set_long_default_callback != NULL )
    set_long_default_callback( Key, Value );
}

void handle_set_string_default(const std::string& Key, const std::string& Value)
{
  if( set_string_default_callback != NULL )
    set_string_default_callback( Key, Value );
}

// Initialisation and finalisation routines

using namespace std;

void dasher_initialise( int _width, int _height )
{
  interface = new CDasherInterface;

  dsc = new dasher_screen( _width, _height );
  interface->ChangeScreen( dsc );

  ded = new dasher_edit();
  interface->ChangeEdit( ded );
    

  
  dasher_set_parameter_int( INT_LANGUAGEMODEL, 0 );
 
  dasher_set_parameter_int( INT_VIEW, 0 ); 
  const char *alphabet;
  dasher_get_alphabets( &alphabet, 1 );
  dasher_set_parameter_string( STRING_ALPHABET, alphabet );
  //  dasher_start();

//   const char *alphabet;
//   dasher_get_alphabets( &alphabet, 1 );
//   dasher_set_parameter_string( STRING_ALPHABET, alphabet );
 
  dss = new dasher_settings_store;
  interface->SetSettingsStore( dss );

  dasher_start();

  dui = new dasher_ui;
  interface->SetSettingsUI( dui );

  
}

void dasher_finalise()
{
  delete( interface );
  delete( dui );
  delete( dsc );
  delete( dss );
  delete( ded );
}

// Routines for the UI to request chnages to parameters

void dasher_set_parameter_string( string_param p, const char *value )
{
  string s( value );

  switch( p )
    {
    case STRING_USERDIR:
      interface->SetUserLocation( s );
      break;
    case STRING_SYSTEMDIR:
      interface->SetSystemLocation( s );
      break;
    case STRING_ALPHABET:
      interface->ChangeAlphabet( s );
      break;
    case STRING_EDITFONT:
      break;
    case STRING_DASHERFONT:
      break;
    }
}

void dasher_set_parameter_double( double_param p, double value )
{
  switch( p )
    {
    case DOUBLE_MAXBITRATE:
      interface->ChangeMaxBitRate(value);
      break;
    }
}


void dasher_set_parameter_int( int_param p, long int value )
{
  switch( p )
    {
    case INT_LANGUAGEMODEL:
      interface->ChangeLanguageModel( value );
      break;
    case INT_VIEW:
      interface->ChangeView( value );
      break;
    case INT_SCREENWIDTH:
      break;
    case INT_SCREENHEIGHT:
      break;
    case INT_EDITFONTSIZE:
      break;
    case INT_EDITHEIGHT:
      break;
    }
}

void dasher_set_parameter_bool( bool_param p, bool value )
{
  switch( p )
    {
    case BOOL_DIMENSIONS:
      interface->SetDasherDimensions(value);
      break;
    case BOOL_SHOWTOOLBAR:
      interface->ShowToolbar(value);
      break;
    case BOOL_SHOWTOOLBARTEXT:
      break;
    case BOOL_SHOWTOOLBARLARGEICONS:
      break;
    case BOOL_SHOWSPEEDSLIDER:
      interface->ShowSpeedSlider(value);
      break;
    case BOOL_FIXLAYOUT:
      break;
    case BOOL_TIMESTAMPNEWFILES:
      break;
    case BOOL_COPYALLONSTOP:
      break;
    case BOOL_DRAWMOUSE:
      interface->DrawMouse(value);
      break;
    case BOOL_STARTONSPACE:
      break;
    case BOOL_STARTONLEFT:
      break;
    }
}

void dasher_set_orientation( Dasher::Opts::ScreenOrientations orient )
{
  interface->ChangeOrientation( orient );
}

void dasher_train_file( const char *filename )
{
  interface->TrainFile(filename);
}

int dasher_get_alphabets( const char **alphabetlist, int s )
{
  vector< string > alist;
  interface->GetAlphabets( &alist );

  int i(0);
  vector<string>::iterator it( alist.begin() );

  while(( it != alist.end() ) && ( i < s ))
    {
      alphabetlist[i] = it->c_str();
      ++i;
      ++it;
    }

  return( i );
}

void dasher_set_string_callback( void(*_cb)( string_param, const char * ) )
{
  string_callback = _cb;
}

void dasher_set_double_callback( void(*_cb)( double_param, double ) )
{
  double_callback = _cb;
}

void dasher_set_int_callback( void(*_cb)( int_param, long int ) )
{
  int_callback = _cb;
}

void dasher_set_bool_callback( void(*_cb)( bool_param, bool ) )
{
  bool_callback = _cb;
}

void dasher_set_blank_callback( void (*_cb)() )
{
  blank_callback = _cb;
}

void dasher_set_display_callback( void (*_cb)() )
{
  display_callback =_cb;
}

void dasher_set_draw_rectangle_callback( void (*_cb)(int, int, int, int, int, Opts::ColorSchemes) )
{
  draw_rectangle_callback = _cb;
}

void dasher_set_draw_polyline_callback(void (*_cb)(Dasher::CDasherScreen::point*, int) )
{
  draw_polyline_callback = _cb;
}

void dasher_set_draw_text_callback(void (*_cb)(symbol, int, int, int))
{
  draw_text_callback = _cb;
}

void dasher_set_text_size_callback(void (*_cb)(symbol, int*, int*, int))
{
  text_size_callback = _cb;
}

void dasher_set_edit_output_callback( void (*_cb )(symbol))
{
  edit_output_callback = _cb;
}

void dasher_set_edit_flush_callback( void (*_cb )(symbol))
{
  edit_flush_callback = _cb;
}

void dasher_set_edit_unflush_callback( void (*_cb)() )
{
  edit_unflush_callback = _cb;
}

void dasher_set_clipboard_callback( void (*_cb)( clipboard_action ) )
{
  clipboard_callback = _cb;
}

void dasher_set_get_bool_option_callback( bool (*_cb)(const std::string&) )
{
  get_bool_option_callback = _cb;
}

void dasher_set_get_long_option_callback( long (*_cb)(const std::string&) )
{
  get_long_option_callback = _cb;
}

void dasher_set_get_string_option_callback( string& (*_cb)(const std::string& Key) )
{
  get_string_option_callback = _cb;
}
  
void dasher_set_set_bool_option_callback( void (*_cb)(const std::string&, bool) )
{
  set_bool_option_callback = _cb;
}

void dasher_set_set_long_option_callback( void (*_cb)(const std::string&, long) )
{
  set_long_option_callback = _cb;
}

void dasher_set_set_string_option_callback( void (*_cb)(const std::string&, const std::string&) )
{
  set_string_option_callback = _cb;
}
  
void dasher_set_set_bool_default_callback( void (*_cb)(const std::string&, bool) )
{
  set_bool_default_callback = _cb;
}

void dasher_set_set_long_default_callback( void (*_cb)(const std::string&, long) )
{
  set_long_default_callback = _cb;
}

void dasher_set_set_string_default_callback( void (*_cb)(const std::string&, const std::string&) )
{
  set_string_default_callback = _cb;
}




void dasher_start()
{
  interface->Start();
}

void dasher_redraw()
{
  interface->Redraw();
}

void dasher_tap_on( int x, int y, unsigned long int time )
{
  interface->TapOn( x, y, time );
}

void dasher_unpause( unsigned long int time )
{
  interface->Unpause( time );
}

void dasher_pause( int x, int y )
{
  interface->PauseAt( x, y );
}

// FIXME - this should not use c++ style strings

string dasher_get_display_text( symbol Character )
{
  return(interface->GetDisplayText(Character));
}

string dasher_get_edit_text( symbol Character )
{
  return(interface->GetEditText(Character));
}

void dasher_resize_canvas( int _width, int _height )
{
  delete( dsc );
  dsc = new dasher_screen( _width, _height );
  interface->ChangeScreen( dsc );
}

void dasher_copy()
{
  ded->Copy();
}

void dasher_cut()
{
  ded->Cut();
}

void dasher_paste()
{
  ded->Paste();
}

void dasher_copy_all()
{
  ded->CopyAll();
}

void dasher_select_all()
{
  ded->SelectAll();
}

void dasher_clear()
{
  ded->Clear();
}
