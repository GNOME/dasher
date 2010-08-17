// DasherView.h
//
// Copyright (c) 2001-2005 David Ward

#ifndef __DasherView_h_
#define __DasherView_h_

namespace Dasher {
  class CDasherInput; // Why does DasherView care about input? - pconlon
  class CDasherComponent;
  class CDasherView;
  class CDasherNode;
}

#include "DasherTypes.h"
#include "DasherComponent.h"
#include "ExpansionPolicy.h"
#include "DasherScreen.h"

/// \defgroup View Visualisation of the model
/// @{

/// \brief View base class.
///
/// Dasher views render the tree of Dasher nodes onto a screen.
///
/// Note that we really should aim to avoid having to try and keep
/// multiple pointers to the same object (model etc.) up-to-date at
/// once. We should be able to avoid the need for this just by being
/// sane about passing pointers as arguments to the relevant
/// functions, for example we could pass a pointer to the canvas every
/// time we call the render routine, rather than worrying about
/// notifying this object every time it changes. The same logic can be
/// applied in several other places.
///
/// There are really three roles played by CDasherView: providing high
/// level drawing functions, providing a mapping between Dasher
/// co-ordinates and screen co-ordinates and providing a mapping
/// between true and effective Dasher co-ordinates (eg for eyetracking
/// mode). We should probably consider creating separate classes for
/// these.

class Dasher::CDasherView : public Dasher::CDasherComponent
{
public:

  /// Constructor
  /// 
  /// \param pEventHandler Pointer to the event handler
  /// \param pSettingsStore Pointer to the settings store
  /// \param DasherScreen Pointer to the CDasherScreen object used to do rendering
  CDasherView(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherScreen * DasherScreen);

  virtual ~CDasherView() {
  }

  /// @name Pointing device mappings 
  /// @{

  /// Set the input device class. Note that this class will now assume ownership of the pointer, ie it will delete the object when it's done with it.
  /// \param _pInput Pointer to the new CDasherInput.
  void SetInput(CDasherInput * _pInput);
  void SetDemoMode(bool);
  void SetGameMode(bool);
  /// Translates the screen coordinates to Dasher coordinates
  virtual int GetCoordinates(myint &iDasherX, myint &iDasherY);

  
  /// Get the co-ordinate count from the input device
  
  int GetCoordinateCount();


  /// @}

  /// 
  /// @name Coordinate system conversion
  /// Convert between screen and Dasher coordinates
  /// @{

  /// 
  /// Convert a screen co-ordinate to Dasher co-ordinates
  ///

  virtual void Screen2Dasher(screenint iInputX, screenint iInputY, myint & iDasherX, myint & iDasherY) = 0;

  ///
  /// Convert Dasher co-ordinates to screen co-ordinates
  ///

  virtual void Dasher2Screen(myint iDasherX, myint iDasherY, screenint & iScreenX, screenint & iScreenY) = 0;

  ///
  /// Convert Dasher co-ordinates to polar co-ordinates (r,theta), with 0<r<1, 0<theta<2*pi
  ///
  virtual void Dasher2Polar(myint iDasherX, myint iDasherY, double &r, double &theta) = 0;

  virtual bool IsSpaceAroundNode(myint y1, myint y2)=0;

  virtual void VisibleRegion( myint &iDasherMinX, myint &iDasherMinY, myint &iDasherMaxX, myint &iDasherMaxY ) = 0;

  /// @}

  /// Change the screen - must be called if the Screen is replaced or resized
  /// \param NewScreen Pointer to the new CDasherScreen.

  virtual void ChangeScreen(CDasherScreen * NewScreen);

  /// @name High level drawing
  /// Drawing more complex structures, generally implemented by derived class
  /// @{

  /// Top-level/public render function - render all the nodes.
  /// @param pRoot outermost node to render. should cover screen if possible;
  /// function will blank out around it (in white) if not
  /// @return the innermost node covering the crosshair
  virtual CDasherNode *Render(CDasherNode *pRoot, myint iRootMin, myint iRootMax, CExpansionPolicy &policy)=0;

