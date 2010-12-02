#ifndef __canvas_h__
#define __canvas_h__

#include "../DasherCore/DasherScreen.h"
#include "../DasherCore/DasherTypes.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "PangoCache.h"

#include <iostream>

//#undef WITH_CAIRO

#if WITH_CAIRO

/* Cairo drawing backend */
#include <gdk/gdkcairo.h>

typedef struct {
  double r, g, b;
} my_cairo_colour_t;

#else /* WITHOUT_CAIRO */

#endif /* WITH_CAIRO */

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

  CCanvas(GtkWidget * pCanvas, CPangoCache * pPangoCache, int width, int height);
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

  void DrawString(const std::string &String, screenint x1, screenint y1, int Size, int Col);

  ///
  /// Draw a rectangle
  /// \param x1 x coordiate of the top left corner
  /// \param y1 y coordiate of the top left corner
  /// \param x2 x coordiate of the bottom right corner
  /// \param y2 y coordiate of the bottom right corner
  /// \param iColour Colour to fill rectangle, -1 = don't
  /// \param iOutlineColour Colour to draw outline; -1 = use default
  /// \param iThickness Line width in which to draw outline; -1 = don't
  ///
  void DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, int iOutlineColour, int iThickness);

  void DrawCircle(screenint iCX, screenint iCY, screenint iR, int iFillColour, int iLineColour, int iThickness);

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
  /// Draw a closed polygon (linking last vertex back to first)
  /// @param fillColour colour to fill; -1 => don't fill
  /// @param outlineColour colour to draw outline...
  /// @param iWidth ...and line thickness; -1 => don't draw outline
  ///

  void Polygon(point *Points, int Number, int fillColour, int outlineColour, int iWidth);

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

  /// 
  /// Canvas width
  ///

  int m_iWidth;

  ///
  /// Canvas height
  ///

  int m_iHeight;
  void ChangeState(int value);
private:

     void CirclePoints(int cx, int cy, int x, int y, int Colour,int iWidth);
     void CircleMidpoint(int xCenter, int yCenter, int radius, int Colour,int iWidth);
     void CircleFill(int xCenter,int yCenter,int radius,int Colour);
     void HVThinLine(int x0,int y0,int x1,int y1,int Colour);
    /** IGNAS**/ 
  ///
  /// Frame procedure
  ///
#ifdef FRAMERATE_DIAGNOSTICS
      void NewFrame();
      int count_frames;//=0;
      time_t lasttime;//=-1;
      time_t count_time;//=0;
      long total_frames;//=0;
      int count_rectangles;
#endif

      guchar *display_data ;
      guchar *display_backgrounddata;
      GdkPixbuf * display_pixbuf;
//      GdkPixmap * display_pixmap;
      guchar *display_fontdata;
      
      int display_depth;
      int display_fontdepth;
      int display_fontwidth;
      int display_fontheight;
/**IGNAS**/
  
  void Line(int x0,int y0,int x1,int y1,int iWidth, int Colour);
  void SetPixel(GdkColor *c,int x,int y,int iWidth=1);  
  bool HorizontalIntersectionPoint(int h,int a,int b,int x0,int y0,int x1,int y1, int *p);
///
  /// The GTK drawing area for the canvas
  ///

  bool m_bCaptureBackground;
  
  GtkWidget *m_pCanvas;
  gint *point_id;
  gint *point_amount;
  gint *point_data;
#if WITH_CAIRO

	cairo_t *display_fontcairo;
        cairo_surface_t *display_fontcairosurface;

#else
        GdkPixmap* display_fontgdk;

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
  my_cairo_colour_t *cairo_colours;
#else
  GdkColor *colours;
#endif  

};

#endif
