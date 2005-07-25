#include "Canvas.h"
#include "DasherControl.h"

#include "../DasherCore/DasherTypes.h"

using namespace Dasher;

extern "C" gint canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);

CCanvas::CCanvas(GtkWidget *pCanvas, CPangoCache *pPangoCache)
  : CDasherScreen(pCanvas->allocation.width, pCanvas->allocation.height) {
  
  m_pCanvas = pCanvas;
  m_pPangoCache = pPangoCache;
  
  m_iWidth = m_pCanvas->allocation.width;
  m_iHeight = m_pCanvas->allocation.height;
  
  // Construct the buffer pixmaps

  m_pDisplayBuffer = gdk_pixmap_new(pCanvas->window, m_iWidth, m_iHeight, -1);
  m_pDecorationBuffer = gdk_pixmap_new(pCanvas->window, m_iWidth, m_iHeight, -1);
  m_pOnscreenBuffer = gdk_pixmap_new(pCanvas->window, m_iWidth, m_iHeight, -1);

  // Set the display buffer to be current

  m_pOffscreenBuffer = m_pDisplayBuffer;

  m_pPangoInk = new PangoRectangle;

  lSignalHandler = g_signal_connect(m_pCanvas, "expose_event", G_CALLBACK(canvas_expose_event), this);

  gtk_widget_add_events(m_pCanvas, GDK_EXPOSURE_MASK);
  gtk_widget_add_events(m_pCanvas, GDK_BUTTON_PRESS_MASK);
  gtk_widget_add_events(m_pCanvas, GDK_BUTTON_RELEASE_MASK);
}

CCanvas::~CCanvas() {
  // Free the buffer pixmaps

  g_free(m_pDisplayBuffer);
  g_free(m_pDecorationBuffer);
  g_free(m_pOnscreenBuffer);

  g_signal_handler_disconnect(m_pCanvas, lSignalHandler);

  delete m_pPangoInk;
}

void CCanvas::Blank() {

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context, &origvalues);

  GdkColor background = colours[0];

  gdk_colormap_alloc_color(colormap, &background, FALSE, TRUE);
  gdk_gc_set_foreground(graphics_context, &background);

  gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, TRUE, 0, 0, m_iWidth, m_iHeight);

  gdk_gc_set_values(graphics_context, &origvalues, GDK_GC_FOREGROUND);
}

void CCanvas::Display() {

  // FIXME - Some of this stuff is probably not needed

  GdkRectangle update_rect;

  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context, &origvalues);

  GdkColor background = colours[0];

  gdk_colormap_alloc_color(colormap, &background, FALSE, TRUE);
  gdk_gc_set_foreground(graphics_context, &background);

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

  gdk_gc_set_values(graphics_context, &origvalues, GDK_GC_FOREGROUND);
}

void CCanvas::DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme, bool bDrawOutline) {
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  GdkColor outline = colours[3];
  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context, &origvalues);

  GdkColor foreground = colours[Color];

  gdk_colormap_alloc_color(colormap, &foreground, FALSE, TRUE);
  gdk_colormap_alloc_color(colormap, &outline, FALSE, TRUE);

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
  
  gdk_gc_set_foreground(graphics_context, &foreground);
  gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, TRUE, iLeft, iTop, iWidth, iHeight);
  
  if(bDrawOutline) {
    gdk_gc_set_foreground(graphics_context, &outline);
    gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, FALSE, iLeft, iTop, iWidth, iHeight);
  }

  gdk_gc_set_values(graphics_context, &origvalues, GDK_GC_FOREGROUND);
}

void CCanvas::Polygon(Dasher::CDasherScreen::point *Points, int Number, int Colour, int iWidth) {
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  GdkColor colour = colours[Colour];
  GdkPoint *gdk_points;

  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));
  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  gdk_gc_get_values(graphics_context, &origvalues);
  colormap = gdk_colormap_get_system();

  gdk_colormap_alloc_color(colormap, &colour, FALSE, TRUE);
  gdk_gc_set_foreground(graphics_context, &colour);

  gdk_gc_set_line_attributes(graphics_context, iWidth, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );
 
  for(int i = 0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_polygon(m_pOffscreenBuffer, graphics_context, TRUE, gdk_points, Number);
  gdk_gc_set_values(graphics_context, &origvalues, GDK_GC_FOREGROUND);
  gdk_gc_set_values(graphics_context, &origvalues, GDK_GC_LINE_WIDTH);
  g_free(gdk_points);
}

