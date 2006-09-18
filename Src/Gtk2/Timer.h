#ifndef __timer_h__
#define __timer_h__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

gint timer_callback(gpointer data);
gint long_timer_callback(gpointer data);
long get_time();

#endif
