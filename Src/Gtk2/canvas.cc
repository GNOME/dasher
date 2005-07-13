#include "canvas.h"

#include "DasherControlPrivate.h"

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
GdkColor *colours;

extern gboolean setup,preferences;
extern long mouseposstartdist;
extern gboolean firstbox, secondbox,paused;

gboolean firsttime = TRUE;

extern "C" gint canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
extern "C" gint canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data);


gboolean drawoutline=FALSE;

/// Regenerate the on- and off- screen rendering buffers

void rebuild_buffer()
{
  g_free(offscreen_display_buffer);
  g_free(offscreen_decoration_buffer);
  g_free(onscreen_buffer);

  offscreen_display_buffer = gdk_pixmap_new(the_canvas->window, the_canvas->allocation.width, the_canvas->allocation.height, -1); 
  offscreen_decoration_buffer = gdk_pixmap_new(the_canvas->window, the_canvas->allocation.width, the_canvas->allocation.height, -1);
  onscreen_buffer = gdk_pixmap_new(the_canvas->window, the_canvas->allocation.width, the_canvas->allocation.height, -1);

  offscreen_buffer = offscreen_display_buffer;
}

/// Initialise the canvas - create rendering buffers and initialise
/// font rendering.

void initialise_canvas( int width, int height )
{
  // Create new pixmaps

  offscreen_display_buffer = gdk_pixmap_new(the_canvas->window, width, height, -1);
  offscreen_decoration_buffer = gdk_pixmap_new(the_canvas->window, width, height, -1);
  onscreen_buffer = gdk_pixmap_new(the_canvas->window, width, height, -1);

  std::cout << offscreen_display_buffer << " " << offscreen_decoration_buffer << " " << onscreen_buffer << std::endl;

  std::cout << GTK_WIDGET_DRAWABLE( onscreen_buffer ) << std::endl;

  offscreen_buffer = offscreen_display_buffer;

  // Pango font rendering stuff

  the_pangolayout = gtk_widget_create_pango_layout (GTK_WIDGET(the_canvas), "");
  font = pango_font_description_new();
  pango_font_description_set_family( font,"Serif");

  ink = new PangoRectangle;
  logical = new PangoRectangle;

  g_signal_connect( the_canvas, "expose_event", G_CALLBACK(canvas_expose_event), NULL );
  g_signal_connect( the_canvas, "configure_event", G_CALLBACK(canvas_configure_event), NULL );

  gtk_widget_add_events (the_canvas, GDK_EXPOSURE_MASK );
  gtk_widget_add_events (the_canvas, GDK_BUTTON_PRESS_MASK);
  gtk_widget_add_events (the_canvas, GDK_BUTTON_RELEASE_MASK);
}

/// Blank the offscreen buffer

void blank_callback()
{

  if (setup==false||preferences==true) 
    return;

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor background = colours[0];

  gdk_colormap_alloc_color(colormap, &background, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &background);
  
  gdk_draw_rectangle (offscreen_buffer,		      
		      graphics_context,
                      TRUE,
                      0, 0,
		      the_canvas->allocation.width,
		      the_canvas->allocation.height);

  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}


void display_callback()
{ 


  GdkRectangle update_rect;

  if (setup==false||preferences==true)
    return;

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor background = colours[0];

  gdk_colormap_alloc_color(colormap, &background, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &background);

  // Draw the target areas for 'dwell' start mode if necessary

  if (paused==true) {
    if (firstbox==true) {
      draw_mouseposbox(0);
    } else if (secondbox==true) {
      draw_mouseposbox(1);
    }
  }

  // Copy the offscreen buffer into the onscreen buffer
  
  gdk_draw_drawable( onscreen_buffer,
		     the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
		     offscreen_buffer,
		     0, 0, 0,0,
		     the_canvas->allocation.width,
		     the_canvas->allocation.height);

  // Blank the offscreen buffer (?)

  gdk_draw_rectangle( offscreen_buffer,
 		      graphics_context,
		      TRUE,
		      0, 0,
 		      the_canvas->allocation.width,
 		      the_canvas->allocation.height);
  
  // Invalidate the full canvas to force it to be redrawn on-screen

  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = the_canvas->allocation.width;
  update_rect.height = the_canvas->allocation.height;

  gdk_window_invalidate_rect(the_canvas->window,&update_rect,FALSE);

  // Restore original graphics context (?)

  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void draw_rectangle_callback(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme)
{
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  if (setup==false||preferences==true)
    return;

  GdkColor outline = colours[3];
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor foreground = colours[Color];

  gdk_colormap_alloc_color(colormap, &foreground,FALSE,TRUE);
  gdk_colormap_alloc_color(colormap, &outline,FALSE,TRUE);
  gdk_gc_set_foreground (graphics_context, &foreground);
  
  if( x2 > x1 ) {
    if( y2 > y1 ) {
      gdk_draw_rectangle (offscreen_buffer, graphics_context, TRUE, x1, y1, x2-x1, y2-y1);
      if (drawoutline==TRUE) {
	gdk_gc_set_foreground (graphics_context, &outline);
	gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, x1, y1, x2-x1, y2-y1);
      }
    }
    else {
      gdk_draw_rectangle (offscreen_buffer, graphics_context, TRUE, x1, y2, x2-x1, y1-y2);
      if (drawoutline==TRUE) {
	gdk_gc_set_foreground (graphics_context, &outline);
	gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, x1, y2, x2-x1, y1-y2);
      }
    }
  }
  else {
    if( y2 > y1 ) {
      gdk_draw_rectangle (offscreen_buffer, graphics_context, TRUE, x2, y1, x1-x2, y2-y1);
      if (drawoutline==TRUE) {
	gdk_gc_set_foreground (graphics_context, &outline);
	gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, x2, y1, x1-x2, y2-y1);
      }
    }
    else {
      gdk_draw_rectangle (offscreen_buffer, graphics_context, TRUE, x2, y2, x1-x2, y1-y2);
      if (drawoutline==TRUE) {
	gdk_gc_set_foreground (graphics_context, &outline);
	gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, x2, y2, x1-x2, y1-y2);
      }
    }
  }
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void draw_polyline_callback(Dasher::CDasherScreen::point* Points, int Number)
{
  draw_colour_polyline_callback(Points, Number, 0);
}

