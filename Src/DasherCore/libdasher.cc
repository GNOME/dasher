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
void (*colour_scheme_callback)(int, int*, int*, int*) = NULL;
void (*draw_rectangle_callback)(int, int, int, int, int, Opts::ColorSchemes) = NULL;
void (*draw_polyline_callback)(Dasher::CDasherScreen::point*, int) = NULL;
void (*draw_text_callback)(symbol, int, int, int) = NULL;
void (*draw_text_string_callback)(std::string, int, int, int) = NULL;
void (*text_size_callback)(symbol, int*, int*, int) = NULL;

void (*edit_output_callback)(symbol) = NULL;
void (*edit_outputcontrol_callback)(void*, int) = NULL;
void (*edit_delete_callback)() = NULL;
void (*get_new_context_callback)(std::string &, int ) = NULL;

void (*clipboard_callback)( clipboard_action ) = NULL;

bool (*get_bool_option_callback)(const std::string&, bool *) = NULL;
bool (*get_long_option_callback)(const std::string&, long *) = NULL;
bool (*get_string_option_callback)(const std::string&, std::string *) = NULL;
  
void (*set_bool_option_callback)(const std::string&, bool) = NULL;
void (*set_long_option_callback)(const std::string&, long) = NULL;
void (*set_string_option_callback)(const std::string&, const std::string&) = NULL;

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

void handle_colour_scheme(CCustomColours *Colours)
{
  if (colour_scheme_callback==NULL || Colours==NULL)
    return;

  int numcolours=Colours->GetNumColours();
  int* reds = new int[numcolours];
  int* greens = new int[numcolours];
  int* blues = new int[numcolours];
  for (int i=0; i<numcolours; i++) {
    reds[i]=Colours->GetRed(i);
    greens[i]=Colours->GetGreen(i);
    blues[i]=Colours->GetBlue(i);
  }
  colour_scheme_callback(numcolours,reds,greens,blues);
  delete reds;
  delete greens;
  delete blues;
}

