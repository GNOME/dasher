#include <gdk--/font.h>
#include <gtk--/style.h>
#include <gtk--/pixmap.h>
#include <gdk/gdk.h>
#include <iostream.h>

#include <string>

#include "GtkDasherCanvas.h"

#include "DasherScreen.h"

#include <X11/Xlib.h>
#include <gdk/gdkx.h>

#define MAXFONTSIZE 25
#define MINFONTSIZE 8

GtkDasherCanvas::GtkDasherCanvas( int _width, int _height, CDasherInterface *_interface )
  : DrawingArea(), CDasherScreen( _width, _height ), pmwidth( _width), pmheight( _height ), 
  interface( _interface ), fontname( "fixed" )
{ 

  // Tell the system we want to receive button press events

  set_events( GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK );

  // Set the size of the drawing area

  set_usize(pmwidth, pmheight);

  // Initialise the double buffer class

  buffer = new GtkDoubleBuffer( pmwidth, pmheight, 16 );

  // Initialise the font list

  font_list = new Gdk_Font[17];
  font_init = new bool[17];

  build_fonts();

}

void GtkDasherCanvas::build_fonts()
{
  for( int i(0); i < 17; ++i )
    font_init[i] = false;
 
  char **fontnames;
  int nfonts;

  char xfontstringbuffer[ 256 ];

  snprintf( xfontstringbuffer, 256, "-*-%s-*-*-*-*-*-*-*-*-*-*-*-*", fontname.c_str() );

  fontnames = XListFonts( GDK_DISPLAY(), xfontstringbuffer, 1024, &nfonts );

  char size_buffer[256];

  for( int i(0); i < nfonts; ++i )
    {
      int str_pos(0);
      int field(0);

      for( int a(0); a < strlen( fontnames[i] ); ++a )
	{
	  if( fontnames[i][a] == '-' )
	    ++field;
	  else
	    if( (field == 7) && (str_pos < 255))
	      {
		size_buffer[str_pos] = fontnames[i][a];
		++str_pos;
	      }
	}

      size_buffer[str_pos] = 0;

      int fsize;

      fsize = atoi( size_buffer );

      if(( fsize >= MINFONTSIZE ) && ( fsize <= MAXFONTSIZE ))
	{
	  int idx;
	  idx = fsize - MINFONTSIZE;

	  if( !font_init[idx] )
	  {
	    font_init[idx] = true;
	    font_list[idx].create( fontnames[i] );
	  }
	}

    }

}

const Gdk_Font *GtkDasherCanvas::get_font( int size ) const
{
  int d( 10000 );
  int idx(-1);
  
  for( int i(0); i < MAXFONTSIZE - MINFONTSIZE; ++i )
    if( font_init[i] && ( abs( i - size + MINFONTSIZE ) < d ) )
      {
	d =  abs(i - size + MINFONTSIZE);
	idx = i;
      }

  if( idx == -1 )
    return( NULL );
  else
    return( &font_list[idx] );
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
    

  buffer->get_bg()->draw_rectangle( graphics_context, true, 0, 0, pmwidth, pmheight );
  buffer->get_fg()->draw_rectangle( graphics_context, true, 0, 0, pmwidth, pmheight );

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
  if(( width() != pmwidth ) || ( height() != pmheight ))
    {
      cout << "Resizing canvas" << endl;

      pmwidth = width();
      pmheight = height();

      delete( buffer );

      buffer = new GtkDoubleBuffer( pmwidth, pmheight, 16 );

      // Need to tell the interface that its dimensions have changed here.

      interface->Redraw();
    }

  get_window().draw_pixmap( this->get_style()->get_white_gc(), *(buffer->get_fg()), 0, 0, 0, 0, pmwidth, pmheight );

  return( true );
}


//  gint GtkDasherCanvas::button_press_event_impl(GdkEventButton *event)
//    {
//      cout << "foo" << endl;
//      return( true );
//    }


void GtkDasherCanvas::SetFont(std::string Name)
{
  cout << "Setting font to " << Name << endl;

  if( Name == "" )
    fontname = "fixed";
  else
    fontname = Name;
  build_fonts();
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
    

  buffer->get_bg()->draw_rectangle( graphics_context, true, 0, 0, pmwidth, pmheight );
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












