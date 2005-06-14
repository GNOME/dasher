#include "DasherInterface.h"
#include "DasherSettingsInterface.h"
#include "CustomColours.h"

using namespace Dasher;
using namespace std;

void handle_parameter_string( string_param p, const string & value );
void handle_parameter_double( double_param p, double value );
void handle_parameter_int( int_param p, long int value );
void handle_parameter_bool( bool_param p, bool value );

void handle_blank();
void handle_display();
void handle_colour_scheme(const CCustomColours *Colours);

void handle_draw_rectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme);
void handle_draw_polyline(Dasher::CDasherScreen::point* Points, int Number);
void handle_draw_colour_polyline(Dasher::CDasherScreen::point* Points, int Number, int Colour);
void handle_draw_colour_polygon(Dasher::CDasherScreen::point* Points, int Number, int Colour);
void handle_draw_text(symbol Character, int x1, int y1, int size);
void handle_draw_text(const std::string &String, int x1, int y1, int size);
void handle_text_size(const std::string &String, int* Width, int* Height, int Size);
void handle_edit_output(symbol Character);
void handle_edit_outputcontrol(void* pointer, int data);
void handle_edit_delete(symbol Character);
void handle_get_new_context( std::string &str, int max );
void handle_send_marker( int iMarker );

void handle_clipboard( clipboard_action act );

bool handle_get_bool_option(const std::string& Key, bool *Value);
bool handle_get_long_option(const std::string& Key, long *Value);
bool handle_get_string_option(const std::string& Key, std::string *Value);
void handle_set_bool_option(const std::string& Key, bool Value);
void handle_set_long_option(const std::string& Key, long Value);
void handle_set_string_option(const std::string& Key, const std::string& Value);

int dasherfontsize=0;

class dasher_ui : public CDasherSettingsInterface
{
 public:

  void ChangeAlphabet(const string& NewAlphabetID) 
    { 
      handle_parameter_string( STRING_ALPHABET, NewAlphabetID ); 
    };

  void ChangeColour(const string& NewColourID)
    {
      handle_parameter_string( STRING_COLOUR, NewColourID );
    };

  void ChangeMaxBitRate(double NewMaxBitRate) 
    { 
      handle_parameter_double( DOUBLE_MAXBITRATE, NewMaxBitRate ); 
    };

  void ChangeLanguageModel(unsigned int NewLanguageModelID) 
    {
      handle_parameter_int( INT_LANGUAGEMODEL, NewLanguageModelID );
    };

  void ChangeLMOption( const std::string &pname, long int Value )
    {
      // FIXME - all this should be much more flexible

      if( pname == "LMMaxOrder" ) {
	handle_parameter_int( INT_LM_MAXORDER, Value ); 
      }
      else if( pname == "LMAlpha" ) {
	handle_parameter_int( INT_LM_ALPHA, Value ); 
      }
      else if( pname == "LMBeta" ) {
	handle_parameter_int( INT_LM_BETA, Value ); 
      }
      else if( pname == "LMExclusion" ) {
	handle_parameter_int( INT_LM_EXCLUSION, Value ); 
      }
      else if( pname == "LMUpdateExclusion" ) {
	handle_parameter_int( INT_LM_UPDATE_EXCLUSION, Value ); 
      }
      else if( pname == "LMMixture" ) {
	handle_parameter_int( INT_LM_MIXTURE, Value ); 
      }


    };

  void ChangeView(unsigned int NewViewID)
    {
      handle_parameter_int( INT_VIEW, NewViewID );
    };

  void ChangeOrientation(Opts::ScreenOrientations Orientation) 
    {
      handle_parameter_int( INT_ORIENTATION, Orientation );
    };

  void SetFileEncoding(Opts::FileEncodingFormats Encoding) 
    {
      handle_parameter_int( INT_ENCODING, Encoding );
    };

  void SetScreenSize(long Width, long Height) 
    {
      handle_parameter_int( INT_SCREENWIDTH, Width );
      handle_parameter_int( INT_SCREENHEIGHT, Height );
    };

