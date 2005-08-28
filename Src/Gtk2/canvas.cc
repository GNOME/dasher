#include "canvas.h"

#include <iostream>
#include <sstream>


GtkWidget *the_canvas;
GdkPixmap *offscreen_display_buffer;
GdkPixmap *offscreen_decoration_buffer;
GdkPixmap *offscreen_buffer;
GdkPixmap *onscreen_buffer;
PangoLayout *the_pangolayout;
std::map< std::string, PangoLayout * > oPangoCache;

PangoFontDescription *font;
PangoRectangle *ink,*logical;

extern gboolean setup,preferences;
extern long mouseposstartdist;
extern gboolean firstbox, secondbox,paused;

gboolean drawoutline=FALSE;

static PangoLayout * create_pango_layout ();

#if WITH_CAIRO

/* Cairo drawing backend */
#include <gdk/gdkcairo.h>

cairo_t *display_cr;
cairo_t *decoration_cr;
cairo_t *cr;
typedef struct {
  double r, g, b;
} my_cairo_colour_t;
my_cairo_colour_t *cairo_colours = NULL;

#define BEGIN_DRAWING_BACKEND				\
  cairo_save(cr)

#define END_DRAWING_BACKEND				\
  cairo_restore(cr)

#define SET_COLOR_BACKEND(c)				\
  do {							\
    my_cairo_colour_t _c = cairo_colours[(c)];		\
    cairo_set_source_rgb(cr, _c.r, _c.g, _c.b);	\
  } while (0)

#else /* WITHOUT_CAIRO */

/* Gdk drawing backend */

GdkColormap *colormap;
GdkGC *gc;
GdkColor *colours = NULL;

#define BEGIN_DRAWING_BACKEND				\
  GdkGCValues origvalues;				\
  gdk_gc_get_values(gc,&origvalues)

#define END_DRAWING_BACKEND				\
  gdk_gc_set_values(gc,&origvalues,GDK_GC_FOREGROUND)

#define SET_COLOR_BACKEND(c)				\
  do {							\
    GdkColor _c = colours[(c)];				\
    gdk_colormap_alloc_color(colormap, &_c, FALSE, TRUE);	\
    gdk_gc_set_foreground (gc, &_c);	\
  } while (0)

#endif /* WITH_CAIRO */





/* Initialization routines */


/// Initialise the canvas - create rendering buffers and initialise
/// font rendering.


static void allocate_buffers( void )
{
  offscreen_display_buffer = gdk_pixmap_new(the_canvas->window, the_canvas->allocation.width, the_canvas->allocation.height, -1); 
  offscreen_decoration_buffer = gdk_pixmap_new(the_canvas->window, the_canvas->allocation.width, the_canvas->allocation.height, -1);
  offscreen_buffer = offscreen_display_buffer;
  onscreen_buffer = offscreen_decoration_buffer;
#if WITH_CAIRO

  // The lines between origin and pointer is draw here
  decoration_cr = gdk_cairo_create(offscreen_decoration_buffer);
  cr = decoration_cr;
//  cairo_translate(cr, -0.5, -0.5);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_set_line_width(cr, 1.0);

  // Base stuff are drawn here
  display_cr = gdk_cairo_create(offscreen_display_buffer);
  cr = display_cr;
//  cairo_translate(cr, -0.5, -0.5);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_set_line_width(cr, 1.0);
#endif
}

static void free_buffers( void )
{
#if WITH_CAIRO
  cr = NULL;
  cairo_destroy(display_cr);
  cairo_destroy(decoration_cr);
#endif
  offscreen_buffer = onscreen_buffer = NULL;
  g_object_unref(offscreen_display_buffer);
  g_object_unref(offscreen_decoration_buffer);
}

static void init_backend( void )
{
#if WITH_CAIRO
#else
  colormap = gdk_colormap_get_system();
  gc = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
#endif
}

void send_marker_callback( int iMarker ) {
  switch( iMarker ) {
  case 0:
    // Starting a new frame, so clear the background buffer

    offscreen_buffer = offscreen_display_buffer;
#if WITH_CAIRO
    cr = display_cr;
#endif

    break;
  case 1:

    onscreen_buffer = offscreen_display_buffer;
    gdk_draw_drawable( offscreen_decoration_buffer,
		       the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
		       offscreen_display_buffer,
		       0, 0, 0,0,
		       the_canvas->allocation.width,
		       the_canvas->allocation.height);
    onscreen_buffer = offscreen_buffer = offscreen_decoration_buffer;
#if WITH_CAIRO
    cr = decoration_cr;
#endif

    break;
  }
}






/* Drawing primitives */

#define SHOULD_WE					\
  if (setup==false||preferences==true)			\
    return
