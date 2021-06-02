#ifndef __canvas_h__
#define __canvas_h__

#include <cstdlib>

#include "../DasherCore/DasherScreen.h"
#include "../DasherCore/DasherTypes.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <pango/pango.h>
#include <map>

#include <iostream>

#define BEGIN_DRAWING_BACKEND				\
  cairo_save(cr)

#define END_DRAWING_BACKEND				\
  cairo_restore(cr)

#define SET_COLOR_BACKEND(c)				\
  cairo_set_source(cr, cairo_colours[(c)])

// Some other useful macros (for all backends)

#define BEGIN_DRAWING					\
  BEGIN_DRAWING_BACKEND

#define END_DRAWING					\
  END_DRAWING_BACKEND

#define SET_COLOR(c)					\
  SET_COLOR_BACKEND(c)

/// CCanvas
///
/// Method definitions for CCanvas, implementing the CDasherScreen
/// interface.  Please think very carefully before implementing new
/// functionality in this class. Anything which isn't a 'drawing
/// primitive' should really not be here - higher level drawing
/// functions belong in CDasherView.

class CCanvas:public Dasher::CLabelListScreen {

public:
  typedef Dasher::screenint screenint;
  /// Creates a new canvas - initially of zero size, so drawing
  /// operations won't do anything until a call to resize() is made.
  /// \param pCanvas The GTK drawing area used by the canvas
  ///

  CCanvas(GtkWidget *pCanvas);
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

  void SetFont(const std::string &strName);

  ///Make a label for use with this screen; caches Pango layout information inside it.
  CDasherScreen::Label *MakeLabel(const std::string &strText, unsigned int iWrapSize=0) override;

  ///
  /// Return the physical extent of a given string being rendered at a given size.
  /// \param String The string to be rendered
  /// \param Width Pointer to a variable to be filled with the width
  /// \param Height Pointer to a variable to be filled with the height
  /// \param Size Size at which the string will be rendered (units?)
  ///

  std::pair<screenint,screenint> TextSize(CDasherScreen::Label *label, unsigned int Size) override;

  ///
  /// Draw a text string
  /// \param String The string to be rendered
  /// \param x1 The x coordinate at which to draw the text (be more precise)
  /// \param y1 The y coordinate at which to draw the text (be more precise)
  /// \param Size The size at which to render the rectangle (units?)
  ///

  void DrawString(CDasherScreen::Label *label, screenint x1, screenint y1, unsigned int Size, int iColor) override;

  ///
  /// Draw a rectangle
  /// \param x1 x coordiate of the top left corner
  /// \param y1 y coordiate of the top left corner
  /// \param x2 x coordiate of the bottom right corner
  /// \param y2 y coordiate of the bottom right corner
  /// \param Color Colour to fill the rectangle (-1 = don't fill)
  /// \param iOutlineColour Colour to draw the outline (-1 = use default)
  /// \param iThickness line width of outline (<=0 = don't outline)
  ///
  void DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, int iOutlineColour, int iThickness) override;

  void DrawCircle(screenint iCX, screenint iCY, screenint iR, int iFillColour, int iLineColour, int iThickness) override;

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

  void SendMarker(int iMarker) override;

  /// 
  /// Draw a coloured polyline
  /// \param Points Array of vertices
  /// \param Number Size of 'Points' array
  /// \param Colour Colour with which to draw the line
  ///

  void Polyline(point * Points, int Number, int iWidth, int Colour) override;

  /// 
  /// Draw a closed polygon (linking last vertex back to first)
  /// @param fillColour colour to fill; -1 => don't fill
  /// @param outlineColour colour to draw outline...
  /// @param iWidth ...and line thickness; -1 => don't draw outline
  ///

  void Polygon(point *Points, int Number, int fillColour, int outlineColour, int iWidth) override;

  /// 
  /// Marks the end of the display process - at this point the offscreen buffer is copied onscreen.
  ///

  void Display() override;

  ///
  /// Update the colour definitions
  /// \param Colours New colours to use
  ///

  void SetColourScheme(const Dasher::CColourIO::ColourInfo *pColourScheme) override;

  /// 
  /// Gets the location and size of our canvas.
  /// Returns true on success, false otherwise.
  bool GetCanvasSize(GdkRectangle *pRectangle);

  // Redeclare to make public and adjust cairo/gdk surface sizes
  void resize(screenint w,screenint h);
  // Returns true if cursor is over visible part of this window.
  bool IsWindowUnderCursor() override;

private:

  ///
  /// The GTK drawing area for the canvas
  ///

  GtkWidget *m_pCanvas;

  void InitSurfaces();
  void DestroySurfaces();

  /// The offscreen buffer containing the 'background'
  cairo_surface_t *m_pDisplaySurface;

  /// The offscreen buffer containing the full display. This is
  /// constructed by first copying the display buffer across and then
  /// drawing decorations such as the mouse cursor on top.
  cairo_surface_t *m_pDecorationSurface;

  /// The onscreen buffer - copied onscreen whenever an expose event occurs.
  //cairo_surface_t *m_pOnscreenSurface;

  /// Pointer to which of the offscreen buffers is currently active.
  cairo_surface_t *m_pOffscreenbuffer;

  std::string m_strFontName;
  std::map<unsigned int,PangoFontDescription *> m_mFonts;

  class CPangoLabel : public CLabelListScreen::Label {
  public:
    CPangoLabel(CCanvas *pCanvas, const std::string &strText, unsigned int iWrapFontSize)
    : CLabelListScreen::Label(pCanvas, strText, iWrapFontSize) {
    }
    std::map<unsigned int,PangoLayout *> m_mLayouts;
  };

  PangoLayout *GetLayout(CPangoLabel *label, unsigned int iFontSize);

  cairo_t *display_cr;
  cairo_t *decoration_cr;

  cairo_t *cr; // offscreen
  cairo_pattern_t **cairo_colours;
};

#endif
