#include <gdk--/font.h>
#include <gtk--/style.h>
#include <gtk--/pixmap.h>
#include <gdk/gdk.h>
#include <iostream.h>
#include <string>

#include "GtkDasherCanvas.h"

#include "DasherScreen.h"

GtkDasherCanvas::GtkDasherCanvas( int _width, int _height)
  : DrawingArea(), CDasherScreen( _width, _height ), width( _width), height( _height )
{
  set_events( GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK );

  set_usize(width, height);

  //  fg_buffer = gdk_pixmap_new( get_window(), width, height, -1 );
  //  bg_buffer = gdk_pixmap_new( get_window(), width, height, -1 );

  //fg_buffer = new Gdk_Pixmap( width, height, -1 );
  //bg_buffer = new Gdk_Pixmap(  width, height, -1 );
    buffer = new GtkDoubleBuffer( width, height, 16 );

    f_small.create("-misc-fixed-medium-r-*-*-*-110-*-*-*-*-*-*");
    f_medium.create("-misc-fixed-medium-r-*-*-*-110-*-*-*-*-*-*");
    f_large.create("-misc-fixed-medium-r-*-*-*-110-*-*-*-*-*-*");
}

Gdk_Font GtkDasherCanvas::get_font( int size ) const
{
  return( f_small );
}

void GtkDasherCanvas::clear()

  {
    // Wipe the two drawing areas:
    if( is_realized() )
      {
   Gdk_GC graphics_context;
  graphics_context.create(get_window());


  // FIXME - This is very broked - we should set up the colour map in advance

  Gdk_Color some_color;
  Gdk_Colormap some_colormap(Gdk_Colormap::get_system());
  some_color.set_red(65535);
  some_color.set_green(65535);
  some_color.set_blue(65535);
  some_colormap.alloc(some_color);
  graphics_context.set_foreground(some_color);
    

  buffer->get_bg()->draw_rectangle( graphics_context, true, 0, 0, width, height );
  buffer->get_fg()->draw_rectangle( graphics_context, true, 0, 0, width, height );

      }

}

GtkDasherCanvas::~GtkDasherCanvas()
{
  //  delete( fg_buffer );
  // delete( bg_buffer );
  delete( buffer );
}

gint GtkDasherCanvas::expose_event_impl(GdkEventExpose *event)
{
  get_window().draw_pixmap( this->get_style()->get_white_gc(), *(buffer->get_fg()), 0, 0, 0, 0, width, height );

  return( true );
}


//  gint GtkDasherCanvas::button_press_event_impl(GdkEventButton *event)
//    {
//      cout << "foo" << endl;
//      return( true );
//    }


void GtkDasherCanvas::SetFont(std::string Name)
{
}

void GtkDasherCanvas::TextSize(symbol Character, int* Width, int* Height, int Size) const
{
  Gdk_Font chosen_font;

  chosen_font = get_font( Size );

  *Width = chosen_font.char_width('A');
  *Height = chosen_font.char_height('A');
}
 
void GtkDasherCanvas::DrawText(symbol Character, int x1, int y1, int Size) const
{
  // Fixme - need to do symbol->character lookup properly.

  if( is_realized() )
    {
  char foo;

  foo = Character + 96;

  string label(&foo, 1);

  Gdk_Font chosen_font;

  chosen_font = get_font( Size );

  buffer->get_bg()->draw_string(chosen_font, this->get_style()->get_black_gc(), x1, y1+chosen_font.char_height('A'), label);
    }    
}
  
void GtkDasherCanvas::DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const
{
  if( is_realized() )
    {
  Gdk_GC graphics_context;
  graphics_context.create(get_window()); 

  Gdk_Color some_color;
  Gdk_Colormap some_colormap(Gdk_Colormap::get_system());
  some_color.set_red((Color & 1) * 30000 + 30000 );
  some_color.set_green(((Color & 2) >> 1 ) * 30000 + 30000);
  some_color.set_blue(((Color & 3) >> 2 ) * 30000 + 30000);
  some_colormap.alloc(some_color);
  graphics_context.set_foreground(some_color);


  buffer->get_bg()->draw_rectangle( graphics_context, true, x1, y1, x2-x1, y2-y1 );
    }
}

void GtkDasherCanvas::Polyline(point* Points, int Number) const
{ 
  if( is_realized() )
    {
  Gdk_GC graphics_context;
  graphics_context.create(get_window()); 

  Gdk_Color some_color;
  Gdk_Colormap some_colormap(Gdk_Colormap::get_system());
  some_color.set_red(0);
  some_color.set_green(0);
  some_color.set_blue(0);
  some_colormap.alloc(some_color);
  graphics_context.set_foreground(some_color);

  for( int i(0); i < Number - 1; ++i )
    buffer->get_bg()->draw_line( graphics_context, Points[i].x, Points[i].y, Points[i+1].x, Points[i+1].y );
    }
}
 
void GtkDasherCanvas::DrawPolygon(point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const
{
}

void GtkDasherCanvas::Blank() const
{
  if( is_realized() )
    {
     Gdk_GC graphics_context;
  graphics_context.create(get_window());


  // FIXME - This is very broked - we should set up the colour map in advance

  Gdk_Color some_color;
  Gdk_Colormap some_colormap(Gdk_Colormap::get_system());
  some_color.set_red(65535);
  some_color.set_green(65535);
  some_color.set_blue(65535);
  some_colormap.alloc(some_color);
  graphics_context.set_foreground(some_color);
    

  buffer->get_bg()->draw_rectangle( graphics_context, true, 0, 0, width, height );
    }
 }

void GtkDasherCanvas::Display()
{
  swap_buffers();

  draw(0);
}

void GtkDasherCanvas::swap_buffers()
{
  buffer->swap_buffers();
}












