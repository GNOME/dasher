// GtkDasherCanvas.cc
// (c) 2002 Philip Cowans

#include <gdk--/font.h>
#include <gtk--/style.h>
#include <gtk--/pixmap.h>
#include <gdk/gdk.h>
#include <iostream>

#include <string>

#include "GtkDasherCanvas.h"

#include "DasherScreen.h"

#include <X11/Xlib.h>
#include <gdk/gdkx.h>

#include <iconv.h>

GtkDasherCanvas::GtkScreenWrapper::GtkScreenWrapper(  int _width, int _height, GtkDasherCanvas *_owner )
 : owner( _owner ), CDasherScreen( _width, _height )
{
}

GtkDasherCanvas::GtkDasherCanvas( int _width, int _height, CDasherInterface *_interface )
  : DrawingArea(), pmwidth( _width), pmheight( _height ), 
  interface( _interface ), fontname( "fixed" )
{ 
  enc = 1; // Asume encoding is iso8859-1 unless we're told otherwise.

  cdesc = iconv_open( "ISO-8859-1", "UTF-8" );

  wrapper = new GtkScreenWrapper( _width, _height, this );

  // Tell the system we want to receive button press events

  set_events( GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK );

  // Set the size of the drawing area

  set_usize(pmwidth, pmheight);

  // Initialise the double buffer class

  buffer = new GtkDoubleBuffer( pmwidth, pmheight, DefaultDepth(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL))));

  // Set the font sizes

  MaxFontSize = 20;
  MinFontSize = 8;

  // Initialise the font list

  font_list = new Gdk_Font[17];
  font_init = new bool[17];

  build_fonts( enc );

}

void GtkDasherCanvas::realize_impl()
{
  DrawingArea::realize_impl();

  build_colours();
}

bool GtkDasherCanvas::build_fonts( int encoding )
{
  for( int i(0); i < 17; ++i )
    font_init[i] = false;
 
  char **fontnames;
  int nfonts;

  char xfontstringbuffer[ 256 ];

  snprintf( xfontstringbuffer, 256, "-*-%s-*-*-*-*-*-*-*-*-*-*-iso8859-%d", fontname.c_str(), encoding );

  fontnames = XListFonts( GDK_DISPLAY(), xfontstringbuffer, 1024, &nfonts );

  char size_buffer[256];

  bool got_one( false );

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

      if(( fsize >= MinFontSize ) && ( fsize <= MaxFontSize ))
	{
	  int idx;
	  idx = fsize - MinFontSize;

	  if( !font_init[idx] )
	  {
	    font_init[idx] = true;
	    font_list[idx].create( fontnames[i] );
	    got_one = true;
	  }
	}

    }

  if( !got_one )
    std::cerr << "Warning - failed to find a font" << std::endl;

  return( got_one );

}

const Gdk_Font *GtkDasherCanvas::get_font( int size ) const
{
  int d( 10000 );
  int idx(-1);
  
  for( int i(0); i < MaxFontSize - MinFontSize; ++i )
    if( font_init[i] && ( abs( i - size + MinFontSize ) < d ) )
      {
	d =  abs(i - size + MinFontSize);
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

  Gdk_GC gc2;
  
  gc2.create(*(buffer->get_map_text()));

  GdkColor clr;

  clr.pixel = 0;

  Gdk_Color some_color2( clr );
  

//   buffer->get_map_text()->draw_rectangle( gc2, true, 0, 0, pmwidth, pmheight );
  
//   buffer->get_bg_text()->draw_rectangle( this->get_style()->get_white_gc(), true, 0, 0, pmwidth, pmheight );

  
//   buffer->get_bg_squares()->draw_rectangle( this->get_style()->get_white_gc(), true, 0, 0, pmwidth, pmheight );
//   buffer->get_fg()->draw_rectangle( this->get_style()->get_white_gc(), true, 0, 0, pmwidth, pmheight );

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
  // FIXME - resizing should probably be done on the configure_event
  // rather than the expose_event
  
  if(( width() != pmwidth ) || ( height() != pmheight ))
    {
      pmwidth = width();
      pmheight = height();

      delete( buffer );

      buffer = new GtkDoubleBuffer( pmwidth, pmheight, DefaultDepth(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL))));

      delete( wrapper );
      wrapper = new GtkScreenWrapper( pmwidth, pmheight, this );

      interface->ChangeScreen( wrapper );
    }

  Gdk_GC gc3;
  
  gc3.create(*(buffer->get_bg_text()));
  
  Gdk_Color clr3;
  
  clr3.set_rgb( 65535,65535,65535);
  
  gc3.set_foreground(clr3);


  get_window().draw_pixmap( gc3, *(buffer->get_fg()), 0, 0, 0, 0, pmwidth, pmheight );

  return( true );
}

