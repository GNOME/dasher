/* \file Canvas.cpp
   \brief Class definitions for CCanvas
*/

#include "Canvas.h"
#include "DasherControl.h"

#include "../DasherCore/DasherTypes.h"

#include <iostream>

using namespace Dasher;

extern "C" gint canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);

CCanvas::CCanvas( GtkWidget *pCanvas, CPangoCache *pPangoCache ) 
  : CDasherScreen( pCanvas->allocation.width, pCanvas->allocation.height ) {

  m_pCanvas = pCanvas;
  m_pPangoCache = pPangoCache;

  m_iWidth = m_pCanvas->allocation.width;
  m_iHeight = m_pCanvas->allocation.height;

  // Construct the buffer pixmaps

  m_pDisplayBuffer = gdk_pixmap_new( pCanvas->window, m_iWidth, m_iHeight, -1 );
  m_pDecorationBuffer = gdk_pixmap_new( pCanvas->window, m_iWidth, m_iHeight, -1 );
  m_pOnscreenBuffer = gdk_pixmap_new( pCanvas->window, m_iWidth, m_iHeight, -1 );

  std::cout << m_pDisplayBuffer << " " << m_pDecorationBuffer << " " << m_pOnscreenBuffer << std::endl;

  // Set the display buffer to be current

  m_pOffscreenBuffer = m_pDisplayBuffer;

//   FIXME - pango cache stuff - implement in another object

//   the_pangolayout = gtk_widget_create_pango_layout (GTK_WIDGET(the_canvas), "");
//   font = pango_font_description_new();
//   pango_font_description_set_family( font,"Serif");

  m_pPangoInk = new PangoRectangle;
  m_pPangoLogical = new PangoRectangle;

//   FIXME - signal connectors - implement out of class

  std::cout << "Connecting: " << this << std::endl;

  lSignalHandler = g_signal_connect( m_pCanvas, "expose_event", G_CALLBACK(canvas_expose_event), this );

  std::cout << "Signal handler: " << lSignalHandler << std::endl;


   gtk_widget_add_events ( m_pCanvas, GDK_EXPOSURE_MASK );
   gtk_widget_add_events ( m_pCanvas, GDK_BUTTON_PRESS_MASK);
   gtk_widget_add_events ( m_pCanvas, GDK_BUTTON_RELEASE_MASK);

}

CCanvas::~CCanvas() {
  // Free the buffer pixmaps

  g_free( m_pDisplayBuffer );
  g_free( m_pDecorationBuffer );
  g_free( m_pOnscreenBuffer );

  std::cout << "Disconnecting: " << lSignalHandler << std::endl;

  g_signal_handler_disconnect( m_pCanvas, lSignalHandler );

  delete m_pPangoInk;
  delete m_pPangoLogical;
}

/// Blank the offscreen buffer

void CCanvas::Blank() const {

  //  std::cout << "In Blank()" << std::endl;

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor background = colours[0];

  gdk_colormap_alloc_color(colormap, &background, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &background);
  
  gdk_draw_rectangle (m_pOffscreenBuffer,		      
		      graphics_context,
                      TRUE,
                      0, 0,
		      m_iWidth, m_iHeight );

  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}


