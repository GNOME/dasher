#include "DasherInterface.h"
#include "DasherSettingsInterface.h"

using namespace Dasher;
using namespace std;

void handle_parameter_string( string_param p, const string & value );
void handle_parameter_double( double_param p, double value );
void handle_parameter_int( int_param p, long int value );
void handle_parameter_bool( bool_param p, bool value );

void handle_blank();
void handle_display();

void handle_draw_rectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme);
void handle_draw_polyline(Dasher::CDasherScreen::point* Points, int Number);
void handle_draw_text(symbol Character, int x1, int y1, int size);
void handle_text_size(symbol Character, int* Width, int* Height, int Size);
void handle_edit_output(symbol Character);
void handle_edit_flush(symbol Character);
void handle_edit_unflush();

void handle_clipboard( clipboard_action act );

bool handle_get_bool_option(const std::string& Key, bool *Value);
bool handle_get_long_option(const std::string& Key, long *Value);
bool handle_get_string_option(const std::string& Key, std::string *Value);
void handle_set_bool_option(const std::string& Key, bool Value);
void handle_set_long_option(const std::string& Key, long Value);
void handle_set_string_option(const std::string& Key, const std::string& Value);

class dasher_ui : public CDasherSettingsInterface
{
 public:

  void ChangeAlphabet(const string& NewAlphabetID) 
    { 
      handle_parameter_string( STRING_ALPHABET, NewAlphabetID ); 
    };

  void ChangeMaxBitRate(double NewMaxBitRate) 
    { 
      handle_parameter_double( DOUBLE_MAXBITRATE, NewMaxBitRate ); 
    };

  void ChangeLanguageModel(unsigned int NewLanguageModelID) 
    {
      handle_parameter_int( INT_LANGUAGEMODEL, NewLanguageModelID );
    };

  void ChangeView(unsigned int NewViewID)
    {
      handle_parameter_int( INT_VIEW, NewViewID );
    };

  void ChangeOrientation(Opts::ScreenOrientations Orientation) {};
  void SetFileEncoding(Opts::FileEncodingFormats Encoding) {};

  void SetScreenSize(long Width, long Height) 
    {
      handle_parameter_int( INT_SCREENWIDTH, Width );
      handle_parameter_int( INT_SCREENHEIGHT, Height );
    };

  void SetDasherFontSize(Dasher::Opts::FontSize fontsize) {};

  void SetDasherDimensions(bool Value) 
    {
      handle_parameter_bool( BOOL_DIMENSIONS, Value );
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

  void SetEditFont(string Name, long Size) 
    {
      handle_parameter_string( STRING_EDITFONT, Name );
      handle_parameter_int( INT_EDITFONTSIZE, Size );
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
};


class dasher_screen : public CDasherScreen
{
 public:

  dasher_screen( int _width, int _height )
    : CDasherScreen( _width, _height )
    {
    };

  void SetFont(std::string Name)
    {
    };

  void SetFontSize(Dasher::Opts::FontSize fontsize)
    {
    };
	
  Dasher::Opts::FontSize GetFontSize()
    {
      //FIXME
      return Dasher::Opts::FontSize(1);
    };

  void TextSize(symbol Character, int* Width, int* Height, int Size) const
    {
      handle_text_size( Character, Width, Height, Size );
    };

  void DrawText(symbol Character, int x1, int y1, int Size) const
    {
      handle_draw_text( Character, x1, y1, Size );
    };

  void DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const
    {
      handle_draw_rectangle( x1, y1, x2, y2, Color, ColorScheme);
    };
	
  void Polyline(point* Points, int Number) const
    {
      handle_draw_polyline( Points, Number );
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
  void get_new_context(std::string& str, int max) {};
  void unflush()
    {
      handle_edit_unflush();
    };
  void output(symbol Symbol) 
    {
      handle_edit_output(Symbol);
    };
  void flush(symbol Symbol)
    {
      handle_edit_flush(Symbol);
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
