#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>

#include <map>
#include <string>


#include "libdasher.h"
//#include "dasher.h"

extern GtkWidget *the_canvas;
extern GdkPixmap *offscreen_display_buffer;
extern GdkPixmap *offscreen_decoration_buffer;
extern GdkPixmap *offscreen_buffer;
extern GdkPixmap *onscreen_buffer;
extern PangoLayout *the_pangolayout;

extern std::map< std::string, PangoLayout * > oPangoCache;

extern "C" gint canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
extern "C" gint canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data);

void rebuild_buffer();
void initialise_canvas( int width, int height );

void blank_callback();
void display_callback();

void draw_rectangle_callback(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme);
void draw_polyline_callback(Dasher::CDasherScreen::point* Points, int Number);
void draw_colour_polyline_callback(Dasher::CDasherScreen::point* Points, int Number, int Colour);
void draw_colour_polygon_callback(Dasher::CDasherScreen::point* Points, int Number, int Colour);
void draw_text_callback(symbol Character, int x1, int y1, int size);
void draw_text_string_callback(std::string String, int x1, int y1, int size);
void text_size_callback(const std::string &String, int* Width, int* Height, int Size);
void send_marker_callback( int iMarker );

PangoLayout *get_pango_layout( std::string sDisplayText, int iSize );

void draw_mouseposbox(int which);

void set_canvas_font(std::string fontname);
void reset_dasher_font();

GdkColor get_color(int Color, Opts::ColorSchemes ColorScheme);

void receive_colour_scheme_callback(int numcolours, int* red, int* green, int* blue);
