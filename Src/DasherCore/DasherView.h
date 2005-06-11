// DasherView.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherView_h_
#define __DasherView_h_

#include "DasherScreen.h"
#include "DasherModel.h"

/////////////////////////////////////////////////////////////////////////////

// CDasherView is an abstract view class
// See the CDasherViewSquare class for an example

// DJW 200504 - at the moment its quite hard work to plug in a new view

/////////////////////////////////////////////////////////////////////////////

namespace Dasher {class CDasherView;}
class Dasher::CDasherView
{
public:

	CDasherView(CDasherScreen* DasherScreen, CDasherModel& DasherModel, Dasher::Opts::ScreenOrientations Orientation=Dasher::Opts::LeftToRight, bool ColourMode=0);
	virtual ~CDasherView() {}		
	
	void ChangeOrientation(Dasher::Opts::ScreenOrientations Orientation);


	void SetDrawMouse(bool bDrawMouse);
	void SetDrawMouseLine(bool bDrawMouseLine);
	void SetDrawKeyboard(bool bDrawKeyboard);
	
	// 0 - no box, 1 - upper box, 2 - lower box
	void SetDrawMousePosBox(int MousePosBox);


	// Renders Dasher
	virtual void Render();

	// Renders Dasher with mouse-dependent items
	virtual bool Render(int iMouseX, int iMouseY, bool bRedrawDisplay);

	// Renders the Dasher node structure
	virtual void RenderNodes()=0;
	
	// translates the screen coordinates to Dasher coordinates and calls
	// dashermodel.TapOnDisplay
	virtual void TapOnDisplay(screenint mousex, screenint mousey, unsigned long Time)=0;
	
	// translates the screen coordinates to Dasher coordinates and calls
	// dashermodel.GoTo
	virtual void GoTo(screenint mousex, screenint mousey)=0;
	
	// Change the screen - must be called if the Screen is replaced or resized
	virtual void ChangeScreen(CDasherScreen* NewScreen);

	virtual int GetAutoOffset() const {return 0;}
	virtual void DrawGoTo(screenint mousex, screenint mousey)=0;
	virtual void DrawMouse(screenint mousex, screenint mousey)=0;
	virtual void DrawMouseLine(screenint mousex, screenint mousey)=0;
	virtual void DrawKeyboard()=0;

	virtual void DrawMousePosBox();

	// Return references to the model and the screen:
	CDasherModel& DasherModel() {return m_DasherModel;}
	const CDasherModel& DasherModel() const {return m_DasherModel;}
	CDasherScreen& Screen() {return *m_pScreen;}

	// Request the Screen to copy its buffer to the Display
	void Display() {m_pScreen->Display();}

	// Toggle advanced colour mode
	void SetColourMode(bool colourmode) {ColourMode=colourmode;}

	// Toggle keyboard control mode
	void SetKeyControl(bool keyboardcontrol) {KeyControl=keyboardcontrol;}

    int GetOneButton() const;
    void SetOneButton(int Value);
    
	virtual void ResetSum() {}
	virtual void ResetSumCounter() {}
	virtual void ResetYAutoOffset() {}
    

protected:
	// Orientation of Dasher Screen
	inline void MapScreen(screenint* DrawX, screenint* DrawY);
	inline void UnMapScreen(screenint* DrawX, screenint* DrawY);

	// Keyboard control is on
	bool KeyControl;

	// Orientation of Dasher Screen
	Dasher::Opts::ScreenOrientations ScreenOrientation;

	// Advanced colour mode
	bool ColourMode;

	int onebutton;
    bool AutoCalibrate;

 
private:
	CDasherScreen* m_pScreen;      // provides the graphics (text, lines, rectangles):
	CDasherModel& m_DasherModel; // Model view represents

	// Pure virtuals to implement
	virtual void Crosshair(myint sx)=0; // Tells m_Screen to draw a crosshair - or other static decoration

	bool m_bDrawMouse;
	bool m_bDrawMouseLine;
	bool m_bDrawKeyboard;
	int m_iDrawMousePosBox;
	int m_iMousePosDist;

};


#include "DasherView.inl"


#endif /* #ifndef __DasherView_h_ */
