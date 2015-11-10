#include "../Common/Common.h"

#include "Canvas.h"
#include "DasherControl.h"

#include "../DasherCore/DasherTypes.h"

using namespace Dasher;

CCanvas::CCanvas(GtkWidget *pCanvas)
  : CLabelListScreen(0,0) {

#if WITH_CAIRO
  cairo_colours = 0;
#else
  colours = 0;
#endif
  
  m_pCanvas = pCanvas;

  gtk_widget_add_events(m_pCanvas, GDK_ALL_EVENTS_MASK);

  InitSurfaces(); //will create 0*0 surfaces. Is that a good idea? It seems to
  // let us create PangoLayouts ok, which is what we need - but if we can
  // create them without a backing surface at all, that might be better...
}

void CCanvas::InitSurfaces() {  
  // Construct the buffer pixmaps
  // FIXME - only allocate without cairo

#if WITH_CAIRO

  m_pDisplaySurface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, GetWidth(), GetHeight());
  m_pDecorationSurface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, GetWidth(), GetHeight());
  //m_pOnscreenSurface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, GetWidth(), GetHeight());

#else

  //m_pDummyBuffer = gdk_pixmap_new(m_pCanvas->window, GetWidth(), GetHeight(), -1);

  m_pDisplayBuffer = gdk_pixmap_new(m_pCanvas->window, GetWidth(), GetHeight(), -1);
  m_pDecorationBuffer = gdk_pixmap_new(m_pCanvas->window, GetWidth(), GetHeight(), -1);
  //m_pOnscreenBuffer = gdk_pixmap_new(m_pCanvas->window, GetWidth(), GetHeight(), -1);

  // Set the display buffer to be current

  m_pOffscreenBuffer = m_pDisplayBuffer;

#endif


#if WITH_CAIRO
  // The lines between origin and pointer is draw here
  decoration_cr = cairo_create(m_pDecorationSurface);
  cr = decoration_cr;
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_set_line_width(cr, 1.0);

  // Base stuff are drawn here
  display_cr = cairo_create(m_pDisplaySurface);
  cr = display_cr;
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_set_line_width(cr, 1.0);

  //onscreen_cr = cairo_create(m_pOnscreenSurface);

#endif
}

void CCanvas::DestroySurfaces() {
  // Free the buffer pixmaps

#if WITH_CAIRO
  cr = NULL;
  cairo_surface_destroy(m_pDisplaySurface);
  cairo_surface_destroy(m_pDecorationSurface);
  //cairo_surface_destroy(m_pOnscreenSurface);  
  cairo_destroy(display_cr);
  cairo_destroy(decoration_cr);
  //cairo_destroy(onscreen_cr);
#else
  //g_object_unref(m_pDummyBuffer);
  g_object_unref(m_pDisplayBuffer);
  g_object_unref(m_pDecorationBuffer);
  //g_object_unref(m_pOnscreenBuffer);
#endif
}

CCanvas::~CCanvas() {
  DestroySurfaces();
#if WITH_CAIRO
  delete[] cairo_colours;
#endif
}

void CCanvas::resize(screenint w,screenint h) {
  DestroySurfaces();
  CDasherScreen::resize(w,h);
  InitSurfaces();
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
  gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, TRUE, 0, 0, GetWidth(), GetHeight());
#endif

  END_DRAWING;
}

