// DasherViewSquare.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// DasherViewSquare.cpp: implementation of the DasherViewSquare class.
// Copyright 2002 David Ward
//////////////////////////////////////////////////////////////////////

#include "DasherModel.h"
#include "DasherViewSquare.h"

using namespace Dasher;

// ARG! MSVC++ V6 doesn't seem to come with min and max in <algorithm>
#ifdef _MSC_VER
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#else
#include <algorithm>
using std::min;
using std::max;
#endif


CDasherViewSquare::CDasherViewSquare(CDasherScreen* DasherScreen, CDasherModel& DasherModel, Dasher::Opts::ScreenOrientations Orientation)
	: CDasherView(DasherScreen, DasherModel, Orientation)
{
	ChangeScreen(DasherScreen);
	
	// tweak these if you know what you are doing
	m_dXmpa=0.2;   // these are for the x non-linearity
	m_dXmpb=0.5;
	m_dXmpc=0.9;
	m_dXmpd=0.5;   // slow X movement when accelerating Y

	double dY1=0.05;    // these are for the y non-linearity
	double dY2=0.95;    // Accelerate Y movement below this point
	double dY3=0.05;    // Accelerate Y movement above this point

	m_Y2=int (dY2 * (CDasherView::DasherModel().DasherY()) );
	m_Y3=int (dY3 * (CDasherView::DasherModel().DasherY()) );
	m_Y1=int(1.0/dY1);
}


int CDasherViewSquare::RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme,
	myint y1, myint y2, int& mostleft, bool& force)
{
	int top = dashery2screen(y1);
	if (top>CanvasY)
		return 0;
	if (top<0)
		top=0;
	
	int bottom = dashery2screen(y2);
	if (bottom<0)
		return 0;
	if (bottom>CanvasY)
		bottom=CanvasY;
	
	int height=bottom-top;
	
	if (height==0)
		force=false;
	
	if (force || height>1) {
		force=true;
		
		int left=dasherx2screen(y2-y1);
		
		int right=CanvasX;
		
		int newleft=left, newtop=top, newright=right, newbottom=bottom;
		MapScreen(&newleft, &newtop);
		MapScreen(&newright, &newbottom);
		Screen().DrawRectangle(newleft, newtop, newright, newbottom, Color, ColorScheme);
		
		if (left<mostleft)
			left=mostleft;
		
		int Size;
		if (left<CanvasX*19/20) {
			Size = 20;
		} else if (left<CanvasX*159/160) {
			Size = 14;
		} else {
			Size = 11;
		}
		
		int TextWidth, TextHeight, OriginX=0, OriginY=0;
		Screen().TextSize(Character, &TextWidth, &TextHeight, Size);
		UnMapScreen(&TextWidth, &TextHeight);
		UnMapScreen(&OriginX, &OriginY);
		int FontHeight = abs(TextHeight-OriginY);
		int FontWidth = abs(TextWidth-OriginX);
		mostleft = left + FontWidth;
		
		int newleft2 = left;
		int newtop2 = (height-FontHeight)/2 + top;
		int newright2 = left + FontWidth;
		int newbottom2 = (height+FontHeight)/2 + top;
		MapScreen(&newleft2, &newtop2);
		MapScreen(&newright2, &newbottom2);
		newleft = min(newleft2, newright2);
		newtop = min(newtop2, newbottom2);
		
		Screen().DrawText(Character, newleft, newtop, Size);
		
		return 1;
	} else 
		return 0;
}


void CDasherViewSquare::CheckForNewRoot()
{
	CDasherNode * const root=DasherModel().Root();
	CDasherNode ** const children=root->Children();
	
	if (children==0)
		return;
	
	int alive=0;
	int theone=0;
	unsigned int i;
	for (i=1;i<root->Chars();i++) {
		if (children[i]->Alive()) {
			alive++;
			theone=i;
		}
	}
	
	if (alive==1) {
		myint y1=DasherModel().Rootmin();
		myint y2=DasherModel().Rootmax();
		myint range=y2-y1;
		myint newy1=y1+(range*children[theone]->Lbnd())/DasherModel().Normalization();
		myint newy2=y1+(range*children[theone]->Hbnd())/DasherModel().Normalization();
		
		if (newy1<0 && newy2> DasherModel().DasherY()) {
			myint left=dasherx2screen(newy2-newy1);
			if (left<0) {
				DasherModel().Make_root(theone);
				return;
			}
		}
	}
}


// work out the next viewpoint
// move the rectangles accordingly
void CDasherViewSquare::TapOnDisplay(int mousex,int mousey, unsigned long Time) 
{
	// convert mouse (screen) coords into dasher coords
	if (mousex>CanvasX)
		mousex=CanvasX;
	
	UnMapScreen(&mousex, &mousey);
	myint DasherMousey=screen2dashery(mousey);
	myint DasherMousex=screen2dasherx(mousex);
	
	DasherModel().Tap_on_display(DasherMousex,DasherMousey, Time);
	CheckForNewRoot();
}


void CDasherViewSquare::ChangeScreen(CDasherScreen* NewScreen)
{
	CDasherView::ChangeScreen(NewScreen);
	int Width = Screen().GetWidth();
	int Height = Screen().GetHeight();
	CanvasX=9*Width/10;
	CanvasBorder=Width-CanvasX;
	CanvasY=Height;
}

