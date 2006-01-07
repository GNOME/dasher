// DasherViewSquare.h
//
// Copyright (c) 2001-2004 David Ward

#ifndef __DasherViewSquare_h__
#define __DasherViewSquare_h__
#include "DasherView.h"
#include <deque>

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

  CDasherViewSquare(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, CDasherScreen *DasherScreen, CDasherModel *DasherModel);
  ~CDasherViewSquare();

  ///
  /// Convert input coordinates to dasher coordinates and evolve the model
  ///

  void TapOnDisplay(screenint mousex, screenint mousey, unsigned long Time);
  void TapOnDisplay(screenint mousex,screenint mousey, unsigned long Time, VECTOR_SYMBOL_PROB* pAdded = NULL, int* pNumDeleted = NULL);

  ///
  /// \todo Document this
  ///
  
  void GoTo(screenint mousex, screenint mousey);

  ///
  /// \todo Document this
  ///

  void ClickTo(int x, int y, int width, int height);
  
  ///
  /// \todo Document this
  ///

  void DrawGoTo(screenint mousex, screenint mousey);

  ///
  /// Render the current state of the model.
  ///

  virtual void RenderNodes();

  /// 
  /// Supply a new screen to draw to
  ///

  void ChangeScreen(CDasherScreen * NewScreen);

  /// 
  /// Draw the mouse pointer
  ///

  void DrawMouse(screenint mousex, screenint mousey);

  ///
  /// Draw the mouse line
  ///

  void DrawMouseLine(screenint mousex, screenint mousey);

  ///
  /// \todo Document this
  ///

  void DrawKeyboard();

  ///
  /// Draw the game mode pointer
  ///

  void DrawGameModePointer();

  /// Get the scale factor for conversion between Dasher co-ordinates
  /// and screen co-ordinates

  void GetScaleFactor( int eOrientation, myint *iScaleFactorX, myint *iScaleFactorY );

  /// Checks for start on mouse behaviour and updates which boxes are
  /// displayed, triggers starts etc.

  virtual bool HandleStartOnMouse(int iTime);

  ///
  /// Event handler
  ///

  virtual void HandleEvent(Dasher::CEvent * pEvent);

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

  int RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme, myint y1, myint y2, int &mostleft, const std::string & displaytext, bool bShove);

  ///
  /// Recursively render all nodes in a tree. Responsible for all the Render_node calls
  ///

  int RecursiveRender(CDasherNode * Render, myint y1, myint y2, int mostleft, std::vector<CDasherNode *> &vNodeList);

  ///
  /// Displays some nodes inside one parent node. Used to group capital letters, accents, punctuation etc.
  ///
  
  void RenderGroups(CDasherNode * Render, myint y1, myint y2, int mostleft);

  /// 
  /// Get the automatic calibration offset
  ///

  int GetAutoOffset() const;
  bool CheckForNewRoot();
  ///
  /// \todo Document this
  ///

  virtual void ResetSum();

  ///
  /// \todo Document this
  ///

  virtual void ResetSumCounter();

  ///
  /// Reset the automatic calibration offset
  ///

  virtual void ResetYAutoOffset();

