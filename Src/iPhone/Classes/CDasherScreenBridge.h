#import "../DasherCore/DasherScreen.h"
#import "../DasherCore/DasherTypes.h"

#import <iostream>
#import "DasherScreenCallbacks.h"

using namespace Dasher;

/// CDasherScreenBridge
///
/// Method definitions for CDasherScreenBridge, implementing the CDasherScreen
/// interface by bridging the methods onto Objective-C methods.

class CDasherScreenBridge:public Dasher::CDasherScreen {
  
public:
  
  /// 
  /// \param pCanvas The GTK drawing area used by the canvas
  /// \param pPangoCache A cache for precomputed Pango layouts
  ///
  
  CDasherScreenBridge(id<DasherScreenCallbacks> dv);
  ~CDasherScreenBridge();
  
  bool GetTouchCoords(screenint &iX, screenint &iY);
  
  // CDasherScreen methods
    
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
  
  void DrawString(const std::string &String, screenint x1, screenint y1, int Size, int iColour);
  
  ///
  /// Draw a rectangle
  /// \param x1 x coordiate of the top left corner
  /// \param y1 y coordiate of the top left corner
  /// \param x2 x coordiate of the bottom right corner
  /// \param y2 y coordiate of the bottom right corner
  /// \param Color Colour to fill the rectangle (-1 = don't fill)
  /// \param iOutlineColour Colour to draw box outline (-1 = use default)
  /// \param iThickness Line width in which to draw box outline (<=0 => don't outline)
  ///
  void DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, int iOutlineColour, int iThickness);
  
  void DrawCircle(screenint iCX, screenint iCY, screenint iR, int iFillColour, int iLineColour, int iLineWidth);
  
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
  ///
  
  void Polygon(point *Points, int Number, int fillColour, int outlineColor, int iWidth);
    
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

private:
  id <DasherScreenCallbacks> dasherView;   // objc counterpart
};
