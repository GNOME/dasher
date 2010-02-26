#ifndef __canvas_h__
#define __canvas_h__

#include <cstdlib>

#include "../DasherCore/DasherScreen.h"
#include "../DasherCore/DasherTypes.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "PangoCache.h"

#include <iostream>

#if WITH_CAIRO

/* Cairo drawing backend */
#include <gdk/gdkcairo.h>

#define BEGIN_DRAWING_BACKEND				\
  cairo_save(cr)

#define END_DRAWING_BACKEND				\
  cairo_restore(cr)

#define SET_COLOR_BACKEND(c)				\
  cairo_set_source(cr, cairo_colours[(c)])

#else /* WITHOUT_CAIRO */

#define BEGIN_DRAWING_BACKEND				\
  GdkGCValues origvalues;				\
  gdk_gc_get_values(graphics_context,&origvalues)

#define END_DRAWING_BACKEND				\
  gdk_gc_set_values(graphics_context,&origvalues,GDK_GC_FOREGROUND)

#define SET_COLOR_BACKEND(c)				\
  do {							\
    GdkColor _c = colours[(c)];				\
    gdk_colormap_alloc_color(colormap, &_c, FALSE, TRUE);	\
    gdk_gc_set_foreground (graphics_context, &_c);	\
  } while (0)

#endif /* WITH_CAIRO */

// Some other useful macros (for all backends)

#define BEGIN_DRAWING					\
  BEGIN_DRAWING_BACKEND

#define END_DRAWING					\
  END_DRAWING_BACKEND

#define SET_COLOR(c)					\
  SET_COLOR_BACKEND(c)

using namespace Dasher;

/// CCanvas
///
/// Method definitions for CCanvas, implementing the CDasherScreen
/// interface.  Please think very carefully before implementing new
/// functionality in this class. Anything which isn't a 'drawing
/// primitive' should really not be here - higher level drawing
/// functions belong in CDasherView.

class CCanvas:public Dasher::CDasherScreen {

public:

  /// 
  /// \param pCanvas The GTK drawing area used by the canvas
  /// \param pPangoCache A cache for precomputed Pango layouts
  ///

  CCanvas(GtkWidget * pCanvas, CPangoCache * pPangoCache);
  ~CCanvas();

  ///
  /// GTK signal handler for exposure of the canvas - cause a redraw to the screen from the buffer.
  ///

  bool ExposeEvent( GtkWidget *pWidget, GdkEventExpose *pEvent);


  // CDasherScreen methods

  ///
  /// Set the font used to render the Dasher display
  /// \param Name The name of the font.
  /// \todo This needs to be reimplemented for 4.0
  /// \deprecated In Linux - now handled by the pango cache, but need to think how this fits in with Windows
  ///  

  void SetFont(std::string Name) {
  };

  ///
  /// Set the font size for rendering
  /// \param fontsize The font size to use
  /// \deprecated Obsolete
  ///

  void SetFontSize(Dasher::Opts::FontSize fontsize) {

  };

  ///
  /// Get the current font size
  /// \deprecated To be removed before 4.0 release
  /// \todo We should not be relying on locally cached variables - check to see whether this is still used or not
  ///

  Dasher::Opts::FontSize GetFontSize() {
    return Dasher::Opts::FontSize(1);
  };

  ///
  /// Return the physical extent of a given string being rendered at a given size.
  /// \param String The string to be rendered
  /// \param Width Pointer to a variable to be filled with the width
  /// \param Height Pointer to a variable to be filled with the height
  /// \param Size Size at which the string will be rendered (units?)
  ///

  void TextSize(const std::string &String, screenint *Width, screenint *Height, int Size);

  ///
  /// Draw a text string
  /// \param String The string to be rendered
  /// \param x1 The x coordinate at which to draw the text (be more precise)
  /// \param y1 The y coordinate at which to draw the text (be more precise)
  /// \param Size The size at which to render the rectangle (units?)
  ///

  void DrawString(const std::string &String, screenint x1, screenint y1, int Size);

