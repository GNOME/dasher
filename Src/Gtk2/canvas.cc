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
extern gboolean paused;

void rebuild_buffer()
{
  delete(offscreen_buffer);
  delete(onscreen_buffer);

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
  if (setup==false || paused==true) 
    return;
  gdk_draw_rectangle (offscreen_buffer,		      
		      the_canvas->style->white_gc,
                      TRUE,
                      0, 0,
		      the_canvas->allocation.width,
		      the_canvas->allocation.height);
}


void display_callback()
{ 
  GdkRectangle update_rect;

  if (setup==false || paused==true)
    return;

  gdk_draw_pixmap(onscreen_buffer,
  		  the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
  		  offscreen_buffer,
  		  0, 0, 0,0,
  		      the_canvas->allocation.width,
  		      the_canvas->allocation.height);

  gdk_draw_rectangle ( offscreen_buffer,
		       the_canvas->style->white_gc,
                      TRUE,
                      0, 0,
		      the_canvas->allocation.width,
		      the_canvas->allocation.height);
  
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = the_canvas->allocation.width;
  update_rect.height = the_canvas->allocation.height;

  gtk_widget_draw(the_canvas,&update_rect);
}

void draw_rectangle_callback(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme)
{
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkRectangle update_rect;

  if (setup==false || paused==true)
    return;

  GdkColor black = {0, 0, 0, 0};
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  GdkColor foreground = colours[Color];

  gdk_color_alloc(colormap, &foreground);
  gdk_gc_set_foreground (graphics_context, &foreground);
  
  if( x2 > x1 ) {
    if( y2 > y1 ) {
      gdk_draw_rectangle (offscreen_buffer, graphics_context, TRUE, x1, y1, x2-x1, y2-y1);
      gdk_gc_set_foreground (graphics_context, &black);
      gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, x1, y1, x2-x1, y2-y1);
    }
    else {
      gdk_draw_rectangle (offscreen_buffer, graphics_context, TRUE, x1, y2, x2-x1, y1-y2);
      gdk_gc_set_foreground (graphics_context, &black);
      gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, x1, y2, x2-x1, y1-y2);
    }
  }
  else {
    if( y2 > y1 ) {
      gdk_draw_rectangle (offscreen_buffer, graphics_context, TRUE, x2, y1, x1-x2, y2-y1);
      gdk_gc_set_foreground (graphics_context, &black);
      gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, x2, y1, x1-x2, y2-y1);
    }
    else {
      gdk_draw_rectangle (offscreen_buffer, graphics_context, TRUE, x2, y2, x1-x2, y1-y2);
      gdk_gc_set_foreground (graphics_context, &black);
      gdk_draw_rectangle (offscreen_buffer, graphics_context, FALSE, x2, y2, x1-x2, y1-y2);
    }
  }

  gdk_gc_set_foreground (graphics_context, &black);
}

void draw_polyline_callback(Dasher::CDasherScreen::point* Points, int Number)
{ 
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkRectangle update_rect;

  if (setup==false || paused==true)
    return;

  GdkColor black = {0, 0, 0, 0};
  GdkPoint *gdk_points;

  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();

  gdk_color_alloc(colormap, &black);
  gdk_gc_set_foreground (graphics_context, &black);

  for (int i=0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_lines(offscreen_buffer, graphics_context, gdk_points, Number);
}

void draw_text_callback(symbol Character, int x1, int y1, int size)
{
  std::string symbol;
  GdkRectangle update_rect;

  if (setup==false || paused==true)
    return;

  pango_font_description_set_size( font,size*PANGO_SCALE);

  pango_layout_set_font_description(the_pangolayout,font);

  symbol = dasher_get_display_text( Character );

  pango_layout_set_text(the_pangolayout,symbol.c_str(),-1);

  pango_layout_get_pixel_extents(the_pangolayout,ink,logical);

  gdk_draw_layout (offscreen_buffer,
		   the_canvas->style->black_gc,
		   x1, y1-(ink->height/2.0), the_pangolayout);

}

void draw_text_string_callback(std::string String, int x1, int y1, int size)
{
  GdkRectangle update_rect;

  if (setup==false || paused==true)
    return;

  pango_font_description_set_size( font,size*PANGO_SCALE);

  pango_layout_set_font_description(the_pangolayout,font);

  pango_layout_set_text(the_pangolayout,String.c_str(),-1);

  pango_layout_get_pixel_extents(the_pangolayout,ink,logical);

  gdk_draw_layout (offscreen_buffer,
		   the_canvas->style->black_gc,
		   x1, y1-(ink->height/2.0), the_pangolayout);

}

void text_size_callback(symbol Character, int* Width, int* Height, int Size)
{
  // FIXME

  if (setup==false || paused==true)
    return;

  *Width = Size;
  *Height = Size;
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
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkRectangle update_rect;
  GdkColor color;
  graphics_context = the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)];
  colormap = gdk_colormap_get_system();
  int top, bottom;

  paused=false;
  dasher_render();
  paused=true;

  switch (which) {
  case 0:
    color.pixel=0;
    color.red=255*257;
    color.green=0*257;
    color.blue=0*257;
    top=0;
    bottom=100;
    break;
  case 1:
    color.pixel=0;
    color.red=255*257;
    color.green=255*257;
    color.blue=0*257;
    bottom=the_canvas->allocation.height;
    top=bottom-100;
    break;
  }

  gdk_draw_pixmap(onscreen_buffer,
  		  the_canvas->style->fg_gc[GTK_WIDGET_STATE (the_canvas)],
  		  offscreen_buffer,
  		  0, 0, 0,0,
		  the_canvas->allocation.width,
		  the_canvas->allocation.height);

  gdk_color_alloc(colormap, &color);
  gdk_gc_set_foreground (graphics_context, &color);
  gdk_draw_rectangle (onscreen_buffer, graphics_context, TRUE, 0, top, the_canvas->allocation.width, bottom);

  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = the_canvas->allocation.width;
  update_rect.height = the_canvas->allocation.height;

  gtk_widget_draw(the_canvas,&update_rect);
}