void GtkDasherCanvas::SetFont(std::string Name)
{
  std::string actual_name( Name );

  // We want to use fixed if no font name is specified (which seems to
  // be the default)

  if( actual_name == "" )
    actual_name = "fixed";

  // Only rebuild the font list if the font name has actually changed
  // (this helps solve problems caused by repeated font changes that
  // happen during resizing)

  if( actual_name != fontname )
    {
      // Woo! Confusing structures - try the requested fontname, and
      // if that fails try fixed, then try to pick any font

      fontname = actual_name;
      if( !build_fonts( enc ) )
	{
	  fontname = "fixed";
	  if( !build_fonts( enc ) )
	    {
	      fontname = "*";
	      build_fonts( enc );
	    }
	}

      
    }
}

void GtkDasherCanvas::SetFontSize(FontSize size) {
  switch( size )
    {
    case Normal:
      MaxFontSize = 20;
      MinFontSize = 8;
      build_fonts( enc );
      break;
    case Big:
      MaxFontSize = 30;
      MinFontSize = 14;
      build_fonts( enc );
      break;
    case VBig:
      MaxFontSize = 40;
      MinFontSize = 20;
      build_fonts( enc );
      break;
    }
}

void GtkDasherCanvas::set_encoding( int _enc )
{
  // Set the ISO8859 codepage to be used (using gtk1.2 we are limited
  // to iso8859 characters, so we need to pick an appropriate subset
  // to be able to display).

  if( _enc != enc )
    {
      enc = _enc;

      iconv_close( cdesc );

      char encstr[256];

      snprintf( encstr, 255, "ISO-8859-%d", enc );

      cdesc = iconv_open( encstr, "UTF-8" );

      if( !build_fonts( enc ) )
	{
	  fontname = "fixed";
	  if( !build_fonts( enc ) )
	    {
	      fontname = "*";
	      build_fonts( enc );
	    }
	}
    }
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
  if( is_realized() )
    {
      std::string symbol;

      symbol = interface->GetDisplayText(Character);

      char *convbuffer = new char[256];
      char *inbuffer = new char[256];

      char *cb( convbuffer );
      char *ib( inbuffer );

      strncpy( inbuffer, symbol.c_str(), 255 );

      size_t inb = symbol.length();
      
      size_t outb = 256;

      iconv( cdesc, &inbuffer, &inb, &convbuffer, &outb );

      std::string csymbol( cb, 256-outb );


      delete cb;
      delete ib;

      const Gdk_Font *chosen_font;
      
      chosen_font = get_font( Size );

      Gdk_GC gc2;
      gc2.create(*(buffer->get_map_text()));
      
      GdkColor clr;

  clr.pixel = 1;

  Gdk_Color some_color2( clr );

      
      gc2.set_foreground(some_color2);

      //      Gtk::Style st( gtk_widget_get_style( buffer->get_map_text()->gdkobj() ) );

      Gdk_GC gc3;

      gc3.create(*(buffer->get_bg_text()));

      Gdk_Color clr3;

      clr3.set_rgb( 0,0,0);

      gc3.set_foreground(clr3);

      buffer->get_bg_text()->draw_string(*chosen_font, gc3, x1, y1+chosen_font->char_height('A'), csymbol);
      buffer->get_map_text()->draw_string(*chosen_font, gc2, x1, y1+chosen_font->char_height('A'), csymbol);

    }    
}

void GtkDasherCanvas::build_colours()
{
  Gdk_Colormap cm(Gdk_Colormap::get_system());

  // Create grphics contexts for the first set of nodes

  for( int i(0); i < 3; ++i )
    {
      nodes1gc[i].create(*(buffer->get_bg_squares())  );

      Gdk_Color col;
      switch(i)
	{
	case 0:
	  col.set_rgb(180*256,245*256,180*256);
	  break;
	case 1:
	  col.set_rgb(160*256,200*256,160*256);
	  break;
	case 2:
	  col.set_rgb(0*256,255*256,255*256);
	  break;
	}
      cm.alloc( col );

      nodes1gc[i].set_foreground( col );
    }

  // Create graphics contexts for the second row of nodes

  for( int i(0); i < 3; ++i )
    {
      nodes2gc[i].create( get_window() );

      Gdk_Color col;
      switch(i)
	{
	case 0:
	  col.set_rgb(255*256,185*256,255*256);
	  break;
	case 1:
	  col.set_rgb(140*256,200*256,255*256);
	  break;
	case 2:
	  col.set_rgb(255*256,175*256,175*256);
	  break;
	}
      cm.alloc( col );

      nodes2gc[i].set_foreground( col );
    }

  {
    special1gc.create( get_window() );

    Gdk_Color col;
    col.set_rgb(240*256,240*256,240*256);
    cm.alloc( col );

    special1gc.set_foreground( col );
  }

  {
    special2gc.create( get_window() );

    Gdk_Color col;
    col.set_rgb(255*256,255*256,255*256);
    cm.alloc( col );

    special2gc.set_foreground( col );
  }

  for( int i(0); i < 3; ++i )
    {
      groupsgc[i].create( get_window() );

      Gdk_Color col;
      switch(i)
	{
	case 0:
	  col.set_rgb(255*256,255*256,0*256);
	  break;
	case 1:
	  col.set_rgb(255*256,100*256,100*256);
	  break;
	case 2:
	  col.set_rgb(0*256,255*256,0*256);
	  break;
	}
      cm.alloc( col );

      groupsgc[i].set_foreground( col );
    }

}