void CCanvas::Polyline(Dasher::CDasherScreen::point *Points, int Number, int iWidth, int Colour) {
  GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  GdkColor colour = colours[Colour];
  GdkPoint *gdk_points;

  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));
  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  gdk_gc_get_values(graphics_context, &origvalues);
  colormap = gdk_colormap_get_system();

  gdk_colormap_alloc_color(colormap, &colour, FALSE, TRUE);
  gdk_gc_set_foreground(graphics_context, &colour);

  gdk_gc_set_line_attributes(graphics_context, iWidth, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );

  for(int i = 0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  gdk_draw_lines(m_pOffscreenBuffer, graphics_context, gdk_points, Number);
  gdk_gc_set_values(graphics_context, &origvalues, GDK_GC_FOREGROUND);
  gdk_gc_set_values(graphics_context, &origvalues, GDK_GC_LINE_WIDTH);
  g_free(gdk_points);
}

void CCanvas::DrawString(const std::string &String, int x1, int y1, int size) {

   GdkGC *graphics_context;
  GdkColormap *colormap;
  GdkGCValues origvalues;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();

  gdk_gc_get_values(graphics_context, &origvalues);

  GdkColor foreground = colours[4];

  gdk_colormap_alloc_color(colormap, &foreground, FALSE, TRUE);
  gdk_gc_set_foreground(graphics_context, &foreground);

  PangoLayout *pLayout(m_pPangoCache->GetLayout(GTK_WIDGET(m_pCanvas), String, size));
  pango_layout_get_pixel_extents(pLayout, m_pPangoInk, NULL);
  
  gdk_draw_layout(m_pOffscreenBuffer, graphics_context, x1, y1 - m_pPangoInk->height / 2, pLayout);

  gdk_gc_set_values(graphics_context, &origvalues, GDK_GC_FOREGROUND);
}

void CCanvas::TextSize(const std::string &String, int *Width, int *Height, int size) {
  PangoLayout *pLayout(m_pPangoCache->GetLayout(GTK_WIDGET(m_pCanvas), String, size));

  pango_layout_get_pixel_extents(pLayout, m_pPangoInk, NULL);

  *Width = m_pPangoInk->width;
  *Height = m_pPangoInk->height;
}

void CCanvas::SendMarker(int iMarker) {

  switch(iMarker) {
  case 0: // Switch to display buffer
    m_pOffscreenBuffer = m_pDisplayBuffer;
    break;
  case 1: // Switch to decorations buffer
    gdk_draw_drawable(m_pDecorationBuffer, m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)], m_pDisplayBuffer, 0, 0, 0, 0, m_iWidth, m_iHeight);
    m_pOffscreenBuffer = m_pDecorationBuffer;
    break;
  }
}

bool CCanvas::ExposeEvent(GtkWidget *pWidget, GdkEventExpose *pEvent) {
  gdk_draw_drawable(m_pCanvas->window, m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)], m_pOnscreenBuffer, pEvent->area.x, pEvent->area.y, pEvent->area.x, pEvent->area.y, pEvent->area.width, pEvent->area.height);
  return true;
}

void CCanvas::SetColourScheme(const Dasher::CCustomColours *Colours) {
  int iNumColours(Colours->GetNumColours());

  colours = new GdkColor[iNumColours];

  for(int i = 0; i < iNumColours; i++) {
    colours[i].pixel = 0;
    colours[i].red = Colours->GetRed(i) * 257;
    colours[i].green = Colours->GetGreen(i) * 257;
    colours[i].blue = Colours->GetBlue(i) * 257;
  }
}

extern "C" gint canvas_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  return ((CCanvas*)data)->ExposeEvent(widget, event);
}