#define BEGIN_DRAWING					\
  SHOULD_WE;						\
  BEGIN_DRAWING_BACKEND

#define END_DRAWING					\
  END_DRAWING_BACKEND

#define SET_COLOR(c)					\
  SET_COLOR_BACKEND(c)

void draw_rectangle_callback(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme)
{
  int i;
  if( x2 < x1)
    i = x2, x2 = x1, x1 = i;
  if( y2 < y1)
    i = y2, y2 = y1, y1 = i;


  BEGIN_DRAWING;
  SET_COLOR(Color);
#if WITH_CAIRO
  cairo_rectangle(cr, x1, y1, x2-x1+1.0, y2-y1+1.0);
  cairo_fill(cr);
#else
  gdk_draw_rectangle (offscreen_buffer, gc, TRUE, x1, y1, x2-x1+1, y2-y1+1);
#endif

  if (drawoutline==TRUE) {
    SET_COLOR(3);
#if WITH_CAIRO
    cairo_rectangle(cr, x1+.5, y1+.5, x2-x1, y2-y1);
    cairo_stroke(cr);
#else
    gdk_draw_rectangle (offscreen_buffer, gc, FALSE, x1, y1, x2-x1, y2-y1);
#endif
  }
  END_DRAWING;
}

/// Blank the offscreen buffer
void blank_callback()
{
  BEGIN_DRAWING;
  SET_COLOR(0);
#if WITH_CAIRO
  cairo_paint(cr);
#else
  gdk_draw_rectangle (offscreen_buffer,		      
		      gc,
                      TRUE,
                      0, 0,
		      the_canvas->allocation.width,
		      the_canvas->allocation.height);
#endif
  END_DRAWING;
}

void draw_colour_polygon_callback(Dasher::CDasherScreen::point* Points, int Number, int Colour)
{
  BEGIN_DRAWING;
  SET_COLOR(Colour);
#if WITH_CAIRO
  cairo_move_to(cr, Points[0].x, Points[0].y);
  for (int i=1; i < Number; i++)
    cairo_line_to(cr, Points[i].x, Points[i].y);
  cairo_close_path(cr);
  cairo_fill(cr);
#else
  GdkPoint *gdk_points;
  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));


  for (int i=0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_polygon(offscreen_buffer, gc, TRUE, gdk_points, Number);
  g_free(gdk_points);
#endif
  END_DRAWING;
}


void draw_colour_polyline_callback(Dasher::CDasherScreen::point* Points, int Number, int Colour)
{ 
  BEGIN_DRAWING;
  SET_COLOR(Colour);
#if WITH_CAIRO
  cairo_move_to(cr, Points[0].x+.5, Points[0].y+.5);
  for (int i=1; i < Number; i++)
    cairo_line_to(cr, Points[i].x+.5, Points[i].y+.5);
  cairo_stroke(cr);
#else
  GdkPoint *gdk_points;
  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));

  for (int i=0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_lines(offscreen_buffer, gc, gdk_points, Number);
  g_free(gdk_points);
#endif
  END_DRAWING;
}

void draw_mouseposbox(int which) {
  BEGIN_DRAWING;

  GdkColor color;
  int top=0;

  switch (which) {
  case 0:
    color.pixel=0;
    color.red=255*257;
    color.green=0*257;
    color.blue=0*257;
    top=the_canvas->allocation.height/2-mouseposstartdist-100;
    break;
  case 1:
    color.pixel=0;
    color.red=255*257;
    color.green=255*257;
    color.blue=0*257;
    top=the_canvas->allocation.height/2+mouseposstartdist;
    break;
  }

#if WITH_CAIRO
  gdk_cairo_set_source_color(cr, &color);
  cairo_rectangle(cr, 0, top, (the_canvas->allocation.width-1), 100);
  cairo_fill(cr);
#else
  gdk_colormap_alloc_color(colormap, &color,FALSE, TRUE);
  gdk_gc_set_foreground (gc, &color);
  gdk_draw_rectangle (offscreen_buffer, gc, TRUE, 0, top, (the_canvas->allocation.width-1), 100);
#endif

  END_DRAWING;
}

void receive_colour_scheme_callback(int numcolours, int* red, int* green, int* blue)
{
#if WITH_CAIRO
  if (cairo_colours)
    delete[] cairo_colours;
  cairo_colours = new my_cairo_colour_t[numcolours];
#else
  if (colours)
    delete[] colours;
  colours = new GdkColor[numcolours];
#endif

  for (int i=0; i<numcolours; i++) {
#if WITH_CAIRO
    cairo_colours[i].r = red[i] / 255.0;
    cairo_colours[i].g = green[i] / 255.0;
    cairo_colours[i].b = blue[i] / 255.0;
#else
    colours[i].pixel=0;
    colours[i].red=red[i]*257;
    colours[i].green=green[i]*257;
    colours[i].blue=blue[i]*257;
#endif
  }
}