  /// @}

  ////// Return a reference to the screen - can't be protected due to circlestarthandler
  
  CDasherScreen *Screen() {
    return m_pScreen;
  }

  ///
  /// @name Low level drawing
  /// Basic drawing primitives specified in Dasher coordinates.
  /// @{

  ///Draw a straight line in Dasher-space - which may be curved on the screen...
  void DasherSpaceLine(myint x1, myint y1, myint x2, myint y2, int iWidth, int iColour);
  
  ///
  /// Draw a polyline specified in Dasher co-ordinates
  ///

  void DasherPolyline(myint * x, myint * y, int n, int iWidth, int iColour);

  /// Draw a polyarrow

  void DasherPolyarrow(myint * x, myint * y, int n, int iWidth, int iColour, double dArrowSizeFactor = 0.7071);

  ///
  /// Draw a rectangle specified in Dasher co-ordinates
  /// Color of -1 => no fill; any other value => fill in that color
  /// iOutlineColor of -1 => no outline; any other value => outline in that color, EXCEPT
  /// Thickness < 1 => no outline.
  ///
  void DasherDrawRectangle(myint iLeft, myint iTop, myint iRight, myint iBottom, const int Color, int iOutlineColour, Opts::ColorSchemes ColorScheme, int iThickness);

  ///
  /// Draw a centred rectangle specified in Dasher co-ordinates (used for mouse cursor)
  ///

  void DasherDrawCentredRectangle(myint iDasherX, myint iDasherY, screenint iSize, const int Color, Opts::ColorSchemes ColorScheme, bool bDrawOutline);

  void DrawText(const std::string & str, myint x, myint y, int Size, int iColor);

  /// @}

protected:
  /// Clips a line (specified in Dasher co-ordinates) to the visible region
  /// by intersecting with all boundaries.
  /// \return true if any part of the line was within the visible region; in this case, (x1,y1)-(x2,y2) delineate exactly that part
  /// false if the line would be entirely outside the visible region; x1, y1, x2, y2 undefined.
  bool ClipLineToVisible(myint &x1, myint &y1, myint &x2, myint &y2); 
  
  ///Convert a straight line in Dasher-space, to coordinates for a corresponding polyline on the screen
  /// (because of nonlinearity, this may require multiple line segments)
  /// \param x1,y1 Dasher co-ordinates of start of line segment; note that these are guaranteed within VisibleRegion.
  /// \param x2,y2 Dasher co-ordinates of end of line segment; also guaranteed within VisibleRegion.
  /// \param vPoints vector to which to add screen points. Note that at the point that DasherLine2Screen is called,
  /// the screen coordinates of the first point should already have been added to this vector; DasherLine2Screen
  /// will then add exactly one CDasherScreen::point for each line segment required.
  virtual void DasherLine2Screen(myint x1, myint y1, myint x2, myint y2, std::vector<CDasherScreen::point> &vPoints)=0;
  
  // Orientation of Dasher Screen
/*   inline void MapScreen(screenint * DrawX, screenint * DrawY); */
/*   inline void UnMapScreen(screenint * DrawX, screenint * DrawY); */
  bool m_bVisibleRegionValid;
  
  ///Number of nodes actually rendered. Updated only by subclasses; TODO does
  /// this belong here? (perhaps for subclass-agnostic clients to inspect...)
  int m_iRenderCount;

private:
  CDasherScreen *m_pScreen;    // provides the graphics (text, lines, rectangles):
  CDasherInput *m_pInput;       // Input device abstraction

  /// Get the co-ordinates from the input device
  int GetInputCoordinates(int iN, myint * pCoordinates); 

  bool m_bDemoMode;
  bool m_bGameMode;
};
/// @}

#endif /* #ifndef __DasherView_h_ */