const Gdk_GC *GtkDasherCanvas::get_color( int colour, Opts::ColorSchemes colour_scheme ) const
{
  switch( colour_scheme )
    {
    case Nodes1:
      return( &nodes1gc[ colour % 3 ] );
      break;
    case Nodes2:
      return( &nodes2gc[ colour % 3 ] );
      break;
    case Special1:
      return( &special1gc );
      break;
    case Special2:
      return( &special2gc );
      break;
    case Groups:
      return( &groupsgc[ colour % 3 ] );
      break;
    }
}
  
void GtkDasherCanvas::DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const
{
  if( is_realized() )
    {
      const Gdk_GC *graphics_context;

      graphics_context = get_color( Color, ColorScheme );

      if( x2 > x1 )
	if( y2 > y1 )
	  buffer->get_bg_squares()->draw_rectangle( *graphics_context, true, x1, y1, x2-x1, y2-y1 );
	else
	  buffer->get_bg_squares()->draw_rectangle( *graphics_context, true, x1, y2, x2-x1, y1-y2 );
      else
	if( y2 > y1 )
	  buffer->get_bg_squares()->draw_rectangle( *graphics_context, true, x2, y1, x1-x2, y2-y1 );
	else
	  buffer->get_bg_squares()->draw_rectangle( *graphics_context, true, x2, y2, x1-x2, y1-y2 );
    }
}

void GtkDasherCanvas::Polyline(Dasher::CDasherScreen::point* Points, int Number) const
{ 
  if( is_realized() )
    {
      Gdk_GC gc3;

      gc3.create(*(buffer->get_bg_squares()));

      Gdk_Color clr3;

      clr3.set_rgb( 0,0,0);

      gc3.set_foreground(clr3);

      for( int i(0); i < Number - 1; ++i )
	buffer->get_bg_squares()->draw_line( gc3, Points[i].x, Points[i].y, Points[i+1].x, Points[i+1].y );
    }
}
 
void GtkDasherCanvas::DrawPolygon(Dasher::CDasherScreen::point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const
{
}

void GtkDasherCanvas::Blank() const
{
  if( is_realized() )
    {

      // FIXME - need to get colourmap for the drawables, not
      // necessarily the system colour map (documentation is vague on
      // how to do this)

      Gdk_Colormap cm(Gdk_Colormap::get_system());
      Gdk_GC gc1;
      
      gc1.create(*(buffer->get_bg_text()));
      
      Gdk_Color some_color1;
      some_color1.set_rgb( 65535, 65535, 65535 );
      cm.alloc( some_color1 );

      gc1.set_foreground(some_color1);

      buffer->get_bg_text()->draw_rectangle( gc1, true, 0, 0, pmwidth, pmheight );
      
      Gdk_GC gc3;
      
      gc3.create(*(buffer->get_bg_squares()));
      
      Gdk_Color some_color3;
      some_color3.set_rgb( 65535, 65535, 65535 );
       cm.alloc( some_color3 );
      gc3.set_foreground(some_color3);
      
      buffer->get_bg_squares()->draw_rectangle( gc3, true, 0, 0, pmwidth, pmheight );
      
      Gdk_GC gc2;
      
      gc2.create(*(buffer->get_map_text()));
      
      GdkColor clr;
      
      clr.pixel = 0;
     
      Gdk_Color some_color2( clr );
      //cm.alloc( some_color2 );
      gc2.set_foreground(some_color2);
      
      buffer->get_map_text()->draw_rectangle( gc2, true, 0, 0, pmwidth, pmheight );
    
    }
 }

void GtkDasherCanvas::Display()
{
  swap_buffers();

  draw(0);
}

void GtkDasherCanvas::swap_buffers()
{
  // Only swap the buffers if the canvas has been realised (otherwise
  // we will be unable to get a graphics context).

  if( is_realized() )
    {
      Gdk_GC graphics_context;
      graphics_context.create( get_window());
      
      buffer->swap_buffers( &graphics_context );
    }
}












