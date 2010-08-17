// DasherViewSquare.h
//
// Copyright (c) 2001-2004 David Ward

#ifndef __DasherViewSquare_h__
#define __DasherViewSquare_h__
#include "DasherView.h"
#include "DelayedDraw.h"
#include "DasherScreen.h"
#include <deque>
#include "Alphabet/GroupInfo.h"



using namespace std;

namespace Dasher {
  class CDasherViewSquare;
  class CDasherView;
  class CDasherModel;
  class CDelayedDraw;
  class CDasherNode;
}

class Dasher::CDasherViewSquare;
class Dasher::CDasherModel;
class Dasher::CDelayedDraw;
class Dasher::CDasherNode;

/// \ingroup View
/// @{

/// An implementation of the DasherView class
///
/// This class renders Dasher in the vanilla style,
/// but with horizontal and vertical mappings
///
/// Horizontal mapping - linear and log
/// Vertical mapping - linear with different gradient
class Dasher::CDasherViewSquare:public Dasher::CDasherView
{
public:

  /// Constructor
  ///
  /// \param pEventHandler Event handler.
  /// \param pSettingsStore Settings store.
  /// \param DasherScreen Pointer to creen to which the view will render.
  /// \todo Don't cache screen and model locally - screen can be
  /// passed as parameter to the drawing functions, and data structure
  /// can be extracted from the model and passed too.

  CDasherViewSquare(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherScreen *DasherScreen);
  ~CDasherViewSquare();

  ///
  /// Event handler
  ///

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  /// 
  /// Supply a new screen to draw to
  ///

  void ChangeScreen(CDasherScreen * NewScreen);

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

private:
  ///draw a possibly-truncated triangle given dasher-space coords & accounting for non-linearity
  /// @param x = max dasher-x extent
  /// @param y1, y2 = dasher-y extent along y-axis
  /// @param midy1,midy2 = extent along line of max x (midy1==midy2 => triangle, midy1<midy2 => truncated tri)
  void TruncateTri(myint x, myint y1, myint y2, myint midy1, myint midy2, int fillColor, int outlineColor, int lineWidth);
  void CircleTo(myint cy, myint r, myint y1, myint x1, myint y3, myint x3, CDasherScreen::point dest, vector<CDasherScreen::point> &pts);
  void Circle(myint Range, myint lowY, myint highY, int fCol, int oCol, int lWidth);
  void Quadric(myint Range, myint lowY, myint highY, int fillColor, int outlineColour, int lineWidth);
  ///draw isoceles triangle, with baseline from y1-y2 along y axis (x=0), and other point at (x,(y1+y2)/2)
  /// (all in Dasher coords).
  void Triangle(myint x, myint y1, myint y2, int fillColor, int outlineColor, int lineWidth);
  
  class CTextString {
  public: //to CDasherViewSquare...
    ///Creates a request that string str will be drawn.
    /// x,y are screen coords of midpoint of leading edge;
    /// iSize is desired size (already computed from requested position)
    CTextString(const std::string & str, screenint x, screenint y, int iSize, int iColor)
    : m_String(str), m_ix(x), m_iy(y), m_iSize(iSize), m_iColor(iColor) {
    }
    ~CTextString();
    std::string m_String;
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
  
  CTextString *DasherDrawText(myint iMaxX, myint iMidY, const std::string & sDisplayText, CTextString *pParent, int iColor);
  
  ///
  /// (Recursively) render a node and all contained subnodes, in disjoint rects.
  /// (i.e. appropriate for LP_SHAPE_TYPE==0). Each call responsible for rendering
  /// exactly the area contained within the node.
  /// @param pOutput The innermost node covering the crosshair (if any)
  void DisjointRender(CDasherNode * Render, myint y1, myint y2, CTextString *prevText, CExpansionPolicy &policy, double dMaxCost,int parent_color, CDasherNode *&pOutput);
  
  /// (Recursively) render a node and all contained subnodes, in overlapping shapes
  /// (according to LP_SHAPE_TYPE: 1=rects, 2=triangles, 3=truncated triangles,
  /// 4=quadrics, 5=semicircles)
  /// Each call responsible for rendering exactly the area contained within the node.
  /// @param pOutput The innermost node covering the crosshair (if any)
  void NewRender(CDasherNode * Render, myint y1, myint y2, CTextString *prevText, CExpansionPolicy &policy, double dMaxCost,int parent_color, CDasherNode *&pOutput);

#ifdef _WIN32
  ///
  /// FIXME - couldn't find windows version of round(double) so here's one!
  /// \param number to be rounded
  ///
  double round(double d)
  {
    if(d - floor(d) < 0.5)
 	   return floor(d);
    else
 	   return ceil(d);
 
  };
#endif

  /// @name Nonlinearity
  /// Implements the non-linear part of the coordinate space mapping
  
  /// Maps a dasher Y coordinate to the Y value in a linear version of Dasher space (i.e. still not screen pixels)
  /// (i.e. screen coordinate = scale(ymap(dasher coord)))
  inline myint ymap(myint iDasherY) const;
  
  /// Inverse of the previous - i.e. dasher coord = iymap(scale(screen coord))
  myint iymap(myint y) const;
  ///parameters used by previous
  const myint m_Y1, m_Y2, m_Y3;

  myint xmap(myint x) const;
  myint ixmap(myint x) const;
  inline void Crosshair(myint sx);
  
  inline myint CustomIDiv(myint iNumerator, myint iDenominator);

  void DasherLine2Screen(myint x1, myint y1, myint x2, myint y2, vector<CDasherScreen::point> &vPoints);
  
  // Called on screen size or orientation changes
  void SetScaleFactor();

  // Data
 
  double m_dXmpa, m_dXmpb, m_dXmpc;
  screenint iCenterX;

  // Cached values for scaling
  myint iScaleFactorX;
  myint iScaleFactorY;

  // The factor that scale factors are multipled by 
  myint m_iScalingFactor;

  // Cached extents of visible region
  myint m_iDasherMinX;
  myint m_iDasherMaxX;
  myint m_iDasherMinY;
  myint m_iDasherMaxY;
};
/// @}
#include "DasherViewSquare.inl"

#endif /* #ifndef __DasherViewSquare_h__ */