void CCanvas::Display() {
  // FIXME - Some of this stuff is probably not needed
  //  GdkRectangle update_rect;

#if WITH_CAIRO
#else
  GdkGC *graphics_context;
  GdkColormap *colormap;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;
  SET_COLOR(0);

  // Copy the offscreen buffer into the onscreen buffer

  // TODO: Reimplement (kind of important!)

  //  gdk_draw_drawable(m_pOnscreenBuffer, m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)], m_pOffscreenBuffer, 0, 0, 0, 0, GetWidth(), GetHeight());
  
  //  BEGIN_DRAWING;

//   cairo_set_source_surface(onscreen_cr, m_pDecorationSurface, 0, 0);
//   cairo_rectangle(onscreen_cr, 0, 0, GetWidth(), GetHeight());
//   cairo_fill(onscreen_cr);



  //  END_DRAWING;

  // Blank the offscreen buffer (?)

  //  gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, TRUE, 0, 0, GetWidth(), GetHeight());

  // Invalidate the full canvas to force it to be redrawn on-screen

 //  update_rect.x = 0;
//   update_rect.y = 0;
//   update_rect.width = GetWidth();
//   update_rect.height = GetHeight();

//   gdk_window_invalidate_rect(m_pCanvas->window, &update_rect, FALSE);

  //  BEGIN_DRAWING;

  //  GdkRectangle sRect = {0, 0, GetWidth(), GetHeight()};
  //  gdk_window_begin_paint_rect(m_pCanvas->window, &sRect);

#if WITH_CAIRO  
  cairo_t *widget_cr;
  widget_cr = gdk_cairo_create(gtk_widget_get_window(m_pCanvas));
  cairo_set_source_surface(widget_cr, m_pDecorationSurface, 0, 0);
  cairo_rectangle(widget_cr, 0, 0, GetWidth(), GetHeight());
  cairo_fill(widget_cr);
  cairo_destroy(widget_cr);
#else
  gdk_draw_drawable(m_pCanvas->window, m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)], m_pDecorationBuffer, 0, 0, 0, 0, GetWidth(), GetHeight());
#endif

  //   gdk_window_end_paint(m_pCanvas->window);

  // Restore original graphics context (colours)

  END_DRAWING;

  // gtk_main_iteration_do(0);
}

void CCanvas::DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, int iOutlineColour, int iThickness) {

  //  std::cout << "Raw Rectangle, (" << x1 << ", " << y1 << ") - (" << x2 << ", " << y2 << ")" << std::endl;

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

  //  std::cout << bFill << " " << Color << " " << iLeft << " " << iTop << " " << iWidth << " " << iHeight << std::endl;

  if(Color!=-1) {
    SET_COLOR(Color);
#if WITH_CAIRO
    //cairo_set_line_width(cr, iThickness); //outline done below
    cairo_rectangle(cr, iLeft, iTop, iWidth, iHeight);
    cairo_fill(cr);
#else
    gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, TRUE, iLeft, iTop, iWidth + 1, iHeight + 1);
#endif
  }

  if(iThickness>0) {
    if( iOutlineColour == -1 )
      SET_COLOR(3);
    else
      SET_COLOR(iOutlineColour);
    
    // TODO: There's a known issue here when the start of one box and
    // the end of the other map to the same pixel. This generally
    // results in box outlines being truncated.

#if WITH_CAIRO
    cairo_set_line_width(cr, iThickness);
    cairo_rectangle(cr, iLeft + 0.5, iTop + 0.5, iWidth, iHeight);
    cairo_stroke(cr);
#else
    gdk_gc_set_line_attributes(graphics_context, iThickness, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );
    gdk_draw_rectangle(m_pOffscreenBuffer, graphics_context, FALSE, iLeft, iTop, iWidth, iHeight);
#endif
  }
  END_DRAWING;
}

void CCanvas::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iFillColour, int iLineColour, int iThickness) {
#if WITH_CAIRO
#else
  GdkGC *graphics_context;
  GdkColormap *colormap;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;

  if(iFillColour!=-1) {
    SET_COLOR(iFillColour);
#if WITH_CAIRO
    cairo_arc(cr, iCX, iCY, iR, 0, 2*M_PI);
    cairo_fill(cr);
#else
    gdk_draw_arc(m_pOffscreenBuffer, graphics_context, true, iCX - iR, iCY - iR, 2*iR, 2*iR, 0, 23040);
#endif
  }

  if (iThickness>0) {
    SET_COLOR(iLineColour);
#if WITH_CAIRO
    cairo_set_line_width(cr, iThickness);
    cairo_arc(cr, iCX, iCY, iR, 0, 2*M_PI);
    cairo_stroke(cr);
#else
    //note fiddle on iThickness, allegedly "to make it work on Windows"(?)...
    gdk_gc_set_line_attributes(graphics_context, iThickness==1 ? 0 : iThickness, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );
    gdk_draw_arc(m_pOffscreenBuffer, graphics_context, false, iCX - iR, iCY - iR, 2*iR, 2*iR, 0, 23040);
#endif
  }

  END_DRAWING;
}

