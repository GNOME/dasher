#include <gdk--/font.h>
#include <gtk--/style.h>
#include <gtk--/pixmap.h>
#include <gdk/gdk.h>
#include <iostream.h>
#include <string>

#include "GtkDasherCanvas.h"

#include "DasherScreen.h"

GtkDasherCanvas::GtkDasherCanvas( int _width, int _height, CDasherInterface *_interface )
  : DrawingArea(), CDasherScreen( _width, _height ), width( _width), height( _height ), 
    interface( _interface )
{
  font_list = new Gdk_Font[17];

  set_events( GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK );

  set_usize(width, height);
  buffer = new GtkDoubleBuffer( width, height, 16 );
  
  // FIXME - Gah - this is so broken
  // (a) Need to do something more sane than this (ie only creating the font objects as they are required)
  // (b) Making sure that font objects for all sizes are drawable (I have no idea how to do this)

  char buffer[256];
  
  for( int i(0); i < 17; ++i )
    {

      
      sprintf( buffer, "-*-fixed-*-*-*-*-20-*-*-*-*-*-*-*" );
      
      font_list[i].create( buffer );
    }
}

const Gdk_Font *GtkDasherCanvas::get_font( int size ) const
{
  //  if( size < 12 )
  //    return( &f_small );
  // else if( size < 17 )
  //  return( &f_medium );
  // else
  //  return( &f_large );
  return( &font_list[size-8] );
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
  const Gdk_Font *chosen_font;

  chosen_font = get_font( Size );

  *Width = chosen_font->char_width('A');
  *Height = chosen_font->char_height('A');
}
 
void GtkDasherCanvas::DrawText(symbol Character, int x1, int y1, int Size) const
{

  // FIXME - symbol here will be a utf-8 encoded string, but I don't
  // know whether draw_string wants that or whether it wants just
  // ascii / whatever the character set of the font is

  if( is_realized() )
    {

      string symbol;
      symbol = interface->GetDisplayText(Character);

      const Gdk_Font *chosen_font;
      
      chosen_font = get_font( Size );
      
      buffer->get_bg()->draw_string(*chosen_font, this->get_style()->get_black_gc(), x1, y1+chosen_font->char_height('A'), symbol);
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
  some_color.set_red(((ColorScheme * 3 + Color) & 1) * 30000 + 30000 );
  some_color.set_green(((ColorScheme * 3 + Color) >> 1 ) * 30000 + 30000);
  some_color.set_blue(((ColorScheme * 3 + Color) >> 2 ) * 30000 + 30000);

  //  cout << "Colour: " << Color << " (" << (Color & 1) * 30000 + 30000 << ", " << ((Color & 2) >> 1 ) * 30000 + 30000 << ", " << ((Color & 3) >> 2 ) * 30000 + 30000 << " = " << ColorScheme << endl;

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












