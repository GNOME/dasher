#include "Gtk2DoubleBuffer.h"

#include <gdk/gdk.h>

Gtk2DoubleBuffer::Gtk2DoubleBuffer(GdkWindow *window, gint width, gint height, gint depth=-1) : swapped( false )
{
  fg_buffer = gdk_pixmap_new(window, width, height, depth);
  bg_buffer = gdk_pixmap_new(window, width, height, depth);
}

Gtk2DoubleBuffer::~Gtk2DoubleBuffer()
{
}

GdkPixmap *Gtk2DoubleBuffer::get_fg()
{
  if (swapped)
    return (bg_buffer);
  else
    return (fg_buffer);
}

GdkPixmap *Gtk2DoubleBuffer::get_bg()
{
  if (swapped)
    return (fg_buffer);
  else
    return (bg_buffer);
}

void Gtk2DoubleBuffer::swap_buffers()
{
  swapped = !swapped;
}



