// DasherViewSquare.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

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
	CDasherViewSquare(CDasherScreen* DasherScreen, CDasherModel& DasherModel, CLanguageModel* LanguageModel, Dasher::Opts::ScreenOrientations Orientation=Dasher::Opts::LeftToRight, bool ColourMode=0);
	void TapOnDisplay(screenint mousex,screenint mousey, unsigned long Time);
	void GoTo(screenint mousex,screenint mousey);
	
	void ChangeScreen(CDasherScreen* NewScreen);
	void CDasherViewSquare::DrawGoTo(screenint mousex, screenint mousey);
	void DrawMouse(screenint mousex, screenint mousey);
	void DrawMouseLine(screenint mousex, screenint mousey);
	void DrawKeyboard();
private:
	// the x and y non-linearities
	void screen2dasher(screenint mousex, screenint mousey, myint* dasherx, myint* dashery) const;
	void AutoCalibrate(screenint *mousex, screenint *mousey);
    int dasherx2screen(myint sx) const ;
	int dashery2screen(myint sy) const ;
	double eyetracker_get_x(double x, double y);
	double eyetracker_get_y(double x, double y);
	double xmax(double x, double y) const;
    double xmap(double x) const;
	double ixmap(double x) const;

	int RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme,
		myint y1, myint y2, int& mostleft, bool& force, bool text, std::string displaytext);
	
	void CheckForNewRoot();
	inline void Crosshair(myint sx);
	double m_dXmpa,m_dXmpb,m_dXmpc,m_dXmpd;
//	myint s_Y1,s_Y2,s_Y3;
	int CanvasX,CanvasY,CanvasBorder;
	
	class Cymap
	{
	public:
		Cymap(myint iScale);
		Cymap() {}
		
		myint map(myint y) const;
	
		myint unmap(myint y) const;
	private:
		myint m_Y1,m_Y2,m_Y3;

	};


	Cymap m_ymap;
	
    // Calibration.
};

#include "DasherViewSquare.inl"


#endif /* #ifndef __DasherViewSquare_h__ */
