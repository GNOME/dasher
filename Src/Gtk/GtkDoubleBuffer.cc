#include "GtkDoubleBuffer.h"

GtkDoubleBuffer::GtkDoubleBuffer(const Gdk_Drawable  &drawable,
				 gint        width, 
				 gint        height,
				 gint        depth=-1)
  : fg_buffer( drawable, width, height, depth ), bg_buffer( drawable, width, height, depth ), swapped( false )
{
}

GtkDoubleBuffer::~GtkDoubleBuffer()
{
}

Gdk_Pixmap *GtkDoubleBuffer::get_fg()
{
  if( swapped )
    return( &bg_buffer );
  else
    return( &fg_buffer );
}

Gdk_Pixmap *GtkDoubleBuffer::get_bg()
{
  if( swapped )
    return( &fg_buffer );
  else
    return( &bg_buffer );
}

void GtkDoubleBuffer::swap_buffers()
{
  swapped = !swapped;
}
