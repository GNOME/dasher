// DasherView.h
//
// Copyright (c) 2001-2005 David Ward

#ifndef __DasherView_h_
#define __DasherView_h_

namespace Dasher {
  class CDasherView;
  class CDasherNode;
}

#include "DasherTypes.h"
#include "ExpansionPolicy.h"
#include "DasherScreen.h"
#include "Observable.h"
#include "Event.h"

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

///Also an Observable: CDasherView* events should be generated whenever the screen
/// geometry changes: e.g. aspect ratio, size, degree of nonlinearity,
/// orientation, or generally whenever values returned by Dasher2Screen/Screen2Dasher
/// might have changed (thus, any code caching such values should recompute/invalidate them).
/// The "event" is just a pointer to the View itself, but can also be used
/// to send round a pointer to a new view (i.e. replacing this one).
/// CGameNodeDrawEvents are broadcast whenever a node with NF_GAME set is rendered (or has
/// its y-coordinate range computed); is using an Observable worth it, or should we just
/// call directly to the game module?
class Dasher::CDasherView : public Observable<CDasherView *>, public Observable<CGameNodeDrawEvent*> {
public:

  /// Constructor
  /// 
  /// \param DasherScreen Pointer to the CDasherScreen object used to do rendering
  CDasherView(CDasherScreen * DasherScreen, Opts::ScreenOrientations orient);

  virtual ~CDasherView() {
  }

  virtual void SetOrientation(Opts::ScreenOrientations newOrient) {m_Orientation=newOrient;}
  Opts::ScreenOrientations GetOrientation() {return m_Orientation;}

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

  /// Change the screen - must be called if the Screen is replaced (not resized).
  /// Default implementation just stores pointer. Note that a call to ChangeScreen
  /// is usually followed by a call to ScreenResized as well, so stuff that only
  /// depends on screen size/resolution can be done there instead.
  /// \param NewScreen Pointer to the new CDasherScreen.
  virtual void ChangeScreen(CDasherScreen * NewScreen);
  
  ///Call when the screen dimensions have been changed, to recalculate scaling factors etc.
  /// The default implementation does nothing.
  /// \param pScreen the screen whose dimensions have changed. TODO we expect this to be
  /// the same one-and-only screen that we are using anyway, so remove parameter?
  virtual void ScreenResized(CDasherScreen *pScreen) {}

  void TransferObserversTo(CDasherView *pNewView) {
    Observable<CDasherView*>::DispatchEvent(pNewView);
  }
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
  
  virtual void DasherSpaceArc(myint cy, myint r, myint x1, myint y1, myint x2, myint y2, int iColour, int iLineWidth)=0;

  ///
  /// Draw a polyline specified in Dasher co-ordinates
  ///

  void DasherPolyline(myint * x, myint * y, int n, int iWidth, int iColour);

  /// Draw a polyarrow
  /// The parameters x and y allow the client to specify points in Dasher space
  /// through which the arrow's main line should be drawn. For example, to draw an
  /// arrow through the Dasher coordinates (1000, 2000) and (3000, 4000), one would pass in:
  ///
  /// myint x[2] = {1000, 3000};
  /// myint y[2] = {2000, 4000};
  ///
  /// \param x - an array of x coordinates to draw the arrow through
  /// \param y - an array of y coordinates to draw the arrow through
  /// \param iWidth - the width to make the arrow lines - typically of the form
  ///        GetLongParameter(LP_LINE_WIDTH)*CONSTANT
  /// \param iColour line colour, as per Polyline (-1 => use "default" 0)
  /// \param dArrowSizeFactor - the factor by which to scale the "hat" on the arrow
  ///
  void DasherPolyarrow(myint * x, myint * y, int n, int iWidth, int iColour, double dArrowSizeFactor = 0.7071);

  ///
  /// Draw a rectangle specified in Dasher co-ordinates
  /// \param Color color in which to fill, -1 => no fill
  /// \param iOutlineColor colour in which to draw outline, -1 => use default
  /// \param iThickness line width for outline, < 1 => no outline.
  ///
  void DasherDrawRectangle(myint iDasherMaxX, myint iDasherMinY, myint iDasherMinX, myint iDasherMaxY, const int Color, int iOutlineColour, int iThickness);

  ///
  /// Draw a centred rectangle specified in Dasher co-ordinates (used for mouse cursor)
  /// \param Color fill color for rectangle (-1 => don't fill)
  /// \param bDrawOutline if true, rectangle will be outlined with width 1 and default line colour (-1 => 3)
  ///

  void DasherDrawCentredRectangle(myint iDasherX, myint iDasherY, screenint iSize, const int Color, bool bDrawOutline);

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
  
  ///Number of nodes actually rendered. Updated only by subclasses; TODO does
  /// this belong here? (perhaps for subclass-agnostic clients to inspect...)
  int m_iRenderCount;

private:
  Opts::ScreenOrientations m_Orientation;
  CDasherScreen *m_pScreen;    // provides the graphics (text, lines, rectangles):
};
/// @}

#endif /* #ifndef __DasherView_h_ */
