#include "../Common/Common.h"

#include "Canvas.h"
#include "DasherControl.h"

#include "../DasherCore/DasherTypes.h"

using namespace Dasher;

extern "C" gint canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);

CCanvas::CCanvas(GtkWidget *pCanvas, CPangoCache *pPangoCache)
  : CDasherScreen(pCanvas->allocation.width, pCanvas->allocation.height) {

#if WITH_CAIRO
  cairo_colours = 0;
#else
  colours = 0;
#endif
  
  m_pCanvas = pCanvas;
  m_pPangoCache = pPangoCache;
  
  m_iWidth = m_pCanvas->allocation.width;
  m_iHeight = m_pCanvas->allocation.height;

  // Construct the buffer pixmaps
  // FIXME - only allocate without cairo

  m_pDummyBuffer = gdk_pixmap_new(pCanvas->window, m_iWidth, m_iHeight, -1);

  m_pDisplayBuffer = gdk_pixmap_new(pCanvas->window, m_iWidth, m_iHeight, -1);
  m_pDecorationBuffer = gdk_pixmap_new(pCanvas->window, m_iWidth, m_iHeight, -1);
  m_pOnscreenBuffer = gdk_pixmap_new(pCanvas->window, m_iWidth, m_iHeight, -1);

  // Set the display buffer to be current

  m_pOffscreenBuffer = m_pDisplayBuffer;


#if WITH_CAIRO
  // The lines between origin and pointer is draw here
  decoration_cr = gdk_cairo_create(m_pDecorationBuffer);
  cr = decoration_cr;
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_set_line_width(cr, 1.0);

  // Base stuff are drawn here
  display_cr = gdk_cairo_create(m_pDisplayBuffer);
  cr = display_cr;
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_set_line_width(cr, 1.0);
#endif

  m_pPangoInk = new PangoRectangle;

  lSignalHandler = g_signal_connect(m_pCanvas, "expose_event", G_CALLBACK(canvas_expose_event), this);

/*  gtk_widget_add_events(m_pCanvas, GDK_EXPOSURE_MASK);
  gtk_widget_add_events(m_pCanvas, GDK_BUTTON_PRESS_MASK);
  gtk_widget_add_events(m_pCanvas, GDK_BUTTON_RELEASE_MASK);
*/
  gtk_widget_add_events(m_pCanvas, GDK_ALL_EVENTS_MASK);
}

CCanvas::~CCanvas() {
  // Free the buffer pixmaps

#if WITH_CAIRO
  cr = NULL;
  cairo_destroy(display_cr);
  cairo_destroy(decoration_cr);
#endif

  g_object_unref(m_pDummyBuffer);
  g_object_unref(m_pDisplayBuffer);
  g_object_unref(m_pDecorationBuffer);
  g_object_unref(m_pOnscreenBuffer);

  g_signal_handler_disconnect(m_pCanvas, lSignalHandler);

  delete m_pPangoInk;
}

void CCanvas::Blank() {
  // FIXME - this is replicated throughout this file - do something
  // about that
#if WITH_CAIRO
#else
  GdkGC *graphics_context;
  GdkColormap *colormap;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;
  SET_COLOR(0);

#if WITH_CAIRO
  cairo_paint(cr);
#else
  gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, TRUE, 0, 0, m_iWidth, m_iHeight);
#endif

  END_DRAWING;
}

void CCanvas::Display() {

  // FIXME - Some of this stuff is probably not needed

  GdkRectangle update_rect;

  GdkGC *graphics_context;
#if WITH_CAIRO
#else
  GdkColormap *colormap;
#endif

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];

#if WITH_CAIRO
#else
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;
  SET_COLOR(0);

  // Copy the offscreen buffer into the onscreen buffer

  gdk_draw_drawable(m_pOnscreenBuffer, m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)], m_pOffscreenBuffer, 0, 0, 0, 0, m_iWidth, m_iHeight);
  
  // Blank the offscreen buffer (?)

  gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, TRUE, 0, 0, m_iWidth, m_iHeight);

  // Invalidate the full canvas to force it to be redrawn on-screen

  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = m_iWidth;
  update_rect.height = m_iHeight;

  gdk_window_invalidate_rect(m_pCanvas->window, &update_rect, FALSE);

  // Restore original graphics context (?)

  END_DRAWING;

  gtk_main_iteration_do(0);
}

