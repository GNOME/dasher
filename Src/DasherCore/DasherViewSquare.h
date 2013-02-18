// DasherViewSquare.h
//
// Copyright (c) 2001-2004 David Ward

#ifndef __DasherViewSquare_h__
#define __DasherViewSquare_h__
#include "DasherView.h"
#include "DasherScreen.h"
#include <deque>
#include "Alphabet/GroupInfo.h"
#include "SettingsStore.h"

using namespace std;

namespace Dasher {
  class CDasherViewSquare;
  class CDasherView;
  class CDasherModel;
  class CDasherNode;
}

/// \ingroup View
/// @{

/// An implementation of the DasherView class
///
/// This class renders Dasher in the vanilla style,
/// but with horizontal and vertical mappings
///
/// Horizontal mapping - linear and log
/// Vertical mapping - linear with different gradient
class Dasher::CDasherViewSquare : public Dasher::CDasherView, public CSettingsUserObserver
{
public:

  /// Constructor
  ///
  /// \param DasherScreen Pointer to screen to which the view will render.
  /// \todo Don't cache screen and model locally - screen can be
  /// passed as parameter to the drawing functions, and data structure
  /// can be extracted from the model and passed too.

  CDasherViewSquare(CSettingsUser *pCreateFrom, CDasherScreen *DasherScreen, Opts::ScreenOrientations orient);
  ~CDasherViewSquare();

  ///
  /// Event handler
  ///

  virtual void HandleEvent(int iParameter);

  //Override to additionally reset scale factors etc.
  void SetOrientation(Opts::ScreenOrientations newOrient);

  /// Resets scale factors etc. that depend on the screen size, to be recomputed when next needed.
  void ScreenResized(CDasherScreen * NewScreen);

  ///
  /// @name Coordinate system conversion
  /// Convert between screen and Dasher coordinates
  /// @{

  ///
  /// Convert a screen co-ordinate to Dasher co-ordinates
  ///
  void Screen2Dasher(screenint iInputX, screenint iInputY, myint & iDasherX, myint & iDasherY);

  ///
  /// Convert Dasher co-ordinates to screen co-ordinates
  ///
  void Dasher2Screen(myint iDasherX, myint iDasherY, screenint & iScreenX, screenint & iScreenY);

  ///
  /// Convert Dasher co-ordinates to polar co-ordinates (r,theta), with 0<r<1, 0<theta<2*pi
  ///
  virtual void Dasher2Polar(myint iDasherX, myint iDasherY, double &r, double &theta);

  ///
  /// Return true if there is any space around a node spanning y1 to y2
  /// and the screen boundary; return false if such a node entirely encloses
  /// the screen boundary
  ///
  bool IsSpaceAroundNode(myint y1, myint y2);

  ///
  /// Get the bounding box of the visible region.
  ///
  void VisibleRegion( myint &iDasherMinX, myint &iDasherMinY, myint &iDasherMaxX, myint &iDasherMaxY );

  ///
  /// Render all nodes, inc. blanking around the root (supplied)
  ///
  virtual CDasherNode *Render(CDasherNode *pRoot, myint iRootMin, myint iRootMax, CExpansionPolicy &policy);

  /// @}

  void DasherSpaceArc(myint cy, myint r, myint x1, myint y1, myint x2, myint y2, int colour, int iLineWidth);
  
private:
  ///draw a possibly-truncated triangle given dasher-space coords & accounting for non-linearity
  /// @param x = max dasher-x extent
  /// @param y1, y2 = dasher-y extent along y-axis
  /// @param midy1,midy2 = extent along line of max x (midy1==midy2 => triangle, midy1<midy2 => truncated tri)
  void TruncateTri(myint x, myint y1, myint y2, myint midy1, myint midy2, int fillColor, int outlineColor, int lineWidth);

  /// compute screen coords for a circle, centered on y-axis, between two points
  /// cy, r - dasher coords of center (on y-axis), radius
  /// x1,y1 - one end-point of arc (dasher coords)
  /// x2,y2 - other end-point (dasher-coords)
  /// dest - point (x2,y2) in screen coords
  /// pts - vector into which to store points; on entry, last element should already be screen-coords of (x1,y1)
  /// dXMul - multiply x coords (in dasher space) by this (i.e. aspect ratio), for ovals
  void CircleTo(myint cy, myint r, myint y1, myint x1, myint y3, myint x3, CDasherScreen::point dest, vector<CDasherScreen::point> &pts, double dXMul);
  void Circle(myint Range, myint lowY, myint highY, int fCol, int oCol, int lWidth);
  void Quadric(myint Range, myint lowY, myint highY, int fillColor, int outlineColour, int lineWidth);
  ///draw isoceles triangle, with baseline from y1-y2 along y axis (x=0), and other point at (x,(y1+y2)/2)
  /// (all in Dasher coords).
  void Triangle(myint x, myint y1, myint y2, int fillColor, int outlineColor, int lineWidth);

