// DasherView.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// DasherView.h: interface for the DasherView class.
// Copyright 2002 David Ward
//////////////////////////////////////////////////////////////////////

#ifndef __DasherView_h_
#define __DasherView_h_

#include "../Common/MSVC_Unannoy.h"
#include "DasherScreen.h"
#include "DasherModel.h"

// CDasherView is an abstract view class
// The implentation must provide several functions - defined here as pure virtual functions
// See the CDasherViewSquare class for an example

namespace Dasher {class CDasherView;}
class Dasher::CDasherView
{
public:
	CDasherView(CDasherScreen* DasherScreen, CDasherModel& DasherModel, Dasher::Opts::ScreenOrientations Orientation=Dasher::Opts::LeftToRight);
	~CDasherView() {}		
	
	void ChangeOrientation(Dasher::Opts::ScreenOrientations Orientation);
	
	// renders Dasher
	inline void Render();
	
	// translates the screen coordinates to Dasher coordinates and calls
	// dashermodel.TapOnDisplay
	virtual void TapOnDisplay(int mousex, int mousey, unsigned long Time)=0;
	// translates the screen coordinates to Dasher coordinates and calls
	// dashermodel.GoTo
	virtual void GoTo(int mousex, int mousey)=0;
	
	virtual void ChangeScreen(CDasherScreen* NewScreen)
	{
		m_Screen=NewScreen;
		
		// DJW - removed floating point stuff
		//XYScale = (double)m_Screen->GetHeight() / m_Screen->GetWidth();
	}

	virtual void DrawGoTo(int mousex, int mousey)=0;
	virtual void DrawMouse(int mousex, int mousey)=0;
	virtual void DrawKeyboard()=0;

	// Return references to the model and the screen:
	CDasherModel& DasherModel() {return m_DasherModel;}
	CDasherScreen& Screen() {return *m_Screen;}

	void Display() {m_Screen->Display();}

	// Toggle advanced colour mode
	void SetColourMode(bool colourmode) {ColourMode=colourmode;}

	// Toggle keyboard control mode
	void SetKeyControl(bool keyboardcontrol) {KeyControl=keyboardcontrol;}

protected:
	// Orientation of Dasher Screen
	inline void MapScreen(int* DrawX, int* DrawY);
	inline void UnMapScreen(int* DrawX, int* DrawY);

	// Keyboard control is on
	bool KeyControl;

private:
	CDasherScreen* m_Screen;      // provides the graphics (text, lines, rectangles):
	CDasherModel& m_DasherModel; // Model view represents
	
	// Pure virtuals to implement
	virtual void Crosshair(myint sx)=0; // Tells m_Screen to draw a crosshair - or other static decoration
	virtual int RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme,
		myint y1, myint y2, int& mostleft, bool& force, bool text, std::string displaytext)=0;
	
	// Responsible for all the Render_node calls
	int RecursiveRender(CDasherNode* Render, myint y1,myint y2,int mostleft, bool text);
	
	// Displays some nodes inside one parent node. Used to group capital letters, accents, punctuation etc.
	void RenderGroups(CDasherNode* Render, myint y1, myint y2, bool text);
	
	// Orientation of Dasher Screen
	Dasher::Opts::ScreenOrientations ScreenOrientation;

	// Advanced colour mode
	bool ColourMode;

};


#include "DasherView.inl"


#endif /* #ifndef __DasherView_h_ */