void CCanvas::DrawRectangle(int x1, int y1, int x2, int y2, int Color, int iOutlineColour, Opts::ColorSchemes ColorScheme, bool bDrawOutline, bool bFill, int iThickness) {
#if WITH_CAIRO
#else
  GdkGC *graphics_context;
  GdkColormap *colormap;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;

  int iLeft;
  int iTop;
  int iWidth;
  int iHeight;

  if( x2 > x1 ) {
    iLeft = x1;
    iWidth = x2 - x1;
  }
  else {
    iLeft = x2;
    iWidth = x1 - x2;
  }

  if( y2 > y1 ) {
    iTop = y1;
    iHeight = y2 - y1;
  }
  else {
    iTop = y2;
    iHeight = y1 - y2;
  }

  if(bFill) {
    SET_COLOR(Color);
#if WITH_CAIRO
    cairo_set_line_width(cr, iThickness);
    cairo_rectangle(cr, x1, y1, x2-x1+1.0, y2-y1+1.0);
    cairo_fill(cr);
#else
    gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, TRUE, iLeft, iTop, iWidth, iHeight);
#endif
  }
  
  if(bDrawOutline) {
    if( iOutlineColour == -1 )
      SET_COLOR(3);
    else
      SET_COLOR(iOutlineColour);
    
#if WITH_CAIRO
    cairo_set_line_width(cr, iThickness);
    cairo_rectangle(cr, x1+.5, y1+.5, x2-x1, y2-y1);
    cairo_stroke(cr);
#else
    gdk_gc_set_line_attributes(graphics_context, iThickness, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );
    gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, FALSE, iLeft, iTop, iWidth, iHeight);
#endif
  }
  END_DRAWING;
}

void CCanvas::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iColour, bool bFill) {
#if WITH_CAIRO
#else
  GdkGC *graphics_context;
  GdkColormap *colormap;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;
  SET_COLOR(iColour);

#if WITH_CAIRO
  cairo_arc(cr, iCX, iCY, iR, 0, 2*M_PI);
#else
  // FIXME
#endif

  END_DRAWING;
}

void CCanvas::Polygon(Dasher::CDasherScreen::point *Points, int Number, int Colour, int iWidth) {

  if(iWidth == 1) // This is to make it work propely on Windows
    iWidth = 0; 

#if WITH_CAIRO
#else
  GdkGC *graphics_context;
  GdkColormap *colormap;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;
  SET_COLOR(Colour);

#if WITH_CAIRO
 cairo_move_to(cr, Points[0].x, Points[0].y);
  for (int i=1; i < Number; i++)
    cairo_line_to(cr, Points[i].x, Points[i].y);
  cairo_close_path(cr);
  cairo_fill(cr);
#else
  GdkPoint *gdk_points;
  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));

  for(int i = 0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_gc_set_line_attributes(graphics_context, iWidth, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );
  gdk_draw_polygon(m_pOffscreenBuffer, graphics_context, TRUE, gdk_points, Number);
  g_free(gdk_points);
#endif

  END_DRAWING;
}

void CCanvas::Polyline(Dasher::CDasherScreen::point *Points, int Number, int iWidth, int Colour) {

  // FIXME - combine this with polygon?

#if WITH_CAIRO
#else
  if(iWidth == 1) // This is to make it work propely on Windows
    iWidth = 0; 

  GdkGC *graphics_context;
  GdkColormap *colormap;
  
  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;
  SET_COLOR(Colour);

#if WITH_CAIRO
  cairo_set_line_width(cr, iWidth);
  cairo_move_to(cr, Points[0].x+.5, Points[0].y+.5);
  for (int i=1; i < Number; i++)
    cairo_line_to(cr, Points[i].x+.5, Points[i].y+.5);
  cairo_stroke(cr);
#else
  GdkPoint *gdk_points;
  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));

  gdk_gc_set_line_attributes(graphics_context, iWidth, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );

  for(int i = 0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_lines(m_pOffscreenBuffer, graphics_context, gdk_points, Number);
  g_free(gdk_points);
#endif 

  END_DRAWING;
}