  class CTextString {
  public: //to CDasherViewSquare...
    ///Creates a request that label will be drawn.
    /// x,y are screen coords of midpoint of leading edge;
    /// iSize is desired size (already computed from requested position)
    CTextString(CDasherScreen::Label *pLabel, screenint x, screenint y, int iSize, int iColor)
    : m_pLabel(pLabel), m_ix(x), m_iy(y), m_iSize(iSize), m_iColor(iColor) {
    }
    ~CTextString();
    CDasherScreen::Label *m_pLabel;
    screenint m_ix,m_iy;
    vector<CTextString *> m_children;
    int m_iSize;
    int m_iColor;
  };

  std::vector<CTextString *> m_DelayedTexts;

  void DoDelayedText(CTextString *pText);
  ///
  /// Draw text specified in Dasher co-ordinates
  ///

  CTextString *DasherDrawText(myint iMaxX, myint iMidY, CDasherScreen::Label *pLabel, CTextString *pParent, int iColor);

  ///
  /// (Recursively) render a node and all contained subnodes, in disjoint rects.
  /// (i.e. appropriate for LP_SHAPE_TYPE==0). Each call responsible for rendering
  /// exactly the area contained within the node.
  /// @param pOutput The innermost node covering the crosshair (if any)
  void DisjointRender(CDasherNode * Render, myint y1, myint y2, CTextString *prevText, CExpansionPolicy &policy, double dMaxCost, CDasherNode *&pOutput);

  /// (Recursively) render a node and all contained subnodes, in overlapping shapes
  /// (according to LP_SHAPE_TYPE: 1=rects, 2=triangles, 3=truncated triangles,
  /// 4=quadrics, 5=semicircles)
  /// Each call responsible for rendering exactly the area contained within the node.
  /// @param pOutput The innermost node covering the crosshair (if any)
  void NewRender(CDasherNode * Render, myint y1, myint y2, CTextString *prevText, CExpansionPolicy &policy, double dMaxCost, CDasherNode *&pOutput);

  /// @name Nonlinearity
  /// Implements the non-linear part of the coordinate space mapping

  /// Maps a dasher coordinate (linear in probability space, -ive x = in margin) to an abstract/resolution-independent
  /// screen coordinate (linear in screen space, -ive x = offscreen) - i.e. pixel coordinate = scale({x,y}map(dasher coord)))
  inline myint ymap(myint iDasherY) const;
  inline myint xmap(myint iDasherX) const;

  /// Inverse of the previous - i.e. dasher coord = iymap(scale(screen coord))
  inline myint iymap(myint y) const;
  inline myint ixmap(myint x) const;

  ///Parameters for y non-linearity. (TODO Make into preprocessor defines?)
  const myint m_Y1, m_Y2, m_Y3;

  inline void Crosshair();
  bool CoversCrosshair(myint Range,myint y1,myint y2);

  //Divides by SCALE_FACTOR, rounding away from 0
  inline myint CustomIDivScaleFactor(myint iNumerator);

  void DasherLine2Screen(myint x1, myint y1, myint x2, myint y2, vector<CDasherScreen::point> &vPoints);

  bool m_bVisibleRegionValid;

  // Called on screen size or orientation changes
  void SetScaleFactor();

  // Parameters for x non-linearity
  double m_dXlogCoeff;
  myint m_iXlogThres;

  //width of margin, in abstract screen coords
  myint iMarginWidth;

  /// There is a ratio of iScaleFactor{X,Y} abstract screen coords to SCALE_FACTOR real pixels
  /// (Note the naming convention: iScaleFactorX/Y refers to X/Y in Dasher-space, which will be
  /// the other way around to real screen coordinates if using a vertical (T-B/B-T) orientation)
  myint iScaleFactorX, iScaleFactorY;
  static const myint SCALE_FACTOR = 1<<26; //was 100,000,000; change to power of 2 => easier to multiply/divide

  /// Cached extents of visible region
  myint m_iDasherMinX;
  myint m_iDasherMaxX;
  myint m_iDasherMinY;
  myint m_iDasherMaxY;
};
/// @}
#include "DasherViewSquare.inl"

#endif /* #ifndef __DasherViewSquare_h__ */
