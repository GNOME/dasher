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
	
	// TODO Sort this out
	void FlushAt(int mousex,int mousey);
	
	// renders Dasher
	inline void Render();
	
	// translates the screen coordinates to Dasher coordinates and calls
	// dashermodel.TapOnDisplay
	virtual void TapOnDisplay(int mousex, int mousey, unsigned long Time)=0;
	
	virtual void ChangeScreen(CDasherScreen* NewScreen)
	{
		m_Screen=NewScreen;
		
		// DJW - removed floating point stuff
		//XYScale = (double)m_Screen->GetHeight() / m_Screen->GetWidth();
	}
	
	// Return references to the model and the screen:
	CDasherModel& DasherModel() {return m_DasherModel;}
	CDasherScreen& Screen() {return *m_Screen;}
protected:
	// Orientation of Dasher Screen
	inline void MapScreen(int* DrawX, int* DrawY);
	inline void UnMapScreen(int* DrawX, int* DrawY);
private:
	CDasherScreen* m_Screen;      // provides the graphics (text, lines, rectangles):
	CDasherModel& m_DasherModel; // Model view represents
	
	// Pure virtuals to implement
	virtual void Crosshair(myint sx)=0; // Tells m_Screen to draw a crosshair - or other static decoration
	virtual int RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme,
		myint y1, myint y2, int& mostleft, bool& force)=0;
	
	// Responsible for all the Render_node calls
	int RecursiveRender(CDasherNode* Render, myint y1,myint y2,int mostleft);
	
	// Displays some nodes inside one parent node. Used to group capital letters, accents, punctuation etc.
	void RenderGroups(CDasherNode* Render, myint y1, myint y2);
	
	// Orientation of Dasher Screen
	Dasher::Opts::ScreenOrientations ScreenOrientation;

	// DJW - removed floating point stuff
	//double XYScale;
};


#include "DasherView.inl"


#endif /* #ifndef __DasherView_h_ */