/* Almost-backend-independent stuff */

void display_callback()
{ 
  BEGIN_DRAWING;

  GdkRectangle update_rect;

  if(paused) {
    if(firstbox)
      draw_mouseposbox(0);
    else if(secondbox)
      draw_mouseposbox(1);
  }
      
  
  if (offscreen_buffer != onscreen_buffer)
    gdk_draw_drawable( onscreen_buffer,
		       the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
		       offscreen_buffer,
		       0, 0, 0,0,
		       the_canvas->allocation.width,
		       the_canvas->allocation.height);



  // Invalidate the full canvas to force it to be redrawn on-screen

  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = the_canvas->allocation.width;
  update_rect.height = the_canvas->allocation.height;

  gdk_window_invalidate_rect(the_canvas->window,&update_rect,FALSE);

  END_DRAWING;
}









/* Backend-dependent Pango stuff */

static
void draw_colour_text_string(std::string String, int Colour, int x1, int y1, int size)
{
  BEGIN_DRAWING;
  SET_COLOR(Colour);

  PangoLayout *pLayout( get_pango_layout( String, size ) );

  pango_layout_get_pixel_extents(pLayout,ink,logical);

#if WITH_CAIRO
  cairo_translate(cr, x1, y1-(int)ink->height/2);
  pango_cairo_show_layout(cr, pLayout);
#else
  gdk_draw_layout(offscreen_buffer,
		  gc, x1, y1-(int)ink->height/2, pLayout);
#endif
  END_DRAWING;
}


static PangoLayout *
create_pango_layout ()
{
#if WITH_CAIRO
  return pango_cairo_create_layout(cr);
#else
  return gtk_widget_create_pango_layout(GTK_WIDGET(the_canvas), "");
#endif
}
















/* Backend-independent stuff */

PangoLayout *get_pango_layout( std::string sDisplayText, int iSize ) {
  
  // Calculate the name of the pango layout in the cache - this
  // includes the display text and the size.

  char buffer[128]; // FIXME - what if we exceed this?
    
  snprintf( buffer, 128, "%d_%s", iSize, sDisplayText.c_str() );

  //  std::stringstream sCacheName;
  //sCacheName << iSize << "_" << sDisplayText;

  std::string sCacheName( buffer );

  // If we haven't got a cached pango layout for this string/size yet,
  // create a new one

  std::map< std::string, PangoLayout * >::iterator it( oPangoCache.find( sCacheName ) );

  if( it != oPangoCache.end() )
    return it->second;
  else {
    PangoLayout *pNewPangoLayout( create_pango_layout() );

    pango_font_description_set_size( font, iSize*PANGO_SCALE );
    pango_layout_set_font_description( pNewPangoLayout,font );
    pango_layout_set_text( pNewPangoLayout,sDisplayText.c_str(),-1 );

    oPangoCache[ sCacheName ] = pNewPangoLayout;

    return pNewPangoLayout;
  }
}

void text_size_callback(const std::string &String, int* Width, int* Height, int size)
{
  *Width = *Height = 0;

  SHOULD_WE;

  // Get a pango layout from the cache.

  PangoLayout *pLayout( get_pango_layout( String, size ) );

  pango_layout_get_pixel_extents( pLayout,ink,logical);

  *Width =ink->width;
  *Height=ink->height;
}

void set_canvas_font(std::string fontname) 
{
  if(fontname!="") {
    pango_font_description_free(font);
    font=pango_font_description_from_string(fontname.c_str());
    oPangoCache.clear();
    dasher_redraw();
  }
}

void reset_dasher_font()
{
  pango_font_description_free(font);
  font=pango_font_description_from_string(DASHERFONT);
  dasher_redraw();
}

void initialise_canvas( void )
{
  allocate_buffers();

  init_backend();

  // Pango font rendering stuff
  the_pangolayout = create_pango_layout ();

  font = pango_font_description_new();
  pango_font_description_set_family(font ,"Serif");

  ink = new PangoRectangle;
  logical = new PangoRectangle;
}

void draw_text_callback(symbol Character, int x1, int y1, int size)
{
  std::string String = dasher_get_display_text( Character );
  int colour = dasher_get_text_colour(Character);
  draw_colour_text_string(String, colour, x1, y1, size);
}

void draw_text_string_callback(std::string String, int x1, int y1, int size)
{
  draw_colour_text_string(String, 4, x1, y1, size);
}

void draw_polyline_callback(Dasher::CDasherScreen::point* Points, int Number)
{
  draw_colour_polyline_callback(Points, Number, 0);
}

void rebuild_buffer()
{
  free_buffers();
  allocate_buffers();
}

