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

  /// @}

private:
  ///
  /// Draw text specified in Dasher co-ordinates
  ///
  
  void DasherDrawText(myint iAnchorX1, myint iAnchorY1, myint iAnchorX2, myint iAnchorY2, const std::string & sDisplayText, int &mostleft, bool bShove);
  
  CDelayedDraw m_DelayDraw;  

  ///
  /// Render the current state of the model.
  ///
  virtual void RenderNodes(CDasherNode *pRoot, myint iRootMin, myint iRootMax, CExpansionPolicy &policy);
  
  ///
  /// Recursively render all nodes in a tree. Responsible for all the Render_node calls
  ///

  void RecursiveRender(CDasherNode * Render, myint y1, myint y2, int mostleft, CExpansionPolicy &policy, double dMaxCost, myint parent_width,int parent_color, int iDepth);

  ///Check that a node is large enough, and onscreen, to render;
  ///calls RecursiveRender if so, or collapses the node immediately if not
  bool CheckRender(CDasherNode * Render, myint y1, myint y2, int mostleft, CExpansionPolicy &policy, double dMaxCost, myint parent_width,int parent_color, int iDepth);

  /// Render a single node
  /// \param Color The colour to draw it
  /// \param y1 Upper extent.
  /// \param y2 Lower extent
  /// \param mostleft The left most position in which the text (l->r)
  /// can be displayed in order to avoid overlap. This is updated by
  /// the function to allow for the new text
  /// \param sDisplayText Text to display.
  /// \param bShove Whether the node shoves
  /// \todo Character and displaytext are redundant. We shouldn't need
  /// to know about alphabets here, so only use the latterr
  //  int RenderNode(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove);

  int RenderNodeOutlineFast(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove); 
  int RenderNodePartFast(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove, myint iParentWidth);
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