private:

  ///
  /// Convert input device position to Dasher co-ordinates
  ///

  void Input2Dasher(screenint iInputX, screenint iInputY, myint & iDasherX, myint & iDasherY, int iType, int iMode);

  /// 
  /// Convert a screen co-ordinate to Dasher co-ordinates
  ///

  void Screen2Dasher(screenint iInputX, screenint iInputY, myint & iDasherX, myint & iDasherY,bool b1D, bool bNonlinearity);

  ///
  /// Convert Dasher co-ordinates to screen co-ordinates
  ///

  void Dasher2Screen(myint iDasherX, myint iDasherY, screenint & iScreenX, screenint & iScreenY);

  ///
  /// Applies the 1D mode transformation
  ///

  void Dasher2OneD(myint & iDasherX, myint & iDasherY);

  ///
  /// Applies the eyetracker mode transformation
  ///

  void Dasher2Eyetracker(myint & iDasherX, myint & iDasherY);

  ///
  /// Trunates co-ordinates to fit on screen
  ///

  void TruncateToScreen(screenint & iX, screenint & iY);

  ///
  /// Draw a polyline specified in Dasher co-ordinates
  ///

  void DasherPolyline(myint * x, myint * y, int n, int iWidth, int iColour);

  ///
  /// Draw a polygon specified in Dasher co-ordinates
  ///

  void DasherPolygon(myint * x, myint * y, int n, int iColour);

  ///
  /// Draw a rectangle specified in Dasher co-ordinates
  ///

  void DasherDrawRectangle(myint iLeft, myint iTop, myint iRight, myint iBottom, const int Color, int iOutlineColour, Opts::ColorSchemes ColorScheme,bool bDrawOutline, bool bFill, int iThickness);

  ///
  /// Draw a centred rectangle specified in Dasher co-ordinates (used for mouse cursor)
  ///

  void DasherDrawCentredRectangle(myint iDasherX, myint iDasherY, screenint iSize, const int Color, Opts::ColorSchemes ColorScheme, bool bDrawOutline);

  ///
  /// Draw text specified in Dasher co-ordinates
  ///

  void DasherDrawText(myint iAnchorX1, myint iAnchorY1, myint iAnchorX2, myint iAnchorY2, const std::string & sDisplayText, int &mostleft, bool bShove);

  ///
  /// Get minimum visible Dasher Y co-ordinate
  /// \deprecated Use VisibleRegion.
  ///

  myint DasherVisibleMinY();

  ///
  /// Get maximum visible Dasher Y co-ordinate
  /// \deprecated Use VisibleRegion.
  ///

  myint DasherVisibleMaxY();

  ///
  /// Get maximum visible Dasher X co-ordinate
  /// \deprecated Use VisibleRegion.
  ///

  myint DasherVisibleMaxX();

  ///
  /// Get the bounding box of the visible region.
  ///

  void VisibleRegion( myint &iDasherMinX, myint &iDasherMinY, myint &iDasherMaxX, myint &iDasherMaxY );

  ///
  /// Unused
  ///

  myint m_iDasherXCache;

  ///
  /// Unused
  ///

  myint m_iDasherYCache;

  //Auto-speed control functions
  
  ///
  /// AUTO-SPEED-CONTROL
  /// This is the main speed control function and drives all of auto speed control.
  /// \param non-linear Dasher x coord
  /// \param non-linear Dasher y coord
  ///
  
  void SpeedControl(myint iDasherX, myint iDasherY);

  ///
  /// AUTO-SPEED-CONTROL
  /// Calculates the running variance of the angle between the +ve x-axis and the line joining 
  /// the cross hair to the mouse position.
  ///
  
  double Variance();

  ///
  /// AUTO-SPEED-CONTROL
  /// Updates the exclusion radius for auto speed control.
  ///
  
  double UpdateMinRadius();

  ///
  /// AUTO-SPEED-CONTROL
  /// Applies changes to the max bit rate depending on the running variance
  /// of the angle between the +ve x-axis and the line joining 
  /// the cross hair to the mouse position.
  ///
  
  double UpdateBitrate();

  ///
  /// AUTO-SPEED-CONTROL
  /// Adapts the number of samples taken so that auto speed control
  /// is invariant to clock rate and user ability (!!!).
  ///
  
  int UpdateSampleSize();

  ///
  /// AUTO-SPEED-CONTROL
  /// Updates the *variances* of the two populations of mixture-of-2-Gaussians
  /// distribution of radii. These are used to calculate the exclusion radius.
  /// \param radius
  ///
  
  void UpdateSigmas(double r);
  
  //
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
  //	myint s_Y1,s_Y2,s_Y3;
  
  // Variables for speed control
  
  //
  // AUTO-SPEED-CONTROL  
  double m_dBitrate; //  stores max bit rate internally
  double m_dSampleScale, m_dSampleOffset; // internal, control sample size
  int m_nSpeedCounter;  // keep track of how many samples
  int m_nSpeedSamples;  // upper limit on #samples
  double m_dSpeedMax, m_dSpeedMin; // bit rate always within this range
  double m_dTier1, m_dTier2, m_dTier3, m_dTier4; // variance tolerance tiers 
  double m_dChange1, m_dChange2, m_dChange3, m_dChange4; // fractional changes to bit rate
  double m_dMinRRate; // controls rate at which min. r adapts HIGHER===SLOWER!
  double m_dSensitivity; // not used, control sensitivity of auto speed control
  typedef std::deque<double> DOUBLE_DEQUE;
  DOUBLE_DEQUE m_dequeAngles; // store angles for statistics
  
  //variables for adaptive radius calculations...
  double m_dSigma1, m_dSigma2, m_dMinRadius;

  // Class definitions

  class Cymap {
  public:
    Cymap(myint iScale);
    Cymap() {}
    myint map(myint y) const;
    myint unmap(myint y) const;
  private:
    myint m_Y1, m_Y2, m_Y3;
  };

  // the x and y non-linearities

  ///
  /// \deprecated See Screen2Dasher
  ///

  void screen2dasher(screenint mousex, screenint mousey, myint * dasherx, myint * dashery) const;
  void AutoCalibrate(screenint * mousex, screenint * mousey);

  ///
  /// \deprecated See Dasher2Screen
  ///

  screenint dasherx2screen(myint sx) const;

  ///
  /// \deprecated See Dasher2Screen
  ///

  screenint dashery2screen(myint sy) const;

  ///
  /// \deprecated See Dasher2Screen
  ///

  Cint32 dashery2screen(myint y1, myint y2, screenint & s1, screenint & s2) const;

  double eyetracker_get_x(double x, double y);
  double eyetracker_get_y(double x, double y);
  double xmax(double x, double y) const;
  double xmap(double x) const;
  double ixmap(double x) const;


  inline void Crosshair(myint sx);

  // Data

  bool bInBox;                  // Whether we're in the mouseposstart box
  int iBoxStart;                // Time that the current box was drawn
  int iBoxEntered;              // Time when the user enttered the current box

  CDelayedDraw *m_pDelayDraw;

  double m_dXmpa, m_dXmpb, m_dXmpc, m_dXmpd;
  screenint CanvasX, CanvasY, CanvasBorder;

  int m_ySum, m_ySumCounter, m_yFilterTimescale, m_ySigBiasPixels, m_ySigBiasPercentage, m_yAutoOffset;

  Cymap m_ymap;

  // Called on screen size changes
  void SetScaleFactor();

  // Cached values for scaling
  myint iLRScaleFactorX;
  myint iLRScaleFactorY;
  myint iTBScaleFactorX;
  myint iTBScaleFactorY;

  // The factor that scale factors are multipled by 
  myint m_iScalingFactor;
};

#include "DasherViewSquare.inl"

#endif /* #ifndef __DasherViewSquare_h__ */