  void SetDasherFontSize(Dasher::Opts::FontSize fontsize) 
    {
      handle_parameter_int( INT_DASHERFONTSIZE, fontsize );
    };

  void SetDasherDimensions(bool Value) 
    {
      handle_parameter_bool( BOOL_DIMENSIONS, Value );
    };

  void SetDasherEyetracker(bool Value) 
    {
      handle_parameter_bool( BOOL_EYETRACKER, Value );
    };

  void ShowToolbar(bool Value)
    {
      handle_parameter_bool( BOOL_SHOWTOOLBAR, Value );
    };

  void ShowToolbarText(bool Value)
    {
      handle_parameter_bool( BOOL_SHOWTOOLBARTEXT, Value );
    };

  void ShowToolbarLargeIcons(bool Value)
    {
      handle_parameter_bool( BOOL_SHOWTOOLBARLARGEICONS, Value );
    };

  void ShowSpeedSlider(bool Value) 
    {
      handle_parameter_bool( BOOL_SHOWSPEEDSLIDER, Value );
    };

  void FixLayout(bool Value)
    {
      handle_parameter_bool( BOOL_FIXLAYOUT, Value );
    };

  void TimeStampNewFiles(bool Value) 
    {
      handle_parameter_bool( BOOL_TIMESTAMPNEWFILES, Value );
    };

  void CopyAllOnStop(bool Value) 
    {
      handle_parameter_bool( BOOL_COPYALLONSTOP, Value );
    };

  void DrawMouse(bool Value) 
    {
      handle_parameter_bool( BOOL_DRAWMOUSE, Value );
    };

  void DrawMouseLine(bool Value) 
    {
      handle_parameter_bool( BOOL_DRAWMOUSELINE, Value );
    };

  void SetEditFont(string Name, long Size) 
    {
      handle_parameter_string( STRING_EDITFONT, Name );
      //      handle_parameter_int( INT_EDITFONTSIZE, Size );
    };

  void SetDasherFont(string Name)
    {
      handle_parameter_string( STRING_DASHERFONT, Name );
    };

  void SetEditHeight(long Value) 
    {
      handle_parameter_int( INT_EDITHEIGHT, Value );
    };

  void StartOnSpace(bool Value)
    {
      handle_parameter_bool( BOOL_STARTONSPACE, Value );
    };

  void StartOnLeft(bool Value)
    {
      handle_parameter_bool( BOOL_STARTONLEFT, Value );
    };
  void KeyControl(bool Value)
    {
      handle_parameter_bool( BOOL_KEYBOARDCONTROL, Value );
    };
  void WindowPause(bool Value)
    {
      handle_parameter_bool( BOOL_WINDOWPAUSE, Value );
    };
  void ControlMode(bool Value)
    {
      handle_parameter_bool( BOOL_CONTROLMODE, Value );
    };
  void ColourMode(bool Value)
    {
      handle_parameter_bool( BOOL_COLOURMODE, Value );
    };
  void KeyboardMode(bool Value)
    {
      handle_parameter_bool( BOOL_KEYBOARDMODE, Value );
    };
  void MouseposStart(bool Value)
    {
      handle_parameter_bool( BOOL_MOUSEPOSSTART, Value );
    };
  void SetUniform(int Value)
    {
      handle_parameter_int( INT_UNIFORM, Value );
    }
  void Speech(bool Value)
    {
      handle_parameter_bool( BOOL_SPEECHMODE, Value );
    }
  void OutlineBoxes(bool Value)
    {
      handle_parameter_bool( BOOL_OUTLINEMODE, Value );
    }
  void PaletteChange(bool Value)
    {
      handle_parameter_bool( BOOL_PALETTECHANGE, Value );
    }
  void SetTruncation( int Value ) {
    handle_parameter_int( INT_TRUNCATION, Value );
  }
  void SetTruncationType( int Value ) {
    handle_parameter_int( INT_TRUNCATION, Value );
  }
};


