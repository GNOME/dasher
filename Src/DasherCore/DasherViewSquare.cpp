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

	double dY1=0.25;    // Amount of acceleration
	double dY2=0.95;    // Accelerate Y movement below this point
	double dY3=0.05;    // Accelerate Y movement above this point

	m_Y2=int (dY2 * (CDasherView::DasherModel().DasherY()) );
	m_Y3=int (dY3 * (CDasherView::DasherModel().DasherY()) );
	m_Y1=int(1.0/dY1);

	KeyControl=false;
}


int CDasherViewSquare::RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme,
	myint y1, myint y2, int& mostleft, bool& force, bool text, std::string displaytext)
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
	
	if (height==0 && text==true)
		force=false;
	
	if (force || height>1) {
		force=true;

		int left=dasherx2screen(y2-y1);
		
		int right=CanvasX;
		
		int newleft=left, newtop=top, newright=right, newbottom=bottom;
		MapScreen(&newleft, &newtop);
		MapScreen(&newright, &newbottom);
		if( !text )
		  Screen().DrawRectangle(newleft, newtop, newright, newbottom, Color, ColorScheme);
		else
		  {
		    if (left<mostleft)
		      left=mostleft;
		    
		    int Size;
		    if (left*Screen().GetFontSize()<CanvasX*19/20) {
		      Size = 20*Screen().GetFontSize();
		    } else if (left*Screen().GetFontSize()<CanvasX*159/160) {
		      Size = 14*Screen().GetFontSize();
		    } else {
		      Size = 11*Screen().GetFontSize();
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

		    if(displaytext!="") {
		      Screen().DrawText(displaytext, newleft, newtop, Size);
		    } else {
		      Screen().DrawText(Character, newleft, newtop, Size);
		    }
		  }
		
		return 1;
	} else 
		return 0;
}


void CDasherViewSquare::CheckForNewRoot()
{
	CDasherNode * const root=DasherModel().Root();
	CDasherNode ** const children=root->Children();


	myint y1=DasherModel().Rootmin();
	myint y2=DasherModel().Rootmax();
	
	if ((y1>0 || y2 < DasherModel().DasherY() || dasherx2screen(y2-y1)>0)) {
	  DasherModel().Reparent_root(root->Lbnd(),root->Hbnd());
	  return;
	}
	    
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
	  y1=DasherModel().Rootmin();
	  y2=DasherModel().Rootmax();
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
        int Swapper;

	if (mousex>CanvasX)
		mousex=CanvasX;

	UnMapScreen(&mousex, &mousey);

	if (DasherModel().Dimensions()==true) {
	  switch (ScreenOrientation) {
	  case (LeftToRight):
	    break;
	  case (RightToLeft):
	    mousex = Screen().GetWidth() - mousex;
	    break;
	  case (TopToBottom):
	    Swapper = ( mousex * Screen().GetHeight()) / Screen().GetWidth();
	    mousex = (mousey  * Screen().GetWidth()) / Screen().GetHeight();
	    mousey = Swapper;
	    break;
	  case (BottomToTop):
	    // Note rotation by 90 degrees not reversible like others
	    Swapper = Screen().GetHeight() - ( mousex * Screen().GetHeight()) / Screen().GetWidth();
	    mousex = (mousey  * Screen().GetWidth()) / Screen().GetHeight();
	    mousey = Swapper;
	    break;
	  }
	}
	
	screen2dasher(&mousex,&mousey);
	DasherModel().Tap_on_display(mousex,mousey, Time);
	CheckForNewRoot();
}

// move to the specified point
void CDasherViewSquare::GoTo(int mousex,int mousey) 
{
	// convert mouse (screen) coords into dasher coords
	
	UnMapScreen(&mousex, &mousey);
	screen2dasher(&mousex,&mousey);
	DasherModel().GoTo(mousex,mousey);
	CheckForNewRoot();
}

void CDasherViewSquare::DrawGoTo(int mousex, int mousey)
{
  // Draw a box surrounding the area of the screen that will be zoomed into
  UnMapScreen(&mousex, &mousey);
  screen2dasher(&mousex,&mousey);
  // So, we have a set of coordinates. We need a bunch of points back.
  myint height=DasherModel().PlotGoTo(mousex, mousey);
  myint top, bottom, left, right;

  // Convert back to screen coordinates?
  top=mousey-height/2;
  bottom=mousey+height/2;
  left=mousex+height/2;
  right=mousex-height/2;
  top=dashery2screen(top);
  bottom=dashery2screen(bottom);
  left=dasherx2screen(left);
  right=dasherx2screen(right);
  
  // Draw the lines
  Screen().DrawRectangle(left, top+5, right, top-5, 1, Opts::ColorSchemes(Objects));
  Screen().DrawRectangle(left+5, top+5, left, bottom-5, 1, Opts::ColorSchemes(Objects));
  Screen().DrawRectangle(left, bottom+5, right, bottom-5, 1, Opts::ColorSchemes(Objects));
}
  

void CDasherViewSquare::DrawMouse(int mousex, int mousey)
{
        if (DasherModel().Dimensions()==true) {
  
	  int Swapper;
	
	  screen2dasher(&mousex,&mousey);
	  mousex=dasherx2screen(mousex);
	  mousey=dashery2screen(mousey);
	  switch (ScreenOrientation) {
	  case (LeftToRight):
	    break;
	  case (RightToLeft):
	    mousex = Screen().GetWidth() - mousex;
	    break;
	  case (TopToBottom):
	    Swapper = ( mousex * Screen().GetHeight()) / Screen().GetWidth();
	    mousex = (mousey  * Screen().GetWidth()) / Screen().GetHeight();
	    mousey = Swapper;
	    break;
	  case (BottomToTop):
	    // Note rotation by 90 degrees not reversible like others
	    Swapper = Screen().GetHeight() - ( mousex * Screen().GetHeight()) / Screen().GetWidth();
	    mousex = (mousey  * Screen().GetWidth()) / Screen().GetHeight();
	    mousey = Swapper;
	    break;
	  }
	}

	Screen().DrawRectangle(mousex-5, mousey-5, mousex+5, mousey+5, 1, Opts::ColorSchemes(Objects));
}

void CDasherViewSquare::DrawKeyboard()
{
  CDasherScreen::point line[2];
  line[0].x = 200;
  line[0].y = 0;
  line[1].x = 200;
  line[1].y = CanvasY/2;
  
  Screen().Polyline(line,2);

  line[0].x = 200;
  line[0].y = CanvasY/2;
  line[1].x = 0;
  line[1].y = CanvasY/2;

  Screen().Polyline(line,2);

  line[0].x = 200;
  line[0].y = CanvasY/2;
  line[1].x = 200;
  line[1].y = CanvasY;

  Screen().Polyline(line,2);
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


