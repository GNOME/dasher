#include "GtkDoubleBuffer.h"

#include "gtk--/pixmap.h"

GtkDoubleBuffer::GtkDoubleBuffer( gint        width, 
				 gint        height,
				 gint        depth=-1)
  : fg_buffer( width, height, depth ), bg_buffer_text( width, height, depth ), bg_buffer_squares( width, height, depth ), bg_map_text( width, height, 1 )
{
  pmwidth = width;
  pmheight = height;

}

GtkDoubleBuffer::~GtkDoubleBuffer()
{
}

Gdk_Pixmap *GtkDoubleBuffer::get_fg()
{
  return( &fg_buffer );
}

Gdk_Pixmap *GtkDoubleBuffer::get_bg_text()
{
  return( &bg_buffer_text );
}

Gdk_Pixmap *GtkDoubleBuffer::get_map_text()
{ 
  return( &bg_map_text );
}

Gdk_Pixmap *GtkDoubleBuffer::get_bg_squares()
{
  return( &bg_buffer_squares );
}

void GtkDoubleBuffer::swap_buffers( Gdk_GC *gc)
{
  //  swapped = !swapped;

  //  Gdk_GC graphics_context;
  //graphics_context.create( get_window());

    fg_buffer.draw_pixmap( *gc, bg_buffer_squares, 0, 0, 0, 0, pmwidth, pmheight );

  GdkBitmap *bm;

  bm = static_cast<GdkBitmap *>( bg_map_text.gdkobj() );


  Gdk_Bitmap gbm( bm );
  

  gc->set_clip_mask( gbm );

  fg_buffer.draw_pixmap( *gc, bg_buffer_text, 0, 0, 0, 0, pmwidth, pmheight );
  //fg_buffer.draw_bitmap( *gc, gbm, 0, 0, 0, 0, pmwidth, pmheight );
    
}
