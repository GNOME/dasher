#ifndef __menu_h__
#define __menu_h__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glade/glade.h>

extern "C" void clipboard_copy(void);
extern "C" void about_dasher(GtkWidget *widget, gpointer user_data);

#endif
