#ifndef GPEDOUBLEBUFFER_H
#define GPEDOUBLEBUFFER_H

#include <gdk/gdk.h>
#include <glib.h>

class Gtk2DoubleBuffer
{
 public:
  Gtk2DoubleBuffer(GdkWindow *window, gint width, gint height, gint depth);
  ~Gtk2DoubleBuffer();
  
  GdkPixmap *get_fg();
  GdkPixmap *get_bg();
  
  void swap_buffers();
  
  GdkPixmap *fg_buffer;
  GdkPixmap *bg_buffer;

 protected:
  gboolean swapped;
};

#endif

