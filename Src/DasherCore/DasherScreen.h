// DasherScreen.h
//
// Copyright (c) 2001-2004 David Ward

#ifndef __DasherScreen_h_
#define __DasherScreen_h_

#include "DasherTypes.h"
#include "../DasherCore/ColourIO.h"

// DJW20050505 - renamed DrawText to DrawString - windows defines DrawText as a macro and it's 
// really hard to work around
// Also make CDasher screen operate in UTF8 strings only

namespace Dasher {
  class CDasherScreen;
  class CDasherInterfaceBase;
}

/// \ingroup View
/// @{
class Dasher::CDasherScreen
{
public:
  //! \param width Width of the screen
  //! \param height Height of the screen
  CDasherScreen(screenint width, screenint height)
  :m_iWidth(width), m_iHeight(height) {
  } 

  virtual ~ CDasherScreen() {
  }

/*   //! Set the widget interface used for communication with the core */
/*   virtual void SetInterface(CDasherInterfaceBase * DasherInterface) { */
/*     m_pDasherInterface = DasherInterface; */
/*   } */

  //! Return the width of the screen
  screenint GetWidth() {
    return m_iWidth;
  }
  //! Return the height of the screen screen
  int GetHeight() {
    return m_iHeight;
  }
  //! Structure defining a point on the screen 
  typedef struct tagpoint {
    screenint x;
    screenint y;
  } point;

  // DasherView asks for the width and height of the given UTF8 string at a requested height,
  // then it is able to sensibly specify the upper left corner in DrawString.
  //! Set Width and Height to those of the string at size Size
  virtual void TextSize(const std::string & String, screenint * Width, screenint * Height, int Size) = 0;

  //! Draw UTF8-encoded string String of size Size positioned at x1 and y1
  virtual void DrawString(const std::string & String, screenint x1, screenint y1, int Size) = 0;

  // Send a marker to indicate 'phases' of drawing. 

  virtual void SendMarker(int /*iMarker*/ ) {
  };

  /// Draw a filled rectangle
  ///
  /// Draw a coloured rectangle on the screen
  /// \param x1 top left of rectangle (x coordinate)
  /// \param y1 top left corner of rectangle (y coordinate)
  /// \param x2 bottom right of rectangle (x coordinate)
  /// \param y2 bottom right of rectangle (y coordinate)
  /// \param Color the colour to be used (numeric)
  /// \param iOutlineColour The colour for the node outlines
  /// \param ColorScheme Which colourscheme is to be used
  /// \param bDrawOutline Whether to draw an outline or not
  /// \param bFill Whether to fill or not
  /// \param iThickness Line thickness for outline

  virtual void DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, int iOutlineColour, Opts::ColorSchemes ColorScheme, bool bDrawOutline, bool bFill, int iThickness) = 0;

  virtual void DrawCircle(screenint iCX, screenint iCY, screenint iR, int iColour, int iFillColour, int iThickness, bool bFill) = 0;

  /// Draw a line of fixed colour (usually black). Intended for static UI elements such as a cross-hair
  /// Draw a line between each of the points in the array
  ///
  /// \param Points an array of points
  /// \param Number the number of points in the array
  /// \param iWidth The line width
  /// \todo This is dumb - why does this need to be a separate function to the coloured version?

  virtual void Polyline(point * Points, int Number, int iWidth) {
    Polyline(Points, Number, iWidth, 0);
  };

  // Draw a line of arbitrary colour.
  //! Draw a line between each of the points in the array
  //!
  //! \param Points an array of points
  //! \param Number the number of points in the array
  //! \param iWidth Width of the line
  //! \param Colour the colour to be drawn

  virtual void Polyline(point * Points, int Number, int iWidth, int Colour) = 0;

  // Draw a filled polygon - given vertices and color id
  // This is not (currently) used in standard Dasher. However, it could be very
  // useful in the future. Please implement unless it will be very difficult,
  // in which case make this function call Polyline.
  //! Draw a filled polygon
  //!
  //! \param Points array of points defining the edge of the polygon
  //! \param Number number of points in the array
  //! \param Color colour of the polygon (numeric)
  virtual void Polygon(point * Points, int Number, int Color) {
    Polygon(Points, Number, Color, 1);
  };

  virtual void Polygon(point * Points, int Number, int Color, int iWidth) = 0;

  // Signal the screen when a frame is started and finished
  //! Signal that a frame is being started
  virtual void Blank() = 0;

  //! Signal that a frame is finished - the screen should be updated
  virtual void Display() = 0;

  /// Set a colour scheme
  ///
  /// \param pColourScheme A colour scheme that should be used
  virtual void SetColourScheme(const Dasher::CColourIO::ColourInfo *pColourScheme) = 0;
  
  virtual void SetLoadBackground (bool value)=0;
  virtual void SetCaptureBackground (bool value)=0;
  
protected:
  //! Width and height of the screen
  const screenint m_iWidth, m_iHeight;

  //! Pointer to a widget interface for communication with the core
  //  CDasherInterfaceBase *m_pDasherInterface;
};
/// @}

#endif /* #ifndef __DasherScreen_h_ */