void CCanvas::Polygon(Dasher::CDasherScreen::point *Points, int Number, int fillColour, int outlineColour, int iWidth) {

  //(ACL) commenting out, we now deal with fill & outline separately. However,
  // TODO: find a windows box on which this actually applies and test it
  //if(iWidth == 1) // This is to make it work properly on Windows
  //  iWidth = 0; 

#if WITH_CAIRO
#else
  GdkGC *graphics_context;
  GdkColormap *colormap;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;

#if WITH_CAIRO
 cairo_move_to(cr, Points[0].x, Points[0].y);
  for (int i=1; i < Number; i++)
    cairo_line_to(cr, Points[i].x, Points[i].y);
  cairo_close_path(cr);
  if (fillColour!=-1) {
    SET_COLOR(fillColour);
    if (iWidth<=0) {
      cairo_fill(cr); //fill only, no outline
    } else {
      cairo_fill_preserve(cr); //leave path defined for cairo_stroke, below
    }
  }
  if (iWidth>0) {
    SET_COLOR(outlineColour==-1 ? 3 : outlineColour);
    cairo_stroke(cr);
  }
#else
  GdkPoint *gdk_points;
  gdk_points = (GdkPoint *) g_malloc(Number * sizeof(GdkPoint));

  for(int i = 0; i < Number; i++) {
    gdk_points[i].x = Points[i].x;
    gdk_points[i].y = Points[i].y;
  }

  if (fillColour != -1) {
    SET_COLOR(fillColour);
    gdk_gc_set_line_attributes(graphics_context, 1, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );
    gdk_draw_polygon(m_pOffscreenBuffer, graphics_context, TRUE, gdk_points, Number);
  }
  if (iWidth > 0) {
    SET_COLOR(outlineColour);
    gdk_gc_set_line_attributes(graphics_context, iWidth, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND );
    gdk_draw_polygon(m_pOffscreenBuffer, graphics_context, FALSE, gdk_points, Number);
  }
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

CDasherScreen::Label *CCanvas::MakeLabel(const string &strText, unsigned int iWrapFontSize) {
  return new CPangoLabel(this, strText, iWrapFontSize);
}

void CCanvas::SetFont(const std::string &strName) {
  m_strFontName=strName;
  for (map<unsigned int,PangoFontDescription *>::iterator it=m_mFonts.begin(); it!=m_mFonts.end(); it++) {
    pango_font_description_free(it->second);
    it->second = pango_font_description_from_string(m_strFontName.c_str());
    pango_font_description_set_size(it->second,it->first * PANGO_SCALE);
  }
  for (set<CLabelListScreen::Label *>::iterator it=LabelsBegin(); it!=LabelsEnd(); it++) {
    map<unsigned int,PangoLayout *> &layouts(static_cast<CPangoLabel *>(*it)->m_mLayouts);
    for (map<unsigned int,PangoLayout *>::iterator it2=layouts.begin(); it2!=layouts.end(); it2++) {
      DASHER_ASSERT(m_mFonts.find(it2->first) != m_mFonts.end()); //central font repository knows about this size
      pango_layout_set_font_description(it2->second,m_mFonts[it2->first]);
    }
  }
}

PangoLayout *CCanvas::GetLayout(CPangoLabel *label, unsigned int iFontSize) {
  {
    map<unsigned int,PangoLayout *>::iterator it = label->m_mLayouts.find(iFontSize);
    if (it != label->m_mLayouts.end()) return it->second;
  }
#if WITH_CAIRO
    PangoLayout *pNewPangoLayout(pango_cairo_create_layout(cr));
#else
    PangoLayout *pNewPangoLayout(gtk_widget_create_pango_layout(m_pCanvas, ""));
#endif
  label->m_mLayouts.insert(pair<unsigned int,PangoLayout *>(iFontSize, pNewPangoLayout));
  if (label->m_iWrapSize) pango_layout_set_width(pNewPangoLayout, GetWidth() * PANGO_SCALE);
  pango_layout_set_text(pNewPangoLayout, label->m_strText.c_str(), -1);
  
  PangoFontDescription *pF;
  {
    map<unsigned int,PangoFontDescription *>::iterator it = m_mFonts.find(iFontSize);
    if (it != m_mFonts.end())
      pF=it->second;
    else {
      pF = pango_font_description_from_string(m_strFontName.c_str());
      pango_font_description_set_size(pF, iFontSize * PANGO_SCALE);
      m_mFonts[iFontSize] = pF;
    }
    pango_layout_set_font_description(pNewPangoLayout, pF);
  }
    return pNewPangoLayout;
}

void CCanvas::DrawString(CDasherScreen::Label *label, screenint x1, screenint y1, unsigned int size, int iColor) {
  
#if WITH_CAIRO
#else
  GdkGC *graphics_context;
  GdkColormap *colormap;

  graphics_context = m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)];
  colormap = gdk_colormap_get_system();
