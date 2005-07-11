#ifndef LIBDASHER_H
#define LIBDASHER_H

#include "DasherScreen.h"
#include "DasherView.h"
#include "DasherTypes.h"
#include "UserLog.h"

using namespace Dasher;

enum string_param{ STRING_ALPHABET, 
		   STRING_EDITFONT, 
		   STRING_DASHERFONT, 
		   STRING_USERDIR,
		   STRING_SYSTEMDIR,
		   STRING_COLOUR };

enum double_param{ DOUBLE_MAXBITRATE };

enum int_param{ INT_LANGUAGEMODEL, 
		INT_VIEW, 
		INT_SCREENWIDTH, 
		INT_SCREENHEIGHT,  
		INT_EDITFONTSIZE,
		INT_DASHERFONTSIZE,
		INT_EDITHEIGHT,
		INT_ORIENTATION,
		INT_ENCODING,
		INT_UNIFORM,
		INT_ONEBUTTON,
		INT_LM_MAXORDER,
		INT_LM_ALPHA,
		INT_LM_BETA,
		INT_LM_EXCLUSION,
		INT_LM_UPDATE_EXCLUSION,
		INT_LM_MIXTURE,
		INT_TRUNCATION,
		INT_TRUNCATIONTYPE };

enum bool_param{ BOOL_DIMENSIONS, 
		 BOOL_EYETRACKER,
		 BOOL_SHOWTOOLBAR, 
		 BOOL_SHOWTOOLBARTEXT,
		 BOOL_SHOWTOOLBARLARGEICONS, 
		 BOOL_SHOWSPEEDSLIDER, 
		 BOOL_FIXLAYOUT, 
		 BOOL_TIMESTAMPNEWFILES, 
		 BOOL_COPYALLONSTOP, 
		 BOOL_DRAWMOUSE, 
		 BOOL_DRAWMOUSELINE, 
		 BOOL_STARTONSPACE, 
		 BOOL_STARTONLEFT,
		 BOOL_KEYBOARDCONTROL,
		 BOOL_WINDOWPAUSE,
		 BOOL_CONTROLMODE,
		 BOOL_COLOURMODE,
		 BOOL_KEYBOARDMODE,
		 BOOL_MOUSEPOSSTART,
		 BOOL_SPEECHMODE,
		 BOOL_OUTLINEMODE,
		 BOOL_PALETTECHANGE};

enum clipboard_action { CLIPBOARD_CUT,
			CLIPBOARD_COPY,
			CLIPBOARD_PASTE,
			CLIPBOARD_COPYALL,
			CLIPBOARD_SELECTALL,
			CLIPBOARD_CLEAR };

void dasher_early_initialise( );
void dasher_late_initialise( int _width, int _height );
void dasher_finalise();

void dasher_set_parameter_string( string_param p, const char *value );
void dasher_set_parameter_double( double_param p, double value );
void dasher_set_parameter_int( int_param p, long int value );
void dasher_set_parameter_bool( bool_param p, bool value );

void dasher_set_orientation( Opts::ScreenOrientations orient );
void dasher_set_encoding( Opts::FileEncodingFormats encoding );

//void dasher_set_lm_parameter( const char *pname, int value );

void dasher_set_string_callback( void(*_cb)( string_param, const char * ) );
void dasher_set_double_callback( void(*_cb)( double_param, double ) );
void dasher_set_int_callback( void(*_cb)( int_param, long int ) );
void dasher_set_bool_callback( void(*_cb)( bool_param, bool ) );

void dasher_set_blank_callback( void(*_cb)() );
void dasher_set_display_callback( void(*_cb)() );
void dasher_set_colour_scheme_callback( void (*_cb)(int, int*, int*, int*) );
void dasher_set_draw_rectangle_callback( void (*_cb)(int, int, int, int, int, Opts::ColorSchemes) );
void dasher_set_draw_polyline_callback( void (*_cb)(Dasher::CDasherScreen::point*, int) );
void dasher_set_draw_colour_polyline_callback( void (*_cb)(Dasher::CDasherScreen::point*, int, int) );
void dasher_set_draw_colour_polygon_callback( void (*_cb)(Dasher::CDasherScreen::point*, int, int) );
void dasher_set_draw_text_callback(void (*_cb)(symbol, int, int, int));
void dasher_set_draw_text_string_callback(void (*_cb)(std::string, int, int, int));
void dasher_set_text_size_callback(void (*_cb)(const std::string &, int*, int*, int));
void dasher_set_send_marker_callback( void (*_cb)( int ) );

void dasher_set_edit_output_callback(void (*_cb)(symbol));
void dasher_set_edit_outputcontrol_callback(void (*_cb)(void*, int));
void dasher_set_edit_delete_callback(void (*_cb)(symbol));
void dasher_set_get_new_context_callback( void (*_cb)( std::string &, int ) );

void dasher_set_clipboard_callback( void (*_cb)( clipboard_action ) );

void dasher_set_get_bool_option_callback( bool (*_cb)(const std::string&, bool *) );
void dasher_set_get_long_option_callback( bool (*_cb)(const std::string&, long *) );
void dasher_set_get_string_option_callback( bool (*_cb)(const std::string&, std::string *) );
void dasher_set_set_bool_option_callback( void (*_cb)(const std::string&, bool) );
void dasher_set_set_long_option_callback( void (*_cb)(const std::string&, long) );
void dasher_set_set_string_option_callback( void (*_cb)(const std::string&, const std::string&) );

void dasher_set_input( CDasherInput *_pInput );

void dasher_train_file( const char *filename );
int dasher_get_alphabets( const char **alphabetlist, int s );
const char* dasher_get_current_alphabet();
int dasher_get_colours( const char **colourlist, int s );
const char* dasher_get_current_colours();
const char* dasher_get_training_file();

void dasher_start();
void dasher_redraw();
void dasher_render();
void dasher_tap_on( int x, int y, unsigned long int time );
void dasher_draw_mouse_position( int x, int y );
void dasher_go_to( int x, int y );
void dasher_draw_go_to( int x, int y );
void dasher_unpause( unsigned long int time );
void dasher_halt();

void dasher_pause( int x, int y );
void dasher_reset_nats();
double dasher_get_nats();

std::string dasher_get_display_text( symbol Character );
std::string dasher_get_edit_text( symbol Character );
int dasher_get_text_colour( symbol Character );
int dasher_get_onebutton(void);

void dasher_resize_canvas( int _width, int _height );

void dasher_cut();
void dasher_copy();
void dasher_paste();
void dasher_copy_all();
void dasher_select_all();
void dasher_clear();

void add_control_tree(ControlTree *controltree);

void add_alphabet_filename(const char* filename);
void add_colour_filename(const char* filename);

CUserLog*   dasher_get_user_log_ptr();
void        dasher_user_log_init_is_done();

#endif
