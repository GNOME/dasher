#ifndef GTKDOUBLEBUFFER_H
#define GTKDOUBLEBUFFER_H

#include <gdk--/drawable.h>
#include <gdk--/pixmap.h>
#include <gdk--/bitmap.h>
#include <gdk/gdk.h>

class GtkDoubleBuffer
{
 public:
  GtkDoubleBuffer( gint        width, 
                  gint        height,
                  gint        depth);
  ~GtkDoubleBuffer();

  Gdk_Pixmap *get_fg();
  Gdk_Pixmap *get_bg_text();  
  Gdk_Pixmap *get_map_text();
  Gdk_Pixmap *get_bg_squares();
  
  
  void swap_buffers(Gdk_GC *gc);


  Gdk_Pixmap fg_buffer;
  Gdk_Pixmap bg_buffer_text;
  Gdk_Pixmap bg_buffer_squares;

  Gdk_Pixmap bg_map_text;
 protected:
  //  bool swapped;

  int pmwidth;
  int pmheight;
};

#endif
