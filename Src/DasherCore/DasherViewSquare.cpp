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

	DASHER_TRACEOUTPUT("RenderNodes\n");

	// Render nodes to screen object (should use off screen buffer)

	RecursiveRender(DasherModel().Root(), DasherModel().Rootmin(), DasherModel().Rootmax(), 2);

	// DelayDraw the text nodes
	m_DelayDraw.Draw(Screen());


	Crosshair(DasherModel().DasherOX()); // add crosshair

}

/////////////////////////////////////////////////////////////////////////////

int CDasherViewSquare::RecursiveRender(CDasherNode* Render, myint y1,myint y2,int mostleft)
{
	int Color;

  	if (ColourMode==true) 
	{
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

	if ((Render->ColorScheme()%2)==1 && Color<130 && ColourMode==true) { // We don't loop on high
	  Color+=130;                                // colours
	}

	//	DASHER_TRACEOUTPUT("%x ",Render);
	
	std::string display;
	if (Render->GetControlTree()!=NULL) 
		display = Render->GetControlTree()->text;

	if (RenderNode(Render->Symbol(), Color, Render->ColorScheme(), y1, y2, mostleft, display))
		RenderGroups(Render, y1, y2);
	else
	{
		Render->Kill();
		return 0;
	}

	int iChildCount = Render->ChildCount();
	if (!iChildCount)
	  return 0;

	int norm=DasherModel().Normalization();
	for (int i=0; i< iChildCount; i++) 
	{
		CDasherNode* pChild = Render->Children()[i];
		if ( pChild->Alive() ) 
		{
			myint Range=y2-y1;
			myint newy1=y1+(Range * pChild->Lbnd() )/norm;
			myint newy2=y1+(Range * pChild->Hbnd() )/norm;
			RecursiveRender(pChild, newy1, newy2, mostleft);
		}
	}
	return 1;


}

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::RenderGroups(CDasherNode* Render, myint y1, myint y2)
{
	CDasherNode** Children = Render->Children();
	if (!Children)
		return;
	int current=0;
	int lower=0;
	int upper=0;
    std::string Label="";

	myint range=y2-y1;
	for (int i=0; i<Render->ChildCount(); i++) {
	  //		int g=Children[i]->Group();
	  int g = 0; // FIXME - hack to make it build
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
					RenderNode(0,DasherModel().GroupColour(current),Opts::Groups,newy1,newy2,mostleft,Label);
				  } else {
				    RenderNode(0,(current%3)+110,Opts::Groups,newy1,newy2,mostleft,Label);
				  }
				} else {
					RenderNode(0,current-1,Opts::Groups,newy1,newy2,mostleft,Label);
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
	myint y1, myint y2, int& mostleft, std::string displaytext)
{

//	DASHER_TRACEOUTPUT("RenderNode Symbol:%d Colour:%d, ColourScheme:%d Display:%s \n",Character,Color,ColorScheme,displaytext.c_str());
//	DASHER_TRACEOUTPUT("RenderNode %I64 %I64",y1,y2);

	// Get the screen positions of the node in co-ords such that dasher RHS runs from 0 to DasherModel.DasherY
	screenint s1,s2;
	Cint32 iSize = dashery2screen(y1,y2,s1,s2);

	// Actual height in pixels
	Cint32 iHeight = myint(iSize * CanvasY)/DasherModel().DasherY();

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

	DASHER_TRACEOUTPUT("--------- %i %i\n",iNewtop,iNewbottom);

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
	Screen().TextSize(DasherModel().GetDisplayText(Character), &TextWidth, &TextHeight, Size);
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
		m_DelayDraw.DelayDrawText(DasherModel().GetDisplayText(Character), iNewleft, iNewtop, Size);
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

void CDasherViewSquare::screen2dasher(screenint imousex, screenint imousey, myint* idasherx, myint* idashery) const
{
    bool eyetracker=DasherModel().Eyetracker();
    // bool DasherRunning = DasherModel().Paused();
	

	imousey += int(m_yAutoOffset);

    // Maybe this mousey tweak should take place earlier, elsewhere, and 
    // have a permanent effect on mousey rather than just local.

    //SUMMARY OF  Y autocallibrate additions: 
    // If autocallibrate  {
    //    tweak mousey right now before anyone looks at it.
    //    If dasher running {
    //        Adjust our tweak estimate
    //    }
    // }
    // end summary

    // If autocallibrate  {
    //    tweak mousey right now before anyone looks at it.
    //           NOTE: yAutoOffset should be set to zero ONCE when Dasher 
    //                 first started, then left alone. In principle, if 
    //                 someone moves their Dasher window from one locn to another
    //  then it might be reasonable to re-zero the offset. But don't.

	// Convert the Y mouse coordinate to one that's based on the canvas size
	double dashery=double(imousey*DasherModel().DasherY()/CanvasY);
	bool useonebutton=0;
	if (useonebutton) {
		int onebutton = CDasherView::GetOneButton();
	    dashery=onebutton;
	}
    
	// Convert the X mouse coordinate to one that's based on the canvas size 
	// - we want this the opposite way round to the mouse coordinate system, 
	// hence the fudging. ixmap gives us the X nonlinearity.	
	double x=ixmap(1.0*(CanvasX-imousex)/CanvasX)*DasherModel().DasherY();

    // Disable one-button mode for now.
       // if (eyetracker==true) { dashery=onebutton; }
	
    // If we're in standard mode, fudge things for the vertical acceleration
	if (DasherModel().Dimensions()==false && KeyControl==false && eyetracker==false) {
		dashery = m_ymap.unmap(dashery);
		if (dashery>DasherModel().DasherY()) {
			dashery=DasherModel().DasherY();
		}
		if (dashery<0) {
			dashery=0;
		}
	}

    // The X and Y origins.
    myint dasherOX=DasherModel().DasherOX();
    //cout << "dasherOX: " << dasherOX << endl;
    myint dasherOY=DasherModel().DasherOY();
    // For Y co-ordinate changes. 
    // disty is the distance between y and centreline. 
    double disty=double(dasherOY)-dashery;                  
    //cout << "disty: " << disty << endl;

	// If we're in one-dimensional mode, make new x,y
	if (DasherModel().Dimensions()==true) {
		//if (eyetracker==true && !(x<DasherModel().DasherOX() && pow(pow(DasherModel().DasherY()/2-dashery,2)+pow(x-DasherModel().DasherOX(),2),0.5)>DasherModel().DasherY()/2.5)) {
		//	*mousex=int(x);
		//	*mousey=int(dashery);
		//	return;
		//}
      
		double disty,circlesize,yfullrange,yforwardrange,angle,ellipse_eccentricity,ybackrange,yb;	

		// The distance between the Y coordinate and the centreline in pixels
		disty=dasherOY-dashery;
        
        //		double rel_dashery=dashery+1726;
		//      double rel_dasherOY=dasherOY+1726;
		//cout << "x: " << x << endl;
		//cout << "dashery: " << rel_dashery << endl << endl;

		// The radius of the circle transcribed by the one-dimensional mapping
		circlesize=    DasherModel().DasherY()/2.5;
		yforwardrange= DasherModel().DasherY()/1.6;
		yfullrange=    yforwardrange*1.6;
		ybackrange=    yfullrange-yforwardrange;
		ellipse_eccentricity=6;
 
		if (disty>yforwardrange) {
			// If the distance between y-coord and centreline is > radius,
			// we should be going backwards, off the top.
			yb=(disty-yforwardrange)/ybackrange;
            
			if (yb>1) {
				x=0;
				dashery=double(dasherOY);
			}
			else { 
				angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));
				x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
				dashery=-(1+cos(angle))*circlesize/2+dasherOY;
			}
		}
		else if (disty <-(yforwardrange)) {
			// Backwards, off the bottom.
			yb=-(disty+yforwardrange)/ybackrange;

			if (yb>1) {
			x=0;
				dashery=double(dasherOY);
            }   
			else {
				angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));

				x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
				dashery=(1+cos(angle))*circlesize/2+dasherOY;
			}   
		}

        else {
          angle=((disty*3.14159/2)/yforwardrange);
          x=cos(angle)*circlesize;
          dashery=-sin(angle)*circlesize+dasherOY;
        }
		x=dasherOX-x;
    }
    else if (eyetracker==true) {

      myint dasherOX=DasherModel().DasherOX(); 
      //cout << "dasherOX: " << dasherOX << endl; 
      myint dasherOY=DasherModel().DasherOY(); 
         
      // X co-ordinate changes. 
      double double_x = (x/dasherOX); 
      double double_y = -((dashery-dasherOY)/(double)(dasherOY) ); 
             
      double xmax_y = xmax(double_x, double_y); 
                 
      if(double_x < xmax_y) { 
        double_x = xmax_y; 
      } 

      x = dasherOX*double_x;                 

      // Finished x-coord changes.

      double repulsionparameter=0.5;
      dashery = dasherOY - (1.0+ double_y*double_y* repulsionparameter ) * disty ;
    } 
    /* 
    // Finish the yautocallibrate
    //    If dasher running, adjust our tweak estimate
      if(!DasherRunning==true) {
        CDasherView::yFilterTimescale = 60;
        CDasherView::ySum += disty; 
        CDasherView::ySumCounter++; 
       
        CDasherView::ySigBiasPercentage=50;
        CDasherView::ySigBiasPixels = CDasherView::ySigBiasPercentage * DasherModel().DasherY() / 100;

        // FIXME: screen2dasher appears to be being called thrice per frame.
        // I don't know why.  
        CDasherView::ySigBiasPixels*=3;

        cout <<"ySum: " << CDasherView::ySum << " | ySigBiasPixels: " << CDasherView::ySigBiasPixels << " | disty: " << disty << " | yAutoOffset: " << CDasherView::yAutoOffset << endl;
  
        if (CDasherView::ySumCounter > CDasherView::yFilterTimescale) {
          CDasherView::ySumCounter = 0;

          // 'Conditions A', as specified by DJCM.  Only make the auto-offset
          // change if we're past the significance boundary.

          if (CDasherView::ySum > CDasherView::ySigBiasPixels || CDasherView::ySum < -CDasherView::ySigBiasPixels) {
             if (CDasherView::ySum > CDasherView::yFilterTimescale)
              CDasherView::yAutoOffset--; 
              else if (CDasherView::ySum < -CDasherView::yFilterTimescale)
              CDasherView::yAutoOffset++;
            CDasherView::ySum = 0;
          }
        }
    }
    */
    *idasherx=myint(x);
	*idashery=myint(dashery);
}

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::AutoCalibrate(screenint *mousex, screenint *mousey)
{
    double dashery=double(*mousey)*double(DasherModel().DasherY())/double(CanvasY);
    myint dasherOY=DasherModel().DasherOY();
    double disty=double(dasherOY)-dashery;
    bool DasherRunning = DasherModel().Paused();


    if(!DasherRunning==true) {
        m_yFilterTimescale = 20;
        m_ySum += (int)disty;
        m_ySumCounter++;

        m_ySigBiasPercentage=50;
        m_ySigBiasPixels = m_ySigBiasPercentage * DasherModel().DasherY() / 100;

        //cout << "yAutoOffset: " << CDasherView::yAutoOffset << endl;

        if (m_ySumCounter > m_yFilterTimescale) {
          m_ySumCounter = 0;

          // 'Conditions A', as specified by DJCM.  Only make the auto-offset
          // change if we're past the significance boundary.

          if (m_ySum > m_ySigBiasPixels || m_ySum < -m_ySigBiasPixels) {
             if (m_ySum > m_yFilterTimescale)
                 m_yAutoOffset--;
             else if (m_ySum < -m_yFilterTimescale)
                 m_yAutoOffset++;
            
             m_ySum = 0;
          }
        }
        
        //*mousey=int(dashery);
    }
}



/////////////////////////////////////////////////////////////////////////////