void CCanvas::Display() { 

  GdkRectangle update_rect;

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor background = colours[0];

  gdk_colormap_alloc_color(colormap, &background, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &background);

  // Draw the target areas for 'dwell' start mode if necessary

  // FIXME - we shouldn't need to know about this sort of thing here
  
  //   if (paused==true) {
  //     if (firstbox==true) {
  //       draw_mouseposbox(0);
  //     } else if (secondbox==true) {
  //       draw_mouseposbox(1);
  //     }
  //   }

  // Copy the offscreen buffer into the onscreen buffer
  
  gdk_draw_drawable( m_pOnscreenBuffer,
		     m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)],
		     m_pOffscreenBuffer,
		     0, 0, 0,0,
		     m_iWidth, m_iHeight );

  // Blank the offscreen buffer (?)
  
  gdk_draw_rectangle( m_pOffscreenBuffer,
		      graphics_context,
		      TRUE,
		      0, 0,
		      m_iWidth, m_iHeight );
  
  // Invalidate the full canvas to force it to be redrawn on-screen
  
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = m_iWidth;
  update_rect.height = m_iHeight;

  gdk_window_invalidate_rect(m_pCanvas->window,&update_rect,FALSE);

  // Restore original graphics context (?)

  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void CCanvas::DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const
{
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  GdkColor outline = colours[3];
  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor foreground = colours[Color];

  gdk_colormap_alloc_color(colormap, &foreground,FALSE,TRUE);
  gdk_colormap_alloc_color(colormap, &outline,FALSE,TRUE);
  gdk_gc_set_foreground (graphics_context, &foreground);

  // FIXME (below) - shouldn make 'drawoutline' a parameter passed to this function
  
  if( x2 > x1 ) {
    if( y2 > y1 ) {
      gdk_draw_rectangle (m_pOffscreenBuffer, graphics_context, TRUE, x1, y1, x2-x1, y2-y1);
//       if (drawoutline==TRUE) {
// 	gdk_gc_set_foreground (graphics_context, &outline);
// 	gdk_draw_rectangle (m_pOffscreenBuffer, graphics_context, FALSE, x1, y1, x2-x1, y2-y1);
//       }
    }
    else {
      gdk_draw_rectangle (m_pOffscreenBuffer, graphics_context, TRUE, x1, y2, x2-x1, y1-y2);
//       if (drawoutline==TRUE) {
// 	gdk_gc_set_foreground (graphics_context, &outline);
// 	gdk_draw_rectangle (m_pOffscreenBuffer, graphics_context, FALSE, x1, y2, x2-x1, y1-y2);
//       }
    }
  }
  else {
    if( y2 > y1 ) {
      gdk_draw_rectangle (m_pOffscreenBuffer, graphics_context, TRUE, x2, y1, x1-x2, y2-y1);
//       if (drawoutline==TRUE) {
// 	gdk_gc_set_foreground (graphics_context, &outline);
// 	gdk_draw_rectangle (m_pOffscreenBuffer, graphics_context, FALSE, x2, y1, x1-x2, y2-y1);
//       }
    }
    else {
      gdk_draw_rectangle (m_pOffscreenBuffer, graphics_context, TRUE, x2, y2, x1-x2, y1-y2);
//       if (drawoutline==TRUE) {
// 	gdk_gc_set_foreground (graphics_context, &outline);
// 	gdk_draw_rectangle (m_pOffscreenBuffer, graphics_context, FALSE, x2, y2, x1-x2, y1-y2);
//       }
    }
  }
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void CCanvas::Polyline(Dasher::CDasherScreen::point* Points, int Number) const {
  Polyline(Points, Number, 0);
}

void CCanvas::Polygon(Dasher::CDasherScreen::point* Points, int Number, int Colour) const
{
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  GdkColor colour = colours[Colour];
  GdkPoint *gdk_points;

  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));
  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)];
  gdk_gc_get_values(graphics_context,&origvalues);
  colormap = gdk_colormap_get_system();

  gdk_colormap_alloc_color(colormap, &colour, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &colour);

  for (int i=0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_polygon(m_pOffscreenBuffer, graphics_context, TRUE, gdk_points, Number);
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
  g_free(gdk_points);
}

void CCanvas::ExposeEvent( GdkEventExpose *pEvent ) {

    gdk_draw_drawable(m_pCanvas->window,
		      m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)],
		      m_pOnscreenBuffer,
		      pEvent->area.x, pEvent->area.y,
		      pEvent->area.x, pEvent->area.y,
		      pEvent->area.width, pEvent->area.height);

    // FIXME - stuff below should happen in the realize event

//   if (firsttime==TRUE) {
//     // canvas_expose_event() is the easiest function to catch
//     // if we want to know when everything is set up and displayed
//     paused=true;
//     firsttime=false;
//   }

}

