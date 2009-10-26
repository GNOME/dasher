// DasherView.h
//
// Copyright (c) 2001-2005 David Ward

#ifndef __DasherView_h_
#define __DasherView_h_

namespace Dasher {
  class CDasherScreen;
  class CDasherModel;
  class CDasherInput; // Why does DasherView care about input? - pconlon
  class CDasherComponent;
  class CDasherView;
  class CDasherNode;
}

#include "DasherTypes.h"
#include "DasherComponent.h"
#include "View/DelayedDraw.h"
#include "ExpansionPolicy.h"

/// \defgroup View Visualisation of the model
/// @{

/// \brief View base class.
///
/// Dasher views represent the visualisation of a Dasher model on the screen.
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
/// We should also attempt to try and remove the need for this class
/// to know about the model. When we call render we should just pass a
/// pointer to the root node, which we can obtain elsewhere, and make
/// sure that the data structure contains all the info we need to do
/// the rendering (eg make sure it contains strings as well as symbol
/// IDs).
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
  /// Translates the screen coordinates to Dasher coordinates and calls
  /// dashermodel.TapOnDisplay
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

  virtual bool IsNodeVisible(myint y1, myint y2) { return true; };

  virtual void VisibleRegion( myint &iDasherMinX, myint &iDasherMinY, myint &iDasherMaxX, myint &iDasherMaxY ) = 0;

  /// @}

  /// Change the screen - must be called if the Screen is replaced or resized
  /// \param NewScreen Pointer to the new CDasherScreen.

  virtual void ChangeScreen(CDasherScreen * NewScreen);

  /// @name High level drawing
  /// Drawing more complex structures, generally implemented by derived class
  /// @{

  /// Renders Dasher with mouse-dependent items
  /// \todo Clarify relationship between Render functions and probably only expose one
  virtual void Render(CDasherNode *pRoot, myint iRootMin, myint iRootMax, CExpansionPolicy &policy, bool bRedrawDisplay, std::vector<std::pair<myint,bool> >* pvGameTargetY);

  /// @}

  ////// Return a reference to the screen - can't be protected due to circlestarthandler
  
  CDasherScreen *Screen() {
    return m_pScreen;
  }

  ///
  /// @name Low level drawing
  /// Basic drawing primitives specified in Dasher coordinates.
  /// @{

  ///
  /// Draw a polyline specified in Dasher co-ordinates
  ///

  void DasherPolyline(myint * x, myint * y, int n, int iWidth, int iColour);

  /// Draw a polyarrow

  void DasherPolyarrow(myint * x, myint * y, int n, int iWidth, int iColour, double dArrowSizeFactor = 0.7071);

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

  void DrawText(const std::string & str, myint x, myint y, int Size);
  /// Request the Screen to copy its buffer to the Display
  /// \todo Shouldn't be public?
  void Display();

  /// @}

protected:
  // Orientation of Dasher Screen
/*   inline void MapScreen(screenint * DrawX, screenint * DrawY); */
/*   inline void UnMapScreen(screenint * DrawX, screenint * DrawY); */
  bool m_bVisibleRegionValid;
  
  CDelayedDraw *m_pDelayDraw;

  int m_iRenderCount;

private:
  CDasherScreen *m_pScreen;    // provides the graphics (text, lines, rectangles):
  CDasherInput *m_pInput;       // Input device abstraction

  /// Renders the Dasher node structure
  virtual void RenderNodes(CDasherNode *pRoot, myint iRootMin, myint iRootMax, CExpansionPolicy &policy, std::vector<std::pair<myint,bool> > *pvGamePointer) = 0;


  /// Get the co-ordinates from the input device
  int GetInputCoordinates(int iN, myint * pCoordinates); 

  bool m_bDemoMode;
  bool m_bGameMode;
};
/// @}

#endif /* #ifndef __DasherView_h_ */
