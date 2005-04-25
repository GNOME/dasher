// DasherViewSquare.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherViewSquare.h"
#include "DasherModel.h"

#include <algorithm>

using namespace Dasher;

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::RenderNodes()
{
	Screen().Blank();
	
	DASHER_ASSERT(DasherModel().Root()!=0);

//	DASHER_TRACEOUTPUT("Render\n");

	// Render nodes to screen object (should use off screen buffer)

	RecursiveRender(DasherModel().Root(), DasherModel().Rootmin(), DasherModel().Rootmax(), 2, false);
//	RecursiveRender(DasherModel().Root(), DasherModel().Rootmin(), DasherModel().Rootmax(), 2, true);

	// DelayDraw the text nodes
	m_DelayDraw.Draw(Screen());


	Crosshair(DasherModel().DasherOX()); // add crosshair

}

/////////////////////////////////////////////////////////////////////////////

int CDasherViewSquare::RecursiveRender(CDasherNode* Render, myint y1,myint y2,int mostleft, bool text)
{
	int Color;

	if (ColourMode==true) {
	  if (Render->Colour()!=-1) {
	    Color = Render->Colour();
	  } else {
	    if (Render->Symbol()==DasherModel().GetSpaceSymbol()) {
	      Color = 9;
	    } else if (Render->Symbol()==DasherModel().GetControlSymbol()) {
	      Color = 8;
	    } else {
	      Color = (Render->Symbol()%3)+10;
	    }
	  }
	} else {
	  Color = Render->Phase()%3; 
	}

	if ((Render->Cscheme()%2)==1 && Color<130 && ColourMode==true) { // We don't loop on high
	  Color+=130;                                // colours
	}

	if (Render->GetControlTree()!=NULL) {
	  if (RenderNode(Render->Symbol(), Color, Render->Cscheme(), y1, y2, mostleft, text,Render->GetControlTree()->text))
	    RenderGroups(Render, y1, y2, text);
	  else
	    Render->Kill();
	} 
	else 
	{
	//	if (Render->Symbol()==1)
	//	DASHER_TRACEOUTPUT("%x ",Render);
	  if (RenderNode(Render->Symbol(), Color, Render->Cscheme(), y1, y2, mostleft, text,""))
	    RenderGroups(Render, y1, y2, text);
	  else
		Render->Kill();
	}
	
	CDasherNode** const Children=Render->Children();
	if (!Children)
	  return 0;
	int norm=DasherModel().Normalization();
		for (unsigned int i=1; i<Render->ChildCount(); i++) {
		if (Children[i]->Alive()) {
			myint Range=y2-y1;
			myint newy1=y1+(Range*Children[i]->Lbnd())/norm;
			myint newy2=y1+(Range*Children[i]->Hbnd())/norm;
			RecursiveRender(Children[i], newy1, newy2, mostleft, text);
		}
	}
	return 1;


}

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::RenderGroups(CDasherNode* Render, myint y1, myint y2, bool text)
{
	CDasherNode** Children = Render->Children();
	if (!Children)
		return;
	int current=0;
	int lower=0;
	int upper=0;
    std::string Label="";

	myint range=y2-y1;
	for (unsigned int i=1; i<Render->ChildCount(); i++) {
		int g=Children[i]->Group();
		if (g!=current) {
			lower=upper;
			upper=i;
			
			if (current!=0) {
				myint lbnd=Children[lower]->Lbnd();
				myint hbnd=Children[upper]->Lbnd();
				myint newy1=y1+(range*lbnd)/DasherModel().Normalization();
				myint newy2=y1+(range*hbnd)/DasherModel().Normalization();
				int mostleft;
				if (ColourMode==true) {
					std::string Label = DasherModel().GroupLabel(current);
				  int Colour = DasherModel().GroupColour(current);
                  
                  if (Colour!=-1) {
					RenderNode(0,DasherModel().GroupColour(current),Opts::Groups,newy1,newy2,mostleft,text,Label);
				  } else {
				    RenderNode(0,(current%3)+110,Opts::Groups,newy1,newy2,mostleft,text,Label);
				  }
				} else {
					RenderNode(0,current-1,Opts::Groups,newy1,newy2,mostleft,text,Label);
				}
			}
			current=g;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

CDasherViewSquare::Cymap::Cymap(myint iScale)
{
	double dY1=0.25;    // Amount of acceleration
	double dY2=0.95;    // Accelerate Y movement below this point
	double dY3=0.05;    // Accelerate Y movement above this point

	m_Y2=myint (dY2 * iScale );
	m_Y3=myint (dY3 * iScale );
	m_Y1=myint(1.0/dY1);
}

/////////////////////////////////////////////////////////////////////////////

CDasherViewSquare::CDasherViewSquare(CDasherScreen* DasherScreen, CDasherModel& DasherModel, Dasher::Opts::ScreenOrientations Orientation, bool Colourmode)
  : CDasherView(DasherScreen, DasherModel, Orientation, Colourmode)
{
	ChangeScreen(DasherScreen);
	
	// tweak these if you know what you are doing
	m_dXmpa=0.2;   // these are for the x non-linearity
	m_dXmpb=0.5;
	m_dXmpc=0.9;
	m_dXmpd=0.5;   // slow X movement when accelerating Y


	KeyControl=false;

	onebutton=-2000;

	m_ymap = Cymap(DasherModel.DasherY());

	CDasherModel::CRange rActive(m_ymap.unmap(0), m_ymap.unmap(DasherModel.DasherY()) );
	DasherModel.SetActive(rActive);
}

/////////////////////////////////////////////////////////////////////////////

int CDasherViewSquare::RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme,
	myint y1, myint y2, int& mostleft, bool text, std::string displaytext)
{

	//		DASHER_TRACEOUTPUT("RenderNode Symbol:%d Colour:%d, ColourScheme:%d Display:%s Force:%d\n",Character,Color,ColorScheme,displaytext.c_str(),force);

	// Get the screen positions of the node in co-ords such that dasher RHS runs from 0 to DasherModel.DasherY
	screenint s1,s2;
	int iSize = dashery2screen(y1,y2,s1,s2);

	// Actual height in pixels
	int iHeight = iSize * CanvasY/DasherModel().DasherY();

	if (iHeight <=1)
		return 0;

	// horizontal width of the square is controlled by the true size (y2-y1) in Dasher world
	screenint iLeft=dasherx2screen(y2-y1);
		
	// All squares are right-aligned.
	screenint iRight=CanvasX;
		
	screenint iNewleft=iLeft, iNewtop=s1, iNewright=iRight, iNewbottom=s2;

	// Do the rotation
	MapScreen(&iNewleft, &iNewtop);
	MapScreen(&iNewright, &iNewbottom);

	Screen().DrawRectangle(iNewleft, iNewtop, iNewright, iNewbottom, Color, ColorScheme);
	
	if (iLeft<mostleft)
		iLeft=mostleft;

	int Size;
	if (iLeft*Screen().GetFontSize()<CanvasX*19/20) 
	{
		Size = 20*Screen().GetFontSize();
	} 
	else if (iLeft*Screen().GetFontSize()<CanvasX*159/160) 
	{
		Size = 14*Screen().GetFontSize();
	} 
	else 
	{
		Size = 11*Screen().GetFontSize();
	}

	screenint TextWidth, TextHeight, OriginX=0, OriginY=0;
	Screen().TextSize(Character, &TextWidth, &TextHeight, Size);
	UnMapScreen(&TextWidth, &TextHeight);
	UnMapScreen(&OriginX, &OriginY);		
	screenint FontHeight = abs(TextHeight-OriginY);		
	screenint FontWidth = abs(TextWidth-OriginX);
	mostleft = iLeft + FontWidth;

	screenint newleft2 = iLeft;
	screenint newtop2 = (iHeight-FontHeight)/2 + s1;
	screenint newright2 = iLeft + FontWidth;
	screenint newbottom2 = (iHeight+FontHeight)/2 + s1;
	MapScreen(&newleft2, &newtop2);
	MapScreen(&newright2, &newbottom2);
	iNewleft = std::min(newleft2, newright2);
	iNewtop = std::min(newtop2, newbottom2);

	//#ifdef DASHER_WIN32
	//	#if defined DrawText
	//	#undef DrawText
	//	#endif
	//#endif
	if(displaytext!= std::string("") ) 
	{
		m_DelayDraw.DelayDrawText(displaytext, iNewleft, iNewtop, Size);
	} else {
		m_DelayDraw.DelayDrawText(Character, iNewleft, iNewtop, Size);
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::CheckForNewRoot()
{
	CDasherNode * const root=DasherModel().Root();
	CDasherNode ** const children=root->Children();


	myint y1=DasherModel().Rootmin();
	myint y2=DasherModel().Rootmax();

	// This says that the root node must enclose everything visible.
	// Tiny probability characters near the bottom will cause a problem
	// with forcing to reparent to the previous one.

	if ((y1>myint(0) || y2 < DasherModel().DasherY() || dasherx2screen(y2-y1)>0)) {
		DasherModel().Reparent_root(root->Lbnd(),root->Hbnd());
		return;
	}

	if (children==0)
		return;

	int alive=0;
	int theone=0;
	unsigned int i;

	// Find whether there is exactly one alive child; if more, we don't care.
	for (i=1;i<root->ChildCount();i++) {
		if (children[i]->Alive()) {
			alive++;
			theone=i;
            if(alive>1) break; 
		}
	}

	if (alive==1) {	  
	// We must have zoomed sufficiently that only one child of the root node 
	// is still alive.  Let's make it the root.

	  y1=DasherModel().Rootmin();
	  y2=DasherModel().Rootmax();
	  myint range=y2-y1;

	  myint newy1=y1+(range*children[theone]->Lbnd())/DasherModel().Normalization();
	  myint newy2=y1+(range*children[theone]->Hbnd())/DasherModel().Normalization();
	  if (newy1<myint(0) && newy2> DasherModel().DasherY()) {
	    myint left=dasherx2screen(newy2-newy1);
	    if (left<myint(0)) {
	      DasherModel().Make_root(theone);
	      return;
	    }
	  }
	}
}

/////////////////////////////////////////////////////////////////////////////

// work out the next viewpoint
// move the rectangles accordingly
void CDasherViewSquare::TapOnDisplay(screenint mousex,screenint mousey, unsigned long Time) 
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
	  default:
	    break;
	  }
	}
	
    bool autocalibrate=1;
    if (autocalibrate) {
        AutoCalibrate(&mousex, &mousey);
    }
	myint idasherx,idashery;
	screen2dasher(mousex,mousey,&idasherx,&idashery);
	DasherModel().Tap_on_display(idasherx,idashery, Time);
	CheckForNewRoot();
}

/////////////////////////////////////////////////////////////////////////////
// move to the specified point

void CDasherViewSquare::GoTo(screenint mousex,screenint mousey) 
{
	// convert mouse (screen) coords into dasher coords
	
	UnMapScreen(&mousex, &mousey);
	myint idasherx,idashery;
	screen2dasher(mousex,mousey,&idasherx,&idashery);
	DasherModel().GoTo(idasherx,idashery);
	CheckForNewRoot();
}

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::DrawGoTo(screenint mousex, screenint mousey)
{
  // Draw a box surrounding the area of the screen that will be zoomed into
  UnMapScreen(&mousex, &mousey);
  myint idasherx,idashery;
  screen2dasher(mousex,mousey,&idasherx,&idashery);
  // So, we have a set of coordinates. We need a bunch of points back.
  myint height=DasherModel().PlotGoTo(idasherx, idashery);
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
  
/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::DrawMouse(screenint mousex, screenint mousey)
{
        if (DasherModel().Dimensions()==true || DasherModel().Eyetracker()==true) {
  
	  int Swapper;
	
	  myint dasherx,dashery;
	  screen2dasher(mousex,mousey,&dasherx,&dashery);
	  mousex=dasherx2screen(dasherx);
	  mousey=dashery2screen(dashery);
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
	  default:
	    break;
	  }
	}

	if (ColourMode==true) {
	  Screen().DrawRectangle(mousex-5, mousey-5, mousex+5, mousey+5, 2, Opts::ColorSchemes(Objects));
	} else {
	  Screen().DrawRectangle(mousex-5, mousey-5, mousex+5, mousey+5, 1, Opts::ColorSchemes(Objects));
	}
}

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::DrawMouseLine(screenint mousex, screenint mousey)
{
        if (DasherModel().Dimensions()==true || DasherModel().Eyetracker()==true) {
  
	  screenint Swapper;
	
	  myint dasherx,dashery;
	  screen2dasher(mousex,mousey,&dasherx,&dashery);
	  mousex=dasherx2screen(dasherx);
	  mousey=dashery2screen(dashery);
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
	  default:
	    break;
	  }
	}

	CDasherScreen::point mouseline[2];

	mouseline[0].x=dasherx2screen(DasherModel().DasherOX());
	mouseline[0].y=CanvasY/2;
	mouseline[1].x=mousex;
	mouseline[1].y=mousey;	  	

	if (ColourMode==true) {
	  Screen().Polyline(mouseline,2,1);
	} else {
	  Screen().Polyline(mouseline,2);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::DrawKeyboard()
{
  CDasherScreen::point line[2];
  line[0].x = 200;
  line[0].y = 0;
  line[1].x = 200;
  line[1].y = CanvasY/2;
  
  if (ColourMode==true) {
    Screen().Polyline(line,2,6);
  } else {
    Screen().Polyline(line,2);
  }

  line[0].x = 200;
  line[0].y = CanvasY/2;
  line[1].x = 0;
  line[1].y = CanvasY/2;

  if (ColourMode==true) {
    Screen().Polyline(line,2,6);
  } else {
    Screen().Polyline(line,2);
  }

  line[0].x = 200;
  line[0].y = CanvasY/2;
  line[1].x = 200;
  line[1].y = CanvasY;

  if (ColourMode==true) {
    Screen().Polyline(line,2,6);
  } else {
    Screen().Polyline(line,2);
  }
}

void CDasherViewSquare::ResetSum() 
{ 
	m_ySum=0; 
}

void CDasherViewSquare::ResetSumCounter() 
{ 
	m_ySumCounter=0; 
}

void CDasherViewSquare::ResetYAutoOffset()
{ 
	m_yAutoOffset=0; 
}


/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::ChangeScreen(CDasherScreen* NewScreen)
{
	CDasherView::ChangeScreen(NewScreen);
	screenint Width = Screen().GetWidth();
	screenint Height = Screen().GetHeight();
	CanvasX=9*Width/10;
	CanvasBorder=Width-CanvasX;
	CanvasY=Height;
}

/////////////////////////////////////////////////////////////////////////////

int CDasherViewSquare::GetAutoOffset() const
{
	 return m_yAutoOffset;
}

/////////////////////////////////////////////////////////////////////////////

int CDasherView::GetOneButton() const {
     return onebutton;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherView::SetOneButton(int Value) {
       if (onebutton < -5000) 
               onebutton=-5000;
       if (onebutton > 7000)
               onebutton=7000;
 
       onebutton += Value;
}

/////////////////////////////////////////////////////////////////////////////

