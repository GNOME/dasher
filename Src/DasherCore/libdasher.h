#ifndef LIBDASHER_H
#define LIBDASHER_H

#include "DasherScreen.h"
#include "DasherView.h"
#include "DasherTypes.h"

using namespace Dasher;

enum string_param{ STRING_ALPHABET, 
		   STRING_EDITFONT, 
		   STRING_DASHERFONT, 
		   STRING_USERDIR,
		   STRING_SYSTEMDIR };

enum double_param{ DOUBLE_MAXBITRATE };

enum int_param{ INT_LANGUAGEMODEL, 
		INT_VIEW, 
		INT_SCREENWIDTH, 
		INT_SCREENHEIGHT,  
		INT_EDITFONTSIZE, 
		INT_EDITHEIGHT };

enum bool_param{ BOOL_DIMENSIONS, 
		 BOOL_SHOWTOOLBAR, 
		 BOOL_SHOWTOOLBARTEXT,
		 BOOL_SHOWTOOLBARLARGEICONS, 
		 BOOL_SHOWSPEEDSLIDER, 
		 BOOL_FIXLAYOUT, 
		 BOOL_TIMESTAMPNEWFILES, 
		 BOOL_COPYALLONSTOP, 
		 BOOL_DRAWMOUSE, 
		 BOOL_STARTONSPACE, 
		 BOOL_STARTONLEFT };

enum clipboard_action { CLIPBOARD_CUT,
			CLIPBOARD_COPY,
			CLIPBOARD_PASTE,
			CLIPBOARD_COPYALL,
			CLIPBOARD_SELECTALL,
			CLIPBOARD_CLEAR };

void dasher_initialise( int _width, int _height );
void dasher_finalise();

void dasher_set_parameter_string( string_param p, const char *value );
void dasher_set_parameter_double( double_param p, double value );
void dasher_set_parameter_int( int_param p, long int value );
void dasher_set_parameter_bool( bool_param p, bool value );

void dasher_set_orientation( Opts::ScreenOrientations orient );

void dasher_set_string_callback( void(*_cb)( string_param, const char * ) );
void dasher_set_double_callback( void(*_cb)( double_param, double ) );
void dasher_set_int_callback( void(*_cb)( int_param, long int ) );
void dasher_set_bool_callback( void(*_cb)( bool_param, bool ) );

void dasher_set_blank_callback( void(*_cb)() );
void dasher_set_display_callback( void(*_cb)() );
void dasher_set_draw_rectangle_callback( void (*_cb)(int, int, int, int, int, Opts::ColorSchemes) );
void dasher_set_draw_polyline_callback( void (*_cb)(Dasher::CDasherScreen::point*, int) );
void dasher_set_draw_text_callback(void (*_cb)(symbol, int, int, int));
void dasher_set_text_size_callback(void (*_cb)(symbol, int*, int*, int));

void dasher_set_edit_output_callback(void (*_cb)(symbol));
void dasher_set_edit_flush_callback(void (*_cb)(symbol));
void dasher_set_edit_unflush_callback( void (*_cb)() );

void dasher_set_clipboard_callback( void (*_cb)( clipboard_action ) );

void dasher_set_get_bool_option_callback( bool (*_cb)(const std::string&, bool *) );
void dasher_set_get_long_option_callback( bool (*_cb)(const std::string&, long *) );
void dasher_set_get_string_option_callback( bool (*_cb)(const std::string&, std::string *) );
void dasher_set_set_bool_option_callback( void (*_cb)(const std::string&, bool) );
void dasher_set_set_long_option_callback( void (*_cb)(const std::string&, long) );
void dasher_set_set_string_option_callback( void (*_cb)(const std::string&, const std::string&) );

void dasher_train_file( const char *filename );
int dasher_get_alphabets( const char **alphabetlist, int s );

void dasher_start();
void dasher_redraw();
void dasher_tap_on( int x, int y, unsigned long int time );
void dasher_unpause( unsigned long int time );
void dasher_pause( int x, int y );

string dasher_get_display_text( symbol Character );
string dasher_get_edit_text( symbol Character );

void dasher_resize_canvas( int _width, int _height );

void dasher_cut();
void dasher_copy();
void dasher_paste();
void dasher_copy_all();
void dasher_select_all();
void dasher_clear();

#endif
