#include "canvas.h"

#include <iostream>

GtkWidget *the_canvas;
GdkPixmap *offscreen_buffer;
GdkPixmap *onscreen_buffer;
PangoLayout *the_pangolayout;
PangoFontDescription *font;
PangoRectangle *ink,*logical;
GdkColor *colours;

extern gboolean setup;
extern long mouseposstartdist;
extern gboolean firstbox, secondbox,paused;

gboolean drawoutline=FALSE;

void rebuild_buffer()
{
  g_free(offscreen_buffer);
  g_free(onscreen_buffer);

  offscreen_buffer = gdk_pixmap_new(the_canvas->window, the_canvas->allocation.width, the_canvas->allocation.height, DefaultDepth(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL))));
  onscreen_buffer = gdk_pixmap_new(the_canvas->window, the_canvas->allocation.width, the_canvas->allocation.height, DefaultDepth(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL))));
}

void initialise_canvas( int width, int height )
{
  offscreen_buffer = gdk_pixmap_new(the_canvas->window, width, height, DefaultDepth(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL))));
   onscreen_buffer = gdk_pixmap_new(the_canvas->window, width, height, DefaultDepth(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL))));
  the_pangolayout = gtk_widget_create_pango_layout (GTK_WIDGET(the_canvas), "");
  font = pango_font_description_new();

  pango_font_description_set_family( font,"Serif");

  ink = new PangoRectangle;
  logical = new PangoRectangle;
}

void blank_callback()
{
  if (setup==false) 
    return;

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor background = colours[0];

  gdk_color_alloc(colormap, &background);
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

  if (setup==false)
    return;

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor background = colours[0];

  gdk_color_alloc(colormap, &background);
  gdk_gc_set_foreground (graphics_context, &background);

  if (paused==true) {
    if (firstbox==true) {
      draw_mouseposbox(0);
    } else if (secondbox==true) {
      draw_mouseposbox(1);
    }
  }

  gdk_draw_pixmap(onscreen_buffer,
  		  the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
  		  offscreen_buffer,
  		  0, 0, 0,0,
  		      the_canvas->allocation.width,
  		      the_canvas->allocation.height);

  gdk_draw_rectangle ( offscreen_buffer,
		       graphics_context,
                      TRUE,
                      0, 0,
		      the_canvas->allocation.width,
		      the_canvas->allocation.height);
  
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = the_canvas->allocation.width;
  update_rect.height = the_canvas->allocation.height;

  gtk_widget_draw(the_canvas,&update_rect);
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void draw_rectangle_callback(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme)
{
  GdkRectangle update_rect;
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  if (setup==false)
    return;

  GdkColor outline = colours[3];
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor foreground = colours[Color];

  gdk_color_alloc(colormap, &foreground);
  gdk_color_alloc(colormap, &outline);
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

void draw_colour_polyline_callback(Dasher::CDasherScreen::point* Points, int Number, int Colour)
{ 
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkRectangle update_rect;
  GdkGCValues origvalues;

  if (setup==false)
    return;

  GdkColor colour = colours[Colour];
  GdkPoint *gdk_points;

  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  gdk_gc_get_values(graphics_context,&origvalues);
  colormap = gdk_colormap_get_system();

  gdk_color_alloc(colormap, &colour);
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
  GdkRectangle update_rect;
  GdkGCValues origvalues;
  std::string symbol;

  if (setup==false)
    return;

  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  int colour=dasher_get_text_colour(Character);

  GdkColor foreground = colours[colour];

  gdk_color_alloc(colormap, &foreground);
  gdk_gc_set_foreground (graphics_context, &foreground);
  
  pango_font_description_set_size( font,size*PANGO_SCALE);

  pango_layout_set_font_description(the_pangolayout,font);

  symbol = dasher_get_display_text( Character );

  pango_layout_set_text(the_pangolayout,symbol.c_str(),-1);

  pango_layout_get_pixel_extents(the_pangolayout,ink,logical);

  gdk_draw_layout (offscreen_buffer,
		   graphics_context,
		   x1, y1-ink->height/2, the_pangolayout);

  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void draw_text_string_callback(std::string String, int x1, int y1, int size)
{
  GdkRectangle update_rect;
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  if (setup==false)
    return;

  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor foreground = colours[4];

  gdk_color_alloc(colormap, &foreground);
  gdk_gc_set_foreground (graphics_context, &foreground);

  pango_font_description_set_size( font,size*PANGO_SCALE);

  pango_layout_set_font_description(the_pangolayout,font);

  pango_layout_set_text(the_pangolayout,String.c_str(),-1);

  pango_layout_get_pixel_extents(the_pangolayout,ink,logical);

  gdk_draw_layout (offscreen_buffer,
		   graphics_context,
		   x1, y1-ink->height/2, the_pangolayout);

  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void text_size_callback(symbol Character, int* Width, int* Height, int size)
{
  if (setup==false)
      return;

  std::string symbol = dasher_get_display_text( Character );

  pango_font_description_set_size( font,size*PANGO_SCALE);

  pango_layout_set_font_description(the_pangolayout,font);

  pango_layout_set_text(the_pangolayout,symbol.c_str(),-1);

  pango_layout_get_pixel_extents(the_pangolayout,ink,logical);

  *Width =ink->width;
  *Height=ink->height;
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
}

GdkFont *get_font(int size)
{
  GdkFont *chosen_font;

  if (size == 11) {
    chosen_font = gdk_font_load("-*-fixed-medium-r-normal--8-*-*-*-*-*-*");
  }
  else {
    if (size == 14) {
      chosen_font = gdk_font_load("-*-fixed-medium-r-normal--13-*-*-*-*-*-*");
    }
    else {
      if (size == 20) {
	chosen_font = gdk_font_load("-*-fixed-medium-r-normal--15-*-*-*-*-*-*");
      }
      else {
	chosen_font = gdk_font_load("-*-fixed-medium-r-normal--8-*-*-*-*-*-*");
      }
    }
  }

  if (chosen_font == NULL)
    chosen_font = gdk_font_load("-*-fixed-medium-r-normal--8-*-*-*-*-*-*");
  
  return chosen_font;
}

void set_canvas_font(std::string fontname) 
{
  if(fontname!="") {
    pango_font_description_free(font);
    font=pango_font_description_from_string(fontname.c_str());
    dasher_redraw();
  }
}

void reset_dasher_font()
{
  pango_font_description_free(font);
  font=pango_font_description_from_string("Serif 12");
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
  if (setup==false)
    return;

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkRectangle update_rect;
  GdkColor color;
  GdkGCValues origvalues;
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();
  int top, bottom;

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
  gdk_color_alloc(colormap, &color);
  gdk_gc_set_foreground (graphics_context, &color);
  gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, 0, top, (the_canvas->allocation.width-1), 100);
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}