void CCanvas::Polyline(Dasher::CDasherScreen::point* Points, int Number, int Colour) const
{ 
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  GdkColor colour = colours[Colour];
  GdkPoint *gdk_points;

  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));
  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)];
  gdk_gc_get_values(graphics_context,&origvalues);
  colormap = gdk_colormap_get_system();

  gdk_colormap_alloc_color(colormap, &colour, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &colour);

  for (int i=0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_lines(m_pOffscreenBuffer, graphics_context, gdk_points, Number);
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
  g_free(gdk_points);
}

void CCanvas::DrawString(symbol Character, int x1, int y1, int size) const
{

  // Urgh - we really don't need to know about symbols here - surely this is never called?

//   GdkGC *graphics_context;
//   GdkColormap *colormap;
//   GdkGCValues origvalues;
//   std::string symbol;

//   symbol = dasher_get_display_text( Character );

//   graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)];
//   colormap = gdk_colormap_get_system();

//   gdk_gc_get_values(graphics_context,&origvalues);

//   int colour=dasher_get_text_colour(Character);

//   GdkColor foreground = colours[colour];

//   gdk_colormap_alloc_color(colormap, &foreground, FALSE, TRUE);
//   gdk_gc_set_foreground (graphics_context, &foreground);
  
//   PangoLayout *pLayout( get_pango_layout( symbol, size ) );

//   pango_layout_get_pixel_extents(pLayout,ink,logical);

//   gdk_draw_layout (m_pOffscreenBuffer,
// 		   graphics_context,
// 		   x1, y1-ink->height/2, pLayout);

//   gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void CCanvas::DrawString(const std::string &String, int x1, int y1, int size) const
{
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;


  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context,&origvalues);

  GdkColor foreground = colours[4];

  gdk_colormap_alloc_color(colormap, &foreground, FALSE, TRUE);
  gdk_gc_set_foreground (graphics_context, &foreground);

  PangoLayout *pLayout( m_pPangoCache->GetLayout( GTK_WIDGET( m_pCanvas ), String, size ) );

  pango_layout_get_pixel_extents(pLayout,m_pPangoInk,m_pPangoLogical);

  gdk_draw_layout (m_pOffscreenBuffer,
		   graphics_context,
		   x1, y1-m_pPangoInk->height/2, pLayout);

  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
}

void CCanvas::TextSize(const std::string &String, int* Width, int* Height, int size) const
{

  // Get a pango layout from the cache.

  PangoLayout *pLayout( m_pPangoCache->GetLayout( GTK_WIDGET( m_pCanvas ), String, size ) );

  pango_layout_get_pixel_extents( pLayout,m_pPangoInk,m_pPangoLogical);

  // We don't actually use the logical, so can we NULLify this?

  *Width = m_pPangoInk->width;
  *Height= m_pPangoInk->height;
}

void CCanvas::SendMarker( int iMarker ) {

  //  std::cout << "Marker: " << iMarker << std::endl;

  switch( iMarker ) {
  case 0:
    // Starting a new frame, so clear the background buffer


    m_pOffscreenBuffer = m_pDisplayBuffer;

    break;
  case 1:

    gdk_draw_drawable( m_pDecorationBuffer,
		       m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)],
		       m_pDisplayBuffer,
		       0, 0, 0,0,
		       m_iWidth, m_iHeight );
    
    m_pOffscreenBuffer = m_pDecorationBuffer;

    break;
  }
}

// Fixme - reimplement all of this