  ///
  /// Draw a rectangle
  /// \param x1 x coordiate of the top left corner
  /// \param y1 y coordiate of the top left corner
  /// \param x2 x coordiate of the bottom right corner
  /// \param y2 y coordiate of the bottom right corner
  /// \param Color Colour to draw the rectangle
  /// \param ColorScheme Which of the alternating colour schemes to use (be more precise)
  /// \param bDrawOutline Whether or not to draw outlines for the boxes
  ///

  void DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, int iOutlineColour, Opts::ColorSchemes ColorScheme, bool bDrawOutine, bool bFill, int iThickness);

  void DrawCircle(screenint iCX, screenint iCY, screenint iR, int iColour, int iFillColour, int iThickness, bool bFill);

  ///
  /// Send a marker to indicate phases of the redraw process. This is
  /// done so that we can do tripple buffering to minimise the amount
  /// of costly font rendering which needs to be done. Marker 1
  /// indicates that start of a new frame. Marker 2 indicates that we
  /// are now drawing decoration (such as mouse cursors etc.) rather
  /// than tne background. Only marker 2 will be send while Dasher is
  /// paused.
  /// \param iMarker ID of the marker being sent.
  ///

  void SendMarker(int iMarker);

  /// 
  /// Draw a coloured polyline
  /// \param Points Array of vertices
  /// \param Number Size of 'Points' array
  /// \param Colour Colour with which to draw the line
  ///

  void Polyline(point * Points, int Number, int iWidth, int Colour);

  /// 
  /// Like polyline, but fill the shape
  /// \todo See comments for DrawPolygon
  ///

  void Polygon(point *Points, int Number, int Colour, int iWidth);

  ///
  /// \todo Not implemented
  /// \todo One of these two routines must be redundant - find out which and kill the other
  ///

  void DrawPolygon(point *Points, int Number, int Color, Opts::ColorSchemes ColorScheme) {
    // not implemented 
  };

  /// 
  /// Blank the diplay
  ///

  void Blank();

  /// 
  /// Marks the end of the display process - at this point the offscreen buffer is copied onscreen.
  ///

  void Display();

  ///
  /// Update the colour definitions
  /// \param Colours New colours to use
  ///

  void SetColourScheme(const CColourIO::ColourInfo *pColourScheme);

  /// 
  /// Gets the location and size of our canvas.
  /// Returns true on success, false otherwise.
  bool GetCanvasSize(GdkRectangle *pRectangle);

  void SetLoadBackground(bool bValue) {
    // Not required in this model
  };

  void SetCaptureBackground(bool bValue) {
    // Not required in this model
  };

  /// 
  /// Canvas width
  ///

  int m_iWidth;

  ///
  /// Canvas height
  ///

  int m_iHeight;

private:

  ///
  /// The GTK drawing area for the canvas
  ///

  GtkWidget *m_pCanvas;

#if WITH_CAIRO

  cairo_surface_t *m_pDisplaySurface;
  cairo_surface_t *m_pDecorationSurface;
  //cairo_surface_t *m_pOnscreenSurface;

  cairo_surface_t *m_pOffscreenbuffer;

#else

  ///
  /// The offscreen buffer containing the 'background'
  ///

  GdkPixmap *m_pDisplayBuffer;

  /// 
  /// The offscreen buffer containing the full display. This is
  /// constructed by first copying the display buffer across and then
  /// drawing decorations such as the mouse cursor on top.
  ///

  GdkPixmap *m_pDecorationBuffer;

  ///
  /// The onscreen buffer - copied onscreen whenever an expose event occurs.
  ///

  //GdkPixmap *m_pOnscreenBuffer;

  ///
  /// Pointer to which of the offscreen buffers is currently active.
  ///

  GdkPixmap *m_pOffscreenBuffer;
  //GdkPixmap *m_pDummyBuffer;

#endif

  /// 
  /// The Pango cache - used to store pre-computed pango layouts as
  /// they are costly to regenerate every time they are needed.
  ///

  CPangoCache *m_pPangoCache;

  ///
  /// Holder for Pango layout extents.
  ///

  PangoRectangle *m_pPangoInk;

#if WITH_CAIRO
  cairo_t *display_cr;
  cairo_t *decoration_cr;

  cairo_t *cr; // offscreen
  cairo_pattern_t **cairo_colours;
#else
  GdkColor *colours;
#endif  

};

#endif
