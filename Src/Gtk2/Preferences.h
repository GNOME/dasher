#ifndef __preferences_h__
#define __preferences_h__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glade/glade.h>

void initialise_preferences_dialogue(GladeXML * pGladeWidgets);
void update_advanced(int iParameter);
void preferences_handle_parameter_change(int iParameter);
extern "C" void preferences_display(GtkWidget *widget, gpointer user_data);
#endif