#endif

  BEGIN_DRAWING;
  SET_COLOR(iColor);

  PangoLayout *pLayout(GetLayout(static_cast<CPangoLabel*>(label),size));

  PangoRectangle sPangoInk;

  pango_layout_get_pixel_extents(pLayout, &sPangoInk, NULL);
  x1 -= sPangoInk.x;
  y1 -= sPangoInk.y;
#if WITH_CAIRO
  cairo_translate(cr, x1, y1);
  pango_cairo_show_layout(cr, pLayout);
#else
  gdk_draw_layout(m_pOffscreenBuffer, graphics_context, x1, y1, pLayout);
#endif

  END_DRAWING;
}

pair<screenint,screenint> CCanvas::TextSize(CDasherScreen::Label *label, unsigned int size) {
  PangoLayout *pLayout(GetLayout(static_cast<CPangoLabel*>(label),size));

  PangoRectangle sPangoInk;
  pango_layout_get_pixel_extents(pLayout, &sPangoInk, NULL);

  return pair<screenint,screenint>(sPangoInk.width,sPangoInk.height);
}

void CCanvas::SendMarker(int iMarker) {

  switch(iMarker) {
  case 0: // Switch to display buffer
#if WITH_CAIRO
    cr = display_cr;
#else
    m_pOffscreenBuffer = m_pDisplayBuffer;
#endif
    break;
  case 1: // Switch to decorations buffer

#if WITH_CAIRO
    cairo_set_source_surface(decoration_cr, m_pDisplaySurface, 0, 0);
    cairo_rectangle(decoration_cr, 0, 0, GetWidth(), GetHeight());
    cairo_fill(decoration_cr);
    cr = decoration_cr;
#else
    gdk_draw_drawable(m_pDecorationBuffer, m_pCanvas->style->fg_gc[GTK_WIDGET_STATE(m_pCanvas)], m_pDisplayBuffer, 0, 0, 0, 0, GetWidth(), GetHeight());
    m_pOffscreenBuffer = m_pDecorationBuffer;
#endif
    break;
  }
}

void CCanvas::SetColourScheme(const CColourIO::ColourInfo *pColourScheme) {
  int iNumColours(pColourScheme->Reds.size());

#if WITH_CAIRO
  if (cairo_colours)
    delete[] cairo_colours;
  cairo_colours = new cairo_pattern_t*[iNumColours];
#else
  if (colours)
    delete[] colours;
  colours = new GdkColor[iNumColours];
#endif

  for(int i = 0; i < iNumColours; i++) {
#if WITH_CAIRO
    cairo_colours[i] = cairo_pattern_create_rgb (
      pColourScheme->Reds[i]   / 255.0,
      pColourScheme->Greens[i] / 255.0,
      pColourScheme->Blues[i]  / 255.0
    );
#else
    colours[i].pixel=0;
    colours[i].red=pColourScheme->Reds[i]*257;
    colours[i].green=pColourScheme->Greens[i]*257;
    colours[i].blue=pColourScheme->Blues[i]*257;
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

  gdk_window_get_position(gtk_widget_get_window(m_pCanvas), &iX, &iY);

  pRectangle->x       = iX;
  pRectangle->y       = iY;
  pRectangle->width   = GetWidth();
  pRectangle->height  = GetHeight();

  return true;
}
