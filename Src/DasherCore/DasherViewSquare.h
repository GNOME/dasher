//////////////////////////////////////////////////////////////////////
// DasherViewSquare.h: interface for the DasherViewSquare class.
// Copyright 20001-2002 David Ward
//////////////////////////////////////////////////////////////////////

#ifndef __DasherViewSquare_h__
#define __DasherViewSquare_h__

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
	
	void ChangeScreen(CDasherScreen* NewScreen);
private:
	// the x and y non-linearities
	inline const myint screen2dasherx(const int mousex);
	inline const myint screen2dashery(const int mousey);
	inline const int dasherx2screen(const myint sx);
	inline const int dashery2screen(const myint sy);
	
	int RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme,
		myint y1, myint y2, int& mostleft, bool& force);
	
	void CheckForNewRoot();
	inline void Crosshair(myint sx);
	double m_dY1,m_dY2,m_dY3,m_dXmpc,m_dXmpa,m_dXmpb;
	int CanvasX,CanvasY,CanvasBorder;
	double ymap(double y);
	double iymap(double y);
	double xmap(double x);
	double ixmap(double x);
};

#include "DasherViewSquare.inl"


#endif /* #ifndef __DasherViewSquare_h__ */
