#include "GtkDoubleBuffer.h"

GtkDoubleBuffer::GtkDoubleBuffer( gint        width, 
				 gint        height,
				 gint        depth=-1)
  : fg_buffer( width, height, depth ), bg_buffer( width, height, depth ), swapped( false )
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
