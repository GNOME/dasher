#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>

#include "libdasher.h"
#include "dasher.h"

extern GtkWidget *the_canvas;
extern GdkPixmap *offscreen_buffer;
extern GdkPixmap *onscreen_buffer;
extern PangoLayout *the_pangolayout;

void rebuild_buffer();
void initialise_canvas( int width, int height );

void blank_callback();
void display_callback();

void draw_rectangle_callback(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme);
void draw_polyline_callback(Dasher::CDasherScreen::point* Points, int Number);
void draw_colour_polyline_callback(Dasher::CDasherScreen::point* Points, int Number, int Colour);
void draw_text_callback(symbol Character, int x1, int y1, int size);
void draw_text_string_callback(std::string String, int x1, int y1, int size);
void text_size_callback(symbol Character, int* Width, int* Height, int Size);

void draw_mouseposbox(int which);

void set_canvas_font(std::string fontname);
void reset_dasher_font();

GdkColor get_color(int Color, Opts::ColorSchemes ColorScheme);

void receive_colour_scheme_callback(int numcolours, int* red, int* green, int* blue);
