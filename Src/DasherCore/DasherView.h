// DasherView.h
//
// Copyright (c) 2001-2005 David Ward

#ifndef __DasherView_h_
#define __DasherView_h_

#include "DasherScreen.h"
#include "DasherModel.h"
#include "DasherInput.h"
#include "DasherComponent.h"

#include <iostream>

// CDasherView is an abstract view class
// See the CDasherViewSquare class for an example

// DJW 200504 - at the moment its quite hard work to plug in a new view

namespace Dasher {
  class CDasherView;
}
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

  CDasherView(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherScreen * DasherScreen, CDasherModel & DasherModel);
  virtual ~ CDasherView() {
  }
  ////// Event handler
  /// \param pEvent Pointer to incoming event
  virtual void HandleEvent(Dasher::CEvent * pEvent);

  /// \deprecated Use parameter interface instead

  void ChangeOrientation(Dasher::Opts::ScreenOrientations Orientation);

  //void SetDrawKeyboard(bool bDrawKeyboard);

  // 0 - no box, 1 - upper box, 2 - lower box
  //void SetDrawMousePosBox(int MousePosBox);

  /// Render the display

  virtual void Render();

  /// Renders Dasher with mouse-dependent items
  /// \todo Clarify relationship between Render functions and probably only expose one

  virtual bool Render(int iMouseX, int iMouseY, bool bRedrawDisplay);

  /// Renders the Dasher node structure
  /// \todo Shouldn't be public?

  virtual void RenderNodes() = 0;

  /// Translates the screen coordinates to Dasher coordinates and calls
  /// dashermodel.TapOnDisplay

	virtual void TapOnDisplay(screenint mousex, screenint mousey, unsigned long Time, VECTOR_SYMBOL_PROB* pAdded = NULL, int* pNumDeleted = NULL)=0;

  /// Handles start-on-mouse behaviour - check whether we are in the box, and change box or start on timer,.
  /// \param iTime Current time in ms.

  virtual bool HandleStartOnMouse(int iTime) = 0;

  /// translates the screen coordinates to Dasher coordinates and calls
  /// dashermodel.GoTo

  virtual void GoTo(screenint mousex, screenint mousey) = 0;

  /// Change the screen - must be called if the Screen is replaced or resized
  /// \param NewScreen Pointer to the new CDasherScreen.

  virtual void ChangeScreen(CDasherScreen * NewScreen);

  /// Get autocallibration offset
  /// \retval Current offset

  virtual int GetAutoOffset() const {
    return 0;
  }
  ////// \todo Document this
  virtual void DrawGoTo(screenint mousex, screenint mousey) = 0;

  /// Draw the mouse cursor
  /// \todo Probably shouldn't be public

  virtual void DrawMouse(screenint mousex, screenint mousey) = 0;

  /// Draw the mouse line
  /// \todo Probably shouldn't be public

  virtual void DrawMouseLine(screenint mousex, screenint mousey) = 0;

  /// \todo Document this

  virtual void DrawKeyboard() = 0;

  /// \todo Document this

  virtual void DrawMousePosBox();

  /// Draw the game mode pointer

  virtual void DrawGameModePointer() = 0;

  /// 
  /// Return a reference to the model

  CDasherModel & DasherModel() {
    return m_DasherModel;
  }

  /// \todo Erm...

  const CDasherModel & DasherModel() const {
    return m_DasherModel;
  }
  ////// Return a reference to the screen
  
  CDasherScreen & Screen() {
    return *m_pScreen;
  }

  /// Request the Screen to copy its buffer to the Display
  /// \todo Shouldn't be public?

  void Display() {
    m_pScreen->Display();
  }

  // Toggle advanced colour mode
  //void SetColourMode(bool colourmode) {ColourMode=colourmode;}

  // Toggle keyboard control mode
  //void SetKeyControl(bool keyboardcontrol) {KeyControl=keyboardcontrol;}

  /// \todo Document this

  virtual void ResetSum() {
  }

  /// \todo Document this

  virtual void ResetSumCounter() {
  }

  /// \todo Document this

  virtual void ResetYAutoOffset() {
  }

  //void SetTruncation( int iTruncation ) {
  //  m_iTruncation = iTruncation;
  //}

  //void SetTruncationType( int iTruncationType ) {
  //  m_iTruncationType = iTruncationType;
  //}

  /// Set the input device class. Note that this class will now assume ownership of the pointer, ie it will delete the object when it's done with it.
  /// \param _pInput Pointer to the new CDasherInput.

  void SetInput(CDasherInput * _pInput) {

    DASHER_ASSERT_VALIDPTR_RW(_pInput);

    // Delete the old class if we have one

    if(m_pInput)
      delete m_pInput;

    m_pInput = _pInput;

    // Tell the new object about maximum values

    myint iMaxCoordinates[2];

    iMaxCoordinates[0] = m_DasherModel.DasherY();
    iMaxCoordinates[1] = m_DasherModel.DasherY();

    m_pInput->SetMaxCoordinates(2, iMaxCoordinates);

  }

  /// Get the co-ordinates from the input device
  /// \todo This shouldn't be public?

  int GetCoordinates(int iN, myint * pCoordinates) {

    if(m_pInput)
      return m_pInput->GetCoordinates(iN, pCoordinates);

    return 0;
  }

  /// Get the co-ordinate count from the input device

  int GetCoordinateCount() {
    if(m_pInput)
      return m_pInput->GetCoordinateCount();

    return 0;
  }

protected:
  // Orientation of Dasher Screen
  inline void MapScreen(screenint * DrawX, screenint * DrawY);
  inline void UnMapScreen(screenint * DrawX, screenint * DrawY);

private:
  CDasherScreen * m_pScreen;    // provides the graphics (text, lines, rectangles):
  CDasherModel & m_DasherModel; // Model view represents
  CDasherInput *m_pInput;       // Input device abstraction

  // Pure virtuals to implement
  virtual void Crosshair(myint sx) = 0; // Tells m_Screen to draw a crosshair - or other static decoration
};

#include "DasherView.inl"

#endif /* #ifndef __DasherView_h_ */
