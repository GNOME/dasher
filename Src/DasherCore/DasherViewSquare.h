// DasherViewSquare.h
//
// Copyright (c) 2001-2004 David Ward

#ifndef __DasherViewSquare_h__
#define __DasherViewSquare_h__
#include "DasherView.h"
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

  /// \param pEventHandler Event handler.
  /// \param pSettingsStore Settings store.
  /// \param DasherScreen Pointer to creen to which the view will render.
  /// \param DasherModel The model which will be rendered.
  /// \todo Don't cache screen and model locally - screen can be
  /// passed as parameter to the drawing functions, and data structure
  /// can be extracted from the model and passed too.

  CDasherViewSquare(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherScreen *DasherScreen);
  ~CDasherViewSquare();

  ///
  /// Convert input coordinates to dasher coordinates and evolve the model
  ///

  int GetCoordinates(unsigned long Time, myint &iDasherX, myint &iDasherY);
  
  ///
  /// \todo Document this
  ///

/*   void DrawGoTo(screenint mousex, screenint mousey); */

  void NewDrawGoTo(myint iDasherMin, myint iDasherMax, bool bActive);

  ///
  /// Render the current state of the model.
  ///

  virtual void RenderNodes(CDasherNode *pRoot, myint iRootMin, myint iRootMax, std::vector<CDasherNode *> &vNodeList, std::vector<CDasherNode *> &vDeleteList, myint *iGamePointer);

  /// 
  /// Supply a new screen to draw to
  ///

  void ChangeScreen(CDasherScreen * NewScreen);

  ///
  /// Draw the game mode pointer
  ///

  void DrawGameModePointer(myint iPosition);

  /// Get the scale factor for conversion between Dasher co-ordinates
  /// and screen co-ordinates

  void GetScaleFactor( int eOrientation, myint *iScaleFactorX, myint *iScaleFactorY );

  ///
  /// Event handler
  ///

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  /// 
  /// Convert a screen co-ordinate to Dasher co-ordinates
  ///

  void Screen2Dasher(screenint iInputX, screenint iInputY, myint & iDasherX, myint & iDasherY,bool b1D, bool bNonlinearity);

  ///
  /// Convert Dasher co-ordinates to screen co-ordinates
  ///

  void Dasher2Screen(myint iDasherX, myint iDasherY, screenint & iScreenX, screenint & iScreenY);


private:

  double xmap(double x) const;
  double ymap(double x) const {
    return m_ymap.map( (myint)x );
  };

  
  ///
  /// Recursively render all nodes in a tree. Responsible for all the Render_node calls
  ///

  int RecursiveRender(CDasherNode * Render, myint y1, myint y2, int mostleft, std::vector<CDasherNode *> &vNodeList, std::vector<CDasherNode *> &vDeleteList, myint *iGamePointer, bool bDraw,myint parent_width,int parent_color);

  ///
  /// Displays some nodes inside one parent node. Used to group capital letters, accents, punctuation etc.
  /// Its not finished!!!!
  ///
  
  void RenderGroupsFast(CDasherNode * Render, myint y1, myint y2,int mostleft);

  /// Render a single node
  /// \param Character Symbol ID to be drawn
  /// \param Color The colour to draw it
  /// \param ColorScheme Which of the alternating colour schemes to use
  /// \param y1 Upper extent.
  /// \param y2 Lower extent
  /// \param mostleft The left most position in which the text (l->r)
  /// can be displayed in order to avoid overlap. This is updated by
  /// the function to allow for the new text
  /// \param displaytext Text to display.
  /// \todo Character and displaytext are redundant. We shouldn't need
  /// to know about alphabets here, so only use the latterr

  int RenderNode(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove);
  int RenderNodeOutlineFast(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove); 
  int RenderNodePartFast(const int Color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove, myint iParentWidth);
  int RenderNodeFatherFast(const int parent_color, myint y1, myint y2, int &mostleft, const std::string &sDisplayText, bool bShove,myint iParentWidth);
  bool IsNodeVisible(myint y1, myint y2);

  ///
  /// Its not finished!!!!
  ///
  
  void RecursiveRenderGroupsFast(SGroupInfo *pCurrentGroup, CDasherNode *pNode, myint y1, myint y2, int mostleft,int iParentColor);

  ///
  /// Get the bounding box of the visible region.
  ///

  void VisibleRegion( myint &iDasherMinX, myint &iDasherMinY, myint &iDasherMaxX, myint &iDasherMaxY );
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

  // Class definitions

  class Cymap {
  public:
    Cymap(myint iScale);
    Cymap() {}
    inline myint map(myint y) const;
    myint unmap(myint y) const;
  private:
    myint m_Y1, m_Y2, m_Y3;
  };

  double xmax(double x, double y) const;
  double ixmap(double x) const;
  inline void Crosshair(myint sx);

  // Called on screen size changes
  void SetScaleFactor();

  // Data

  bool bInBox;                  // Whether we're in the mouseposstart box
  int iBoxStart;                // Time that the current box was drawn
  int iBoxEntered;              // Time when the user enttered the current box
 
  double m_dXmpa, m_dXmpb, m_dXmpc, m_dXmpd;
  screenint CanvasX, CanvasY, CanvasBorder;
  Cymap m_ymap;

  // Cached values for scaling
  myint iLRScaleFactorX;
  myint iLRScaleFactorY;
  myint iTBScaleFactorX;
  myint iTBScaleFactorY;

  // The factor that scale factors are multipled by 
  myint m_iScalingFactor;

  // Cached extents of visible region
  myint m_iDasherMinX;
  myint m_iDasherMaxX;
  myint m_iDasherMinY;
  myint m_iDasherMaxY;
};

#include "DasherViewSquare.inl"

#endif /* #ifndef __DasherViewSquare_h__ */
