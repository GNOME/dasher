#ifndef GTKDOUBLEBUFFER_H
#define GTKDOUBLEBUFFER_H

#include <gdk--/drawable.h>
#include <gdk--/pixmap.h>

class GtkDoubleBuffer
{
 public:
  GtkDoubleBuffer( gint        width, 
                  gint        height,
                  gint        depth);
  ~GtkDoubleBuffer();

  Gdk_Pixmap *get_fg();
  Gdk_Pixmap *get_bg();
  
  void swap_buffers();


  Gdk_Pixmap fg_buffer;
  Gdk_Pixmap bg_buffer;
 protected:
  bool swapped;
};

#endif
