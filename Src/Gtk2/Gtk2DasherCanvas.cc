#include <iostream>
#include <string>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "Gtk2DasherCanvas.h"

#include "DasherScreen.h"

#define MAXFONTSIZE 25
#define MINFONTSIZE 8

Gtk2DasherCanvas::Gtk2ScreenWrapper::Gtk2ScreenWrapper (int width, int height, Gtk2DasherCanvas *owner )
  : CDasherScreen(width, height )
{
  this->owner = owner;
}

Gtk2DasherCanvas::Gtk2DasherCanvas(guint width, guint height, CDasherInterface *interface) : pmwidth (width), pmheight (height)
{
  // we've got a canvas being passed to us. 
  // apparently want to do stuff regarding fonts list...

  wrapper = new Gtk2ScreenWrapper(width, height, this);

  this->interface = interface;
  canvas = gtk_drawing_area_new ();
  
  buffer = new Gtk2DoubleBuffer(canvas->window, width, height, 16);
}

void Gtk2DasherCanvas::CreateNewBuffer()
{
  delete(buffer);
  buffer = new Gtk2DoubleBuffer(canvas->window, canvas->allocation.width, canvas->allocation.height, 16);

  // may need to draw a white rectangle on all of them at this point.
}

Gtk2DasherCanvas::~Gtk2DasherCanvas()
{
  delete(buffer);
}

void Gtk2DasherCanvas::SetFont(std::string Name)
{
}

GdkColor Gtk2DasherCanvas::get_color(int Color, Opts::ColorSchemes ColorScheme) const {
  if (ColorScheme == Special1) {
    GdkColor foreground = { 0, 240*257, 240*257, 240*257 };
    return foreground;
  }
  if (ColorScheme == Special2) {
    GdkColor foreground = { 0, 255*257, 255*257, 255*257 };
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

GdkFont *Gtk2DasherCanvas::GetFont(int size) const
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

void Gtk2DasherCanvas::TextSize(symbol Character, int* Width, int* Height, int Size) const
{
  GdkFont *chosen_font;

  chosen_font = GetFont(Size);
    
  *Width = gdk_char_height(chosen_font, ('A'));
  *Height = gdk_char_height(chosen_font, ('A'));
}

void Gtk2DasherCanvas::DrawText(symbol Character, int x1, int y1, int size) const 
{
  std::string symbol;
  GdkRectangle update_rect;
  GdkFont *chosen_font;  

  symbol = interface->GetDisplayText(Character);

  chosen_font = GetFont(size);

  gdk_draw_string (buffer->get_bg(),
		   chosen_font,
		   canvas->style->black_gc,
		   x1, y1+gdk_char_height(chosen_font, ('A')), symbol.c_str());

  /*
  update_rect.x = x1;
  update_rect.y = y1+gdk_char_height(chosen_font, ('A'));
  update_rect.width = gdk_string_width(chosen_font, symbol.c_str());
  update_rect.height = gdk_char_height(chosen_font, ('A'));

  gtk_widget_draw (GTK_WIDGET (canvas), &update_rect);
  */
  return;
}
  
void Gtk2DasherCanvas::DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const
{

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkRectangle update_rect;

  GdkColor black = {0, 0, 0, 0};
  GdkColor new_foreground = { 0, 45000, 45000, 45000 };
  GdkColor foreground = {0, ((ColorScheme * 3 + Color) & 1) * 30000 + 30000, 
			 ((ColorScheme * 3 + Color) >> 1 ) * 30000 + 30000, 
			 ((ColorScheme * 3 + Color) >> 2 ) * 30000 + 30000 };
  
  graphics_context = canvas->style->fg_gc[GTK_WIDGET_STATE (canvas)];
  colormap = gdk_colormap_get_system();

  foreground = get_color(Color, ColorScheme);

  gdk_color_alloc(colormap, &foreground);
  gdk_gc_set_foreground (graphics_context, &foreground);
  
  if( x2 > x1 ) {
    if( y2 > y1 ) {
      gdk_draw_rectangle (buffer->get_bg(), graphics_context, TRUE, x1, y1, x2-x1, y2-y1);
    }
    else {
      gdk_draw_rectangle (buffer->get_bg(), graphics_context, TRUE, x1, y2, x2-x1, y1-y2);
    }
  }
  else {
    if( y2 > y1 ) {
      gdk_draw_rectangle (buffer->get_bg(), graphics_context, TRUE, x2, y1, x1-x2, y2-y1);
    }
    else {
      gdk_draw_rectangle (buffer->get_bg(), graphics_context, TRUE, x2, y2, x1-x2, y1-y2);
    }
  }

  gdk_gc_set_foreground (graphics_context, &black);
 
  return;
}

void Gtk2DasherCanvas::Polyline(Dasher::CDasherScreen::point* Points, int Number) const
{ 
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkRectangle update_rect;

  GdkColor black = {0, 0, 0, 0};
  GdkPoint gdk_points[Number];

  graphics_context = canvas->style->fg_gc[GTK_WIDGET_STATE (canvas)];
  colormap = gdk_colormap_get_system();

  gdk_color_alloc(colormap, &black);
  gdk_gc_set_foreground (graphics_context, &black);

  for (int i=0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_lines(buffer->get_bg(), graphics_context, gdk_points, Number);
  /*
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = canvas->allocation.width;
  update_rect.height = canvas->allocation.height;

  gtk_widget_draw (GTK_WIDGET (canvas), &update_rect);
  */
  return;
}
 
void Gtk2DasherCanvas::DrawPolygon(Dasher::CDasherScreen::point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const
{
}

void Gtk2DasherCanvas::Blank() const
{
  gdk_draw_rectangle (buffer->get_bg(),
		      canvas->style->white_gc,
                      TRUE,
                      0, 0,
		      canvas->allocation.width,
		      canvas->allocation.height);
}

void Gtk2DasherCanvas::Display()
{
  GdkRectangle update_rect;
  buffer->swap_buffers();

  gdk_draw_rectangle (buffer->get_bg(),
		      canvas->style->white_gc,
                      TRUE,
                      0, 0,
		      canvas->allocation.width,
		      canvas->allocation.height);
  
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = canvas->allocation.width;
  update_rect.height = canvas->allocation.height;

  //gtk_widget_queue_draw_area(canvas,0,0, canvas->allocation.width,canvas->allocation.height);
  gtk_widget_draw(canvas,&update_rect);
}