class dasher_screen : public CDasherScreen
{
 public:

  dasher_screen( screenint _width, screenint _height )
    : CDasherScreen( _width, _height )
    {
    };

  void SetFont(std::string Name)
    {
    };

  void SetFontSize(Dasher::Opts::FontSize fontsize)
    {
    };
	
  Dasher::Opts::FontSize GetFontSize() const
    {
      if (dasherfontsize==0)
	return Dasher::Opts::FontSize(1);
      else
	return Dasher::Opts::FontSize(dasherfontsize);
    };

  void TextSize(const std::string &String, screenint* Width, screenint* Height, int Size) const
    {
      handle_text_size( String, Width, Height, Size );
    };

  void DrawString(symbol Character, screenint x1, screenint y1, int Size) const
    {
      // It's possible that this is no longer used...

      handle_draw_text( Character, x1, y1, Size );
    };

  void DrawString(const std::string &String, screenint x1, screenint y1, int Size) const
    {
      handle_draw_text( String, x1, y1, Size );
    };

  void DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, Opts::ColorSchemes ColorScheme) const
    {
      handle_draw_rectangle( x1, y1, x2, y2, Color, ColorScheme);
    };

  void SendMarker( int iMarker ) const {
    handle_send_marker( iMarker );
  };
	
  void Polyline(point* Points, int Number) const
    {
      handle_draw_polyline( Points, Number );
    };

  void Polyline(point* Points, int Number, int Colour) const
    {
      handle_draw_colour_polyline( Points, Number, Colour );
    };

  void Polygon(point* Points, int Number, int Colour) const
    {
      handle_draw_colour_polygon( Points, Number, Colour );
    };

  void DrawPolygon(point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const
    {
    };
	
  void Blank() const
    {
      handle_blank();
    };

  void Display()
    {
      handle_display();
    };

  void SetColourScheme(const CCustomColours *Colours)
    {
      handle_colour_scheme(Colours);
    };
};

class dasher_settings_store : public CSettingsStore
{
 private:
  bool LoadSetting(const std::string& Key, bool* Value)
    {
      return( handle_get_bool_option( Key, Value ) );
    };

  bool LoadSetting(const std::string& Key, long* Value)
    {
      return( handle_get_long_option( Key, Value ) );
    };

  bool LoadSetting(const std::string& Key, std::string* Value)
    {
      return( handle_get_string_option( Key, Value ) );
    };

  virtual void SaveSetting(const std::string& Key, bool Value)
    {
      handle_set_bool_option( Key, Value );
    };

  void SaveSetting(const std::string& Key, long Value)
    {
      handle_set_long_option( Key, Value );
    };

  void SaveSetting(const std::string& Key, const std::string& Value)
    {
      handle_set_string_option( Key, Value );
    };
};

class dasher_edit : public CDashEditbox
{
 public:
  void write_to_file() {};
  void get_new_context(std::string& str, int max)
    {
      handle_get_new_context( str, max );
    }
  void output(symbol Symbol) 
    {
      handle_edit_output(Symbol);
    };
  void outputcontrol(void* pointer, int data, int type)
    {
      handle_edit_outputcontrol(pointer, data);
    };
  void deletetext(symbol Symbol)
    {
      handle_edit_delete(Symbol);
    };
  void Clear()
    {
      handle_clipboard( CLIPBOARD_CLEAR );
    };
  void SetEncoding(Opts::FileEncodingFormats Encoding) {};
  void SetFont(std::string Name, long Size) {};
  void Cut()
    {
      handle_clipboard( CLIPBOARD_CUT );
    };
  void Copy() 
    {
      handle_clipboard( CLIPBOARD_COPY );
    };
  void Paste() 
    {
      handle_clipboard( CLIPBOARD_PASTE );
    };
  void CopyAll()
    {
      handle_clipboard( CLIPBOARD_COPYALL );
    };
  void SelectAll()
    {
      handle_clipboard( CLIPBOARD_SELECTALL );
    };

};
