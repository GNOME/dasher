// DasherViewSquare.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherViewSquare_h__
#define __DasherViewSquare_h__

#include "../Common/MSVC_Unannoy.h"
#include "DasherScreen.h"
#include "DasherModel.h"
#include "DasherView.h"


// An implementation of the DasherView class
//
// This class renders Dasher in the vanilla style,
// but with horizontal and vertical mappings
//
// Horizontal mapping - linear and log
// Vertical mapping - linear with different gradient

namespace Dasher {class CDasherViewSquare;}
class Dasher::CDasherViewSquare : public Dasher::CDasherView
{
public:
	CDasherViewSquare(CDasherScreen* DasherScreen, CDasherModel& DasherModel, Dasher::Opts::ScreenOrientations Orientation=Dasher::Opts::LeftToRight);
	void TapOnDisplay(int mousex,int mousey, unsigned long Time);
	void GoTo(int mousex,int mousey);
	
	void ChangeScreen(CDasherScreen* NewScreen);
	void CDasherViewSquare::DrawGoTo(int mousex, int mousey);
	void DrawMouse(int mousex, int mousey);
	void DrawKeyboard();
private:
	// the x and y non-linearities
	inline const void screen2dasher(int *mousex, int *mousey);
	inline const int dasherx2screen(const myint sx);
	inline const int dashery2screen(myint sy);
	
	int RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme,
		myint y1, myint y2, int& mostleft, bool& force, bool text, std::string displaytext);
	
	void CheckForNewRoot();
	inline void Crosshair(myint sx);
	double m_dXmpa,m_dXmpb,m_dXmpc,m_dXmpd;
	myint m_Y1,m_Y2,m_Y3;
	myint s_Y1,s_Y2,s_Y3;
	int CanvasX,CanvasY,CanvasBorder;
	double xmap(double x);
	double ixmap(double x);
};

#include "DasherViewSquare.inl"


#endif /* #ifndef __DasherViewSquare_h__ */
