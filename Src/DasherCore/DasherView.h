// DasherView.h
//
// Copyright (c) 2001-2005 David Ward

#ifndef __DasherView_h_
#define __DasherView_h_

namespace Dasher {
  class CDasherScreen;
  class CDasherModel;
  class CDasherInput;
  class CDasherComponent;
  class CDasherView;
  class CDasherNode;
}

#include "DasherTypes.h"
#include "DasherComponent.h"
#include "View/DelayedDraw.h"

// CDasherView is an abstract view class
// See the CDasherViewSquare class for an example

// DJW 200504 - at the moment its quite hard work to plug in a new view

////// \brief View base class.
////// Dasher views represent the visualisation of a Dasher model on the screen.
////// Note that we really should aim to avoid having to try and keep
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
////// There are really three roles played by CDasherView: providing high
/// level drawing functions, providing a mapping between Dasher
/// co-ordinates and screen co-ordinates and providing a mapping
/// between true and effective Dasher co-ordinates (eg for eyetracking
/// mode). We should probably consider creating separate classes for
/// these.

class Dasher::CDasherView:public CDasherComponent
{
public:

  /// 
  /// \param pEventHandler Pointer to the event handler
  /// \param pSettingsStore Pointer to the settings store
  /// \param DasherScreen Pointer to the CDasherScreen object used to do rendering
  /// \param DasherModel Reference to the CDasherModel which is to be represented

  CDasherView(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherScreen * DasherScreen);
  virtual ~ CDasherView() {
  }
  ////// Event handler
  /// \param pEvent Pointer to incoming event
  virtual void HandleEvent(Dasher::CEvent * pEvent);

  virtual bool IsNodeVisible(myint y1, myint y2) { return true; };

  //void SetDrawKeyboard(bool bDrawKeyboard);

  // 0 - no box, 1 - upper box, 2 - lower box
  //void SetDrawMousePosBox(int MousePosBox);

  /// Render the display

/*   virtual void Render(CDasherNode *pRoot, myint iRootMin, myint iRootMax, std::vector<CDasherNode *> &vNodeList, std::vector<CDasherNode *> &vDeleteList); */

  /// Renders Dasher with mouse-dependent items
  /// \todo Clarify relationship between Render functions and probably only expose one

  virtual bool Render(CDasherNode *pRoot, myint iRootMin, myint iRootMax, std::vector<CDasherNode *> &vNodeList, std::vector<CDasherNode *> &vDeleteList, bool bRedrawDisplay);

  /// Renders the Dasher node structure
  /// \todo Shouldn't be public?

  virtual void RenderNodes(CDasherNode *pRoot, myint iRootMin, myint iRootMax, std::vector<CDasherNode *> &vNodeList, std::vector<CDasherNode *> &vDeleteList) = 0;

  /// Translates the screen coordinates to Dasher coordinates and calls
  /// dashermodel.TapOnDisplay

  virtual void TapOnDisplay(screenint mousex, screenint mousey, unsigned long Time, myint &iDasherX, myint &iDasherY, VECTOR_SYMBOL_PROB* pAdded = NULL, int* pNumDeleted = NULL)=0;

  /// 
  /// Convert a screen co-ordinate to Dasher co-ordinates
  ///

  virtual void Screen2Dasher(screenint iInputX, screenint iInputY, myint & iDasherX, myint & iDasherY,bool b1D, bool bNonlinearity) = 0;

  ///
  /// Convert Dasher co-ordinates to screen co-ordinates
  ///

  virtual void Dasher2Screen(myint iDasherX, myint iDasherY, screenint & iScreenX, screenint & iScreenY) = 0;

  ///
  /// Convert input device position to Dasher co-ordinates
  ///

  virtual void Input2Dasher(screenint iInputX, screenint iInputY, myint & iDasherX, myint & iDasherY, int iType, int iMode) = 0;

  /// Change the screen - must be called if the Screen is replaced or resized
  /// \param NewScreen Pointer to the new CDasherScreen.

  virtual void ChangeScreen(CDasherScreen * NewScreen);

  /// Get autocallibration offset
  /// \retval Current offset

  virtual int GetAutoOffset() const {
    return 0;
  }
/*   ////// \todo Document this */
/*   virtual void DrawGoTo(screenint mousex, screenint mousey) = 0; */

   virtual void NewDrawGoTo(myint iDasherMin, myint iDasherMax, bool bActive) = 0;

  /// Draw the game mode pointer - this shouldn't be here

  virtual void DrawGameModePointer() = 0;

  ////// Return a reference to the screen
  
  CDasherScreen *Screen() {
    return m_pScreen;
  }

  /// Request the Screen to copy its buffer to the Display
  /// \todo Shouldn't be public?

  void Display();

  /// \todo Document this

  virtual void ResetSum() {
  }

  /// \todo Document this

  virtual void ResetSumCounter() {
  }

  /// \todo Document this

  virtual void ResetYAutoOffset() {
  }

  /// Set the input device class. Note that this class will now assume ownership of the pointer, ie it will delete the object when it's done with it.
  /// \param _pInput Pointer to the new CDasherInput.

  void SetInput(CDasherInput * _pInput);


  virtual void SpeedControl(myint iDasherX, myint iDasherY, double dFrameRate) {};

  virtual double xmap(double x) const {return 0.0;};   
  virtual double ymap(double x) const {return 0.0;}; 

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


  virtual void VisibleRegion( myint &iDasherMinX, myint &iDasherMinY, myint &iDasherMaxX, myint &iDasherMaxY ) = 0;

  /// Get the co-ordinates from the input device
  /// \todo This shouldn't be public?
  
  int GetCoordinates(int iN, myint * pCoordinates); 
  
  /// Get the co-ordinate count from the input device
  
  int GetCoordinateCount();


protected:
  // Orientation of Dasher Screen
  inline void MapScreen(screenint * DrawX, screenint * DrawY);
  inline void UnMapScreen(screenint * DrawX, screenint * DrawY);
  bool m_bVisibleRegionValid;
 
 
  CDelayedDraw *m_pDelayDraw;

private:
  CDasherScreen *m_pScreen;    // provides the graphics (text, lines, rectangles):
  CDasherInput *m_pInput;       // Input device abstraction

  // Pure virtuals to implement */
  virtual void Crosshair(myint sx) = 0; // Tells m_Screen to draw a crosshair - or other static decoration */


};

#include "DasherView.inl"

#endif /* #ifndef __DasherView_h_ */