void draw_colour_polygon_callback(Dasher::CDasherScreen::point* Points, int Number, int Colour)
{
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  if (setup==false||preferences==true)
    return;

  GdkColor colour = colours[Colour];
  GdkPoint *gdk_points;

  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  gdk_gc_get_values(graphics_context,&origvalues);
  colormap = gdk_colormap_get_system();

  gdk_colormap_alloc_color(colormap, &colour, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &colour);

  for (int i=0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_polygon(offscreen_buffer, graphics_context, TRUE, gdk_points, Number);
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
  g_free(gdk_points);
}


void draw_colour_polyline_callback(Dasher::CDasherScreen::point* Points, int Number, int Colour)
{ 
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  if (setup==false||preferences==true)
    return;

  GdkColor colour = colours[Colour];
  GdkPoint *gdk_points;

  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  gdk_gc_get_values(graphics_context,&origvalues);
  colormap = gdk_colormap_get_system();

  gdk_colormap_alloc_color(colormap, &colour, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &colour);

  for (int i=0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_lines(offscreen_buffer, graphics_context, gdk_points, Number);
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
  g_free(gdk_points);
}

void draw_text_callback(symbol Character, int x1, int y1, int size)
{
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;
  std::string symbol;

  if (setup==false||preferences==true)
    return;

  symbol = dasher_get_display_text( Character );

  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  int colour=dasher_get_text_colour(Character);

  GdkColor foreground = colours[colour];

  gdk_colormap_alloc_color(colormap, &foreground, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &foreground);
  
  PangoLayout *pLayout( get_pango_layout( symbol, size ) );

  pango_layout_get_pixel_extents(pLayout,ink,logical);

  gdk_draw_layout (offscreen_buffer,
		   graphics_context,
		   x1, y1-ink->height/2, pLayout);

  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

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
    PangoLayout *pNewPangoLayout( gtk_widget_create_pango_layout (GTK_WIDGET(the_canvas), "") );

    pango_font_description_set_size( font, iSize*PANGO_SCALE );
    pango_layout_set_font_description( pNewPangoLayout,font );
    pango_layout_set_text( pNewPangoLayout,sDisplayText.c_str(),-1 );

    oPangoCache[ sCacheName ] = pNewPangoLayout;

    return pNewPangoLayout;
  }
}

void draw_text_string_callback(std::string String, int x1, int y1, int size)
{
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  if (setup==false||preferences==true)
    return;

  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor foreground = colours[4];

  gdk_colormap_alloc_color(colormap, &foreground, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &foreground);

  PangoLayout *pLayout( get_pango_layout( String, size ) );

  pango_layout_get_pixel_extents(pLayout,ink,logical);

  gdk_draw_layout (offscreen_buffer,
		   graphics_context,
		   x1, y1-ink->height/2, pLayout);

  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void text_size_callback(const std::string &String, int* Width, int* Height, int size)
{
  if (setup==false||preferences==true)
      return;

  // Get a pango layout from the cache.

  PangoLayout *pLayout( get_pango_layout( String, size ) );

  pango_layout_get_pixel_extents( pLayout,ink,logical);

  *Width =ink->width;
  *Height=ink->height;
}

void send_marker_callback( int iMarker ) {
  switch( iMarker ) {
  case 0:
    // Starting a new frame, so clear the background buffer


    offscreen_buffer = offscreen_display_buffer;

    break;
  case 1:

    gdk_draw_drawable( offscreen_decoration_buffer,
		       the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
		       offscreen_display_buffer,
		       0, 0, 0,0,
		       the_canvas->allocation.width,
		       the_canvas->allocation.height);
    
    offscreen_buffer = offscreen_decoration_buffer;

    break;
  }
}

GdkColor get_color(int Color, Opts::ColorSchemes ColorScheme)
{
  if (ColorScheme == Special1) {
    GdkColor foreground = { 0, 240*257, 240*257, 240*257 };
    return foreground;
  }
  if (ColorScheme == Special2) {
    GdkColor foreground = { 0, 255*257, 255*257, 255*257 };
    return foreground;
  }
  if (ColorScheme == Objects) {
    GdkColor foreground = { 0, 0, 0, 0 };
    return foreground;
  }
  if (ColorScheme == Groups) {
    if (Color%3 == 0) {
      GdkColor foreground = { 0, 255*257, 255*257 ,0 };
      return foreground;
    }
    if (Color%3 == 1) {
      GdkColor foreground = { 0, 255*257 ,100*257, 100*257 };
      return foreground;
    }
    if (Color %3 == 2) {
      GdkColor foreground = { 0, 0, 255*257 ,0 };
      return foreground;
    }
  }
  if (ColorScheme == Nodes1) {
    if (Color%3 == 0) {
      GdkColor foreground = { 0, 180*257 ,245*257 ,180*257 };
      return foreground;
    }
    if (Color%3 == 1) {
      GdkColor foreground = { 0, 160*257 ,200*257 ,160*257 };
      return foreground;
    }
    if (Color %3 == 2) {
      GdkColor foreground = { 0, 0, 255*257, 255*257 };
      return foreground;
    }
  }
  if (ColorScheme == Nodes2) {
    if (Color%3 == 0) {
      GdkColor foreground = { 0, 255*257, 185*257, 255*257 };
      return foreground;
    }
    if (Color%3 == 1) {
      GdkColor foreground = { 0, 140*257, 200*257, 255*257 };
      return foreground;
    }
    if (Color %3 == 2) {
      GdkColor foreground = { 0, 255*257, 175*257, 175*257 };
      return foreground;
    }
  }
  GdkColor foreground = {0, 0, 0, 0 };
  return foreground;
}

void set_canvas_font(std::string fontname) 
{
  if(fontname!="") {
    pango_font_description_free(font);
    font=pango_font_description_from_string(fontname.c_str());
    dasher_redraw();
  }
  //  {
  //  GdkColor foreground = { 0, 255*257, 185*257, 255*257 };
  //  return foreground;
  // }
}

void reset_dasher_font()
{
  pango_font_description_free(font);
  font=pango_font_description_from_string("DASHERFONT");
  dasher_redraw();
}

void receive_colour_scheme_callback(int numcolours, int* red, int* green, int* blue)
{
  colours = new GdkColor[numcolours];
  for (int i=0; i<numcolours; i++) {
    colours[i].pixel=0;
    colours[i].red=red[i]*257;
    colours[i].green=green[i]*257;
    colours[i].blue=blue[i]*257;
  }
}

void draw_mouseposbox(int which) {

  if (setup==false||preferences==true)
    return;

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkColor color;
  GdkGCValues origvalues;
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();
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

  gdk_gc_get_values(graphics_context,&origvalues);
  gdk_colormap_alloc_color(colormap, &color,FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &color);
  gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, 0, top, (the_canvas->allocation.width-1), 100);
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

// GTK event handlers for the canvas

extern "C" gint
canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{

  gdk_draw_drawable(the_canvas->window,
		  the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
		  onscreen_buffer,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  if (firsttime==TRUE) {
    // canvas_expose_event() is the easiest function to catch
    // if we want to know when everything is set up and displayed
    paused=true;
    firsttime=false;
  }

  return TRUE;
}

extern "C" gint
canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{

  // If the canvas is resized, we need to regenerate all of the buffers
  rebuild_buffer();

  dasher_resize_canvas( the_canvas->allocation.width, the_canvas->allocation.height );

  dasher_redraw();

  if (setup==TRUE) {
    // If we're set up and resized, then save those settings

    // FIXME - Reimplement this

//     dasher_set_parameter_int(INT_EDITHEIGHT,gtk_paned_get_position(GTK_PANED(glade_xml_get_widget(widgets,"vpaned1"))));
//     gtk_window_get_size(GTK_WINDOW(window), &dasherwidth, &dasherheight);
//     dasher_set_parameter_int(INT_SCREENHEIGHT, dasherheight);
//     dasher_set_parameter_int(INT_SCREENWIDTH, dasherwidth);
  }

  return FALSE;
}