// GdkColor get_color(int Color, Opts::ColorSchemes ColorScheme)
// {
//   if (ColorScheme == Special1) {
//     GdkColor foreground = { 0, 240*257, 240*257, 240*257 };
//     return foreground;
//   }
//   if (ColorScheme == Special2) {
//     GdkColor foreground = { 0, 255*257, 255*257, 255*257 };
//     return foreground;
//   }
//   if (ColorScheme == Objects) {
//     GdkColor foreground = { 0, 0, 0, 0 };
//     return foreground;
//   }
//   if (ColorScheme == Groups) {
//     if (Color%3 == 0) {
//       GdkColor foreground = { 0, 255*257, 255*257 ,0 };
//       return foreground;
//     }
//     if (Color%3 == 1) {
//       GdkColor foreground = { 0, 255*257 ,100*257, 100*257 };
//       return foreground;
//     }
//     if (Color %3 == 2) {
//       GdkColor foreground = { 0, 0, 255*257 ,0 };
//       return foreground;
//     }
//   }
//   if (ColorScheme == Nodes1) {
//     if (Color%3 == 0) {
//       GdkColor foreground = { 0, 180*257 ,245*257 ,180*257 };
//       return foreground;
//     }
//     if (Color%3 == 1) {
//       GdkColor foreground = { 0, 160*257 ,200*257 ,160*257 };
//       return foreground;
//     }
//     if (Color %3 == 2) {
//       GdkColor foreground = { 0, 0, 255*257, 255*257 };
//       return foreground;
//     }
//   }
//   if (ColorScheme == Nodes2) {
//     if (Color%3 == 0) {
//       GdkColor foreground = { 0, 255*257, 185*257, 255*257 };
//       return foreground;
//     }
//     if (Color%3 == 1) {
//       GdkColor foreground = { 0, 140*257, 200*257, 255*257 };
//       return foreground;
//     }
//     if (Color %3 == 2) {
//       GdkColor foreground = { 0, 255*257, 175*257, 175*257 };
//       return foreground;
//     }
//   }
//   GdkColor foreground = {0, 0, 0, 0 };
//   return foreground;
// }

// void set_canvas_font(std::string fontname) 
// {
//   if(fontname!="") {
//     pango_font_description_free(font);
//     font=pango_font_description_from_string(fontname.c_str());
//     dasher_redraw();
//   }
//   //  {
//   //  GdkColor foreground = { 0, 255*257, 185*257, 255*257 };
//   //  return foreground;
//   // }
// }

// void reset_dasher_font()
// {
//   pango_font_description_free(font);
//   font=pango_font_description_from_string("DASHERFONT");
//   dasher_redraw();
// }


void CCanvas::SetColourScheme(const Dasher::CCustomColours *Colours) {

  int iNumColours( Colours->GetNumColours() );

  colours = new GdkColor[ iNumColours ];

   for (int i=0; i<iNumColours; i++) {
     colours[i].pixel=0;
     colours[i].red=Colours->GetRed(i)*257;
     colours[i].green=Colours->GetGreen(i)*257;
     colours[i].blue=Colours->GetBlue(i)*257;
   }
 }

// void draw_mouseposbox(int which) {

//   if (setup==false||preferences==true)
//     return;

//   GdkGC *graphics_context;
//   GdkColormap *colormap;
//   GdkColor color;
//   GdkGCValues origvalues;
//   graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE (m_pCanvas)];
//   colormap = gdk_colormap_get_system();
//   int top=0;

//   switch (which) {
//   case 0:
//     color.pixel=0;
//     color.red=255*257;
//     color.green=0*257;
//     color.blue=0*257;
//     top=m_pCanvas->allocation.height/2-mouseposstartdist-100;
//     break;
//   case 1:
//     color.pixel=0;
//     color.red=255*257;
//     color.green=255*257;
//     color.blue=0*257;
//     top=m_pCanvas->allocation.height/2+mouseposstartdist;
//     break;
//   }

//   gdk_gc_get_values(graphics_context,&origvalues);
//   gdk_colormap_alloc_color(colormap, &color,FALSE, TRUE);
//   gdk_gc_set_foreground (graphics_context, &color);
//   gdk_draw_rectangle (m_pOffscreenBuffer, graphics_context, FALSE, 0, top, (m_pCanvas->allocation.width-1), 100);
//   gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND);
// }

extern "C" gint
canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  ((CCanvas *)data)->ExposeEvent( event );

  return TRUE;
}