void handle_draw_rectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme)
{
  if( draw_rectangle_callback != NULL ) {
    draw_rectangle_callback( x1, y1, x2, y2, Color, ColorScheme );
  }
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

void handle_draw_text(std::string String, int x1, int y1, int size)
{
  if( draw_text_string_callback != NULL )
    draw_text_string_callback( String, x1, y1, size );
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

void handle_edit_outputcontrol(void* pointer, int data)
{
  if( edit_outputcontrol_callback != NULL )
    edit_outputcontrol_callback( pointer, data );
}

void handle_edit_delete()
{
  if( edit_delete_callback != NULL )
    edit_delete_callback( );
}

void handle_get_new_context( std::string &str, int max )
{
  if( get_new_context_callback != NULL )
    get_new_context_callback( str, max );
}

void handle_clipboard( clipboard_action act )
{
  if( clipboard_callback != NULL )
    clipboard_callback( act );
}


bool handle_get_bool_option(const std::string& Key, bool *Value)
{
  //  cout << "Get bool_option_callback " << get_bool_option_callback << " " << &get_bool_option_callback << endl;

  if( get_bool_option_callback != NULL )
    return( get_bool_option_callback( Key, Value ) );
  else
    return( false );
}

bool handle_get_long_option(const std::string& Key, long *Value)
{
  if( get_long_option_callback != NULL )
    return( get_long_option_callback( Key, Value ) );
  else
    return( false );
}

bool handle_get_string_option(const std::string& Key, std::string *Value)
{
  if( get_string_option_callback != NULL )
    return( get_string_option_callback( Key, Value ) );
  else
    return( false );
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

// Initialisation and finalisation routines

using namespace std;

void dasher_early_initialise()
{
  interface = new CDasherInterface;
  
  
}

void dasher_late_initialise( int _width, int _height)
{
  const char *alphabet;

  dsc = new dasher_screen( _width, _height );
  interface->ChangeScreen( dsc );

  ded = new dasher_edit;
  interface->ChangeEdit( ded );

  dss = new dasher_settings_store;
  interface->SetSettingsStore( dss );

  dasher_start();
  dasher_redraw();

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

// Routines for the UI to request changes to parameters

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
    case STRING_COLOUR:
      interface->ChangeColours( s);
      break;
    case STRING_EDITFONT:
      interface->SetEditFont( s, 0 );
      break;
    case STRING_DASHERFONT:
      interface->SetDasherFont( s );
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
    case INT_ENCODING:
      interface->SetFileEncoding(Dasher::Opts::FileEncodingFormats(value));
      break;
    case INT_DASHERFONTSIZE:
      dasherfontsize=value;
      interface->SetDasherFontSize(Dasher::Opts::FontSize(value));
      break;
    case INT_UNIFORM:
      interface->SetUniform(value);
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
    case BOOL_EYETRACKER:
      interface->SetDasherEyetracker(value);
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
      interface->FixLayout(value);
      break;
    case BOOL_TIMESTAMPNEWFILES:
      interface->TimeStampNewFiles(value);
      break;
    case BOOL_COPYALLONSTOP:
      interface->CopyAllOnStop(value);
      break;
    case BOOL_DRAWMOUSE:
      interface->DrawMouse(value);
      break;
    case BOOL_STARTONSPACE:
      interface->StartOnSpace(value);
      break;
    case BOOL_STARTONLEFT:
      interface->StartOnLeft(value);
      break;
    case BOOL_KEYBOARDCONTROL:
      interface->KeyControl(value);
      break;
    case BOOL_WINDOWPAUSE:
      interface->WindowPause(value);
      break;
    case BOOL_CONTROLMODE:
      interface->ControlMode(value);
      break;
    case BOOL_COLOURMODE:
      interface->ColourMode(value);
      break;
    case BOOL_KEYBOARDMODE:
      interface->KeyboardMode(value);
      break;
    case BOOL_MOUSEPOSSTART:
      interface->MouseposStart(value);
      break;
    case BOOL_SPEECHMODE:
      interface->Speech(value);
      break;
    case BOOL_OUTLINEMODE:
      interface->OutlineBoxes(value);
      break;
    }
}

void dasher_set_orientation( Dasher::Opts::ScreenOrientations orient )
{
  interface->ChangeOrientation( orient );
}

void dasher_set_encoding( Dasher::Opts::FileEncodingFormats encoding )
{
  interface->SetFileEncoding( encoding );
}

void dasher_train_file( const char *filename )
{
  interface->TrainFile(filename);
}

const char* dasher_get_training_file()
{
  return (interface->GetTrainFile()).c_str();
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

int dasher_get_colours( const char **colourlist, int s )
{
  vector< string > alist;
  interface->GetColours( &alist );

  int i(0);
  vector<string>::iterator it( alist.begin() );

  while(( it != alist.end() ) && ( i < s ))
    {
      colourlist[i] = it->c_str();
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

void dasher_set_colour_scheme_callback( void (*_cb)(int, int*, int*, int*) )
{
  colour_scheme_callback = _cb;
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

void dasher_set_draw_text_string_callback(void (*_cb)(std::string, int, int, int))
{
  draw_text_string_callback = _cb;
}

void dasher_set_text_size_callback(void (*_cb)(symbol, int*, int*, int))
{
  text_size_callback = _cb;
}

void dasher_set_edit_output_callback( void (*_cb )(symbol))
{
  edit_output_callback = _cb;
}

void dasher_set_edit_outputcontrol_callback( void (*_cb )(void*, int))
{
  edit_outputcontrol_callback = _cb;
}

void dasher_set_edit_delete_callback( void (*_cb )())
{
  edit_delete_callback = _cb;
}

void dasher_set_get_new_context_callback( void (*_cb)( std::string &, int ) )
{
  get_new_context_callback = _cb;
}

void dasher_set_clipboard_callback( void (*_cb)( clipboard_action ) )
{
  clipboard_callback = _cb;
}

void dasher_set_get_bool_option_callback( bool (*_cb)(const std::string&, bool *) )
{
  //  cout << "Setting bool otion callback to " << &_cb << endl;
  get_bool_option_callback = _cb;
}

void dasher_set_get_long_option_callback( bool (*_cb)(const std::string&, long *) )
{
  get_long_option_callback = _cb;
}

void dasher_set_get_string_option_callback( bool (*_cb)(const std::string&, std::string *) )
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
 
void dasher_start()
{
  interface->Start();
}

void dasher_redraw()
{
  interface->Redraw();
}

void dasher_render()
{
  interface->Render();
}

void dasher_tap_on( int x, int y, unsigned long int time )
{
  interface->TapOn( x, y, time );
}

void dasher_go_to( int x, int y )
{
  interface->GoTo( x, y );
}

void dasher_draw_go_to( int x, int y )
{
  interface->DrawGoTo( x, y );
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

void add_control_tree(ControlTree *controltree)
{
  interface->AddControlTree(controltree);
}
