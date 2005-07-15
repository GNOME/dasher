#include "canvas.h"

#include "Canvas.h"

#include "DasherControlPrivate.h"

#include <iostream>
#include <sstream>


GtkWidget *the_canvas;
GdkPixmap *offscreen_display_buffer;
GdkPixmap *offscreen_decoration_buffer;
GdkPixmap *offscreen_buffer;
GdkPixmap *onscreen_buffer;
PangoLayout *the_pangolayout;
std::map< std::string, PangoLayout * > oPangoCache;

PangoFontDescription *font;
PangoRectangle *ink,*logical;


extern gboolean setup,preferences;
extern long mouseposstartdist;
extern gboolean firstbox, secondbox,paused;

gboolean firsttime = TRUE;



gboolean drawoutline=FALSE;

/// Regenerate the on- and off- screen rendering buffers

// GTK event handlers for the canvas

extern "C" gint
canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  ((CCanvas *)data)->ExposeEvent( event );

  return TRUE;
}

extern "C" gint
canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  ((CDasherControl *)data)->CanvasConfigureEvent();

  // Fixme - reimplement sanely

//   // If the canvas is resized, we need to regenerate all of the buffers
//   rebuild_buffer();

//   dasher_resize_canvas( the_canvas->allocation.width, the_canvas->allocation.height );

//   dasher_redraw();

//   if (setup==TRUE) {
//     // If we're set up and resized, then save those settings

//     // FIXME - Reimplement this

// //     dasher_set_parameter_int(INT_EDITHEIGHT,gtk_paned_get_position(GTK_PANED(glade_xml_get_widget(widgets,"vpaned1"))));
// //     gtk_window_get_size(GTK_WINDOW(window), &dasherwidth, &dasherheight);
// //     dasher_set_parameter_int(INT_SCREENHEIGHT, dasherheight);
// //     dasher_set_parameter_int(INT_SCREENWIDTH, dasherwidth);
//   }

  return FALSE;
}