void CCanvas::DrawString(const std::string &String, int x1, int y1, int size) {

#if WITH_CAIRO
#else
  GdkGC *graphics_context;
  GdkColormap *colormap;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;
  SET_COLOR(4);

#if WITH_CAIRO
  PangoLayout *pLayout(m_pPangoCache->GetLayout(cr, String, size));
#else
  PangoLayout *pLayout(m_pPangoCache->GetLayout(GTK_WIDGET(m_pCanvas), String, size));
#endif

  pango_layout_get_pixel_extents(pLayout, m_pPangoInk, NULL);

#if WITH_CAIRO
  cairo_translate(cr, x1, y1-(int)m_pPangoInk->height/2);
  pango_cairo_show_layout(cr, pLayout);
#else
  gdk_draw_layout(m_pOffscreenBuffer, graphics_context, x1, y1 - m_pPangoInk->height / 2, pLayout);
#endif

  END_DRAWING;
}

void CCanvas::TextSize(const std::string &String, int *Width, int *Height, int size) {

#if WITH_CAIRO
  PangoLayout *pLayout(m_pPangoCache->GetLayout(cr, String, size));
#else
  PangoLayout *pLayout(m_pPangoCache->GetLayout(GTK_WIDGET(m_pCanvas), String, size));
#endif
  pango_layout_get_pixel_extents(pLayout, m_pPangoInk, NULL);

  *Width = m_pPangoInk->width;
  *Height = m_pPangoInk->height;
}

void CCanvas::SendMarker(int iMarker) {

  switch(iMarker) {
  case 0: // Switch to display buffer
    m_pOffscreenBuffer = m_pDisplayBuffer;
#if WITH_CAIRO
    cr = display_cr;
#endif
    break;
  case 1: // Switch to decorations buffer
    gdk_draw_drawable(m_pDecorationBuffer, m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)], m_pDisplayBuffer, 0, 0, 0, 0, m_iWidth, m_iHeight);
    m_pOffscreenBuffer = m_pDecorationBuffer;
#if WITH_CAIRO
    cr = decoration_cr;
#endif
    break;
  }
}

bool CCanvas::ExposeEvent(GtkWidget *pWidget, GdkEventExpose *pEvent) {
  gdk_draw_drawable(m_pCanvas->window, m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)], m_pOnscreenBuffer, pEvent->area.x, pEvent->area.y, pEvent->area.x, pEvent->area.y, pEvent->area.width, pEvent->area.height);
  return true;
}

void CCanvas::SetColourScheme(const Dasher::CCustomColours *Colours) {
  int iNumColours(Colours->GetNumColours());

#if WITH_CAIRO
  if (cairo_colours)
    delete[] cairo_colours;
  cairo_colours = new my_cairo_colour_t[iNumColours];
#else
  if (colours)
    delete[] colours;
  colours = new GdkColor[iNumColours];
#endif

  for(int i = 0; i < iNumColours; i++) {
#if WITH_CAIRO
    cairo_colours[i].r = Colours->GetRed(i) / 255.0;
    cairo_colours[i].g = Colours->GetGreen(i) / 255.0;
    cairo_colours[i].b = Colours->GetBlue(i) / 255.0;
#else
    colours[i].pixel=0;
    colours[i].red=Colours->GetRed(i)*257;
    colours[i].green=Colours->GetGreen(i)*257;
    colours[i].blue=Colours->GetBlue(i)*257;
#endif
  }
}

bool CCanvas::GetCanvasSize(GdkRectangle *pRectangle)
{
  if ((pRectangle == NULL) || (m_pCanvas == NULL))
    return false;
 
  // Using gtk_window_get_frame_extents() only seems to return the position
  // and size of the parent Dasher window.  So we'll get the widgets position
  // and use its size to determine the bounding rectangle.
  int iX = 0;
  int iY = 0;

  gdk_window_get_position(m_pCanvas->window, &iX, &iY);

  pRectangle->x       = iX;
  pRectangle->y       = iY;
  pRectangle->width   = m_iWidth;
  pRectangle->height  = m_iHeight;

  return true;
}

extern "C" gint canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  return ((CCanvas*)data)->ExposeEvent(widget, event);
}

