// DasherViewSquare.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////
#include <iostream>
namespace Dasher {

inline const void CDasherViewSquare::AutoCalibrate(int *mousex, int *mousey)
{
    double dashery=double(*mousey*DasherModel().DasherY()/CanvasY);
    myint dasherOY=DasherModel().DasherOY();
    double disty=dasherOY-dashery;
    bool DasherRunning = DasherModel().Paused();


    if(!DasherRunning==true) {
        CDasherView::yFilterTimescale = 20;
        CDasherView::ySum += (int)disty;
        CDasherView::ySumCounter++;

        CDasherView::ySigBiasPercentage=50;
        CDasherView::ySigBiasPixels = CDasherView::ySigBiasPercentage * DasherModel().DasherY() / 100;

        //cout << "yAutoOffset: " << CDasherView::yAutoOffset << endl;

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
        
        //*mousey=int(dashery);
    }
}

inline const void CDasherViewSquare::screen2dasher(int *mousex, int *mousey)
{
    bool eyetracker=DasherModel().Eyetracker();
    // bool DasherRunning = DasherModel().Paused();
	

	*mousey += yAutoOffset;

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
	double dashery=double(*mousey*DasherModel().DasherY()/CanvasY);
	bool useonebutton=0;
	if (useonebutton) {
	    int onebutton = CDasherView::GetOneButton();
	    dashery=onebutton;
	}
    
	// Convert the X mouse coordinate to one that's based on the canvas size 
	// - we want this the opposite way round to the mouse coordinate system, 
	// hence the fudging. ixmap gives us the X nonlinearity.	
	double x=ixmap(1.0*(CanvasX-*mousex)/CanvasX)*DasherModel().DasherY();

    // Disable one-button mode for now.
       // if (eyetracker==true) { dashery=onebutton; }
	
    // If we're in standard mode, fudge things for the vertical acceleration
	if (DasherModel().Dimensions()==false && KeyControl==false && eyetracker==false) {
		if (dashery>m_Y2)
			dashery= (dashery-m_Y2)*m_Y1 + m_Y2;
		else if (dashery<m_Y3)
			dashery= (dashery-m_Y3)*m_Y1+m_Y3;
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
    double disty=dasherOY-dashery;                  
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
    *mousex=int(x);
	*mousey=int(dashery);
}

inline double CDasherViewSquare::xmax(double x, double y) {
	// DJCM -- define a function xmax(y) thus:
	// xmax(y) = a*[exp(b*y*y)-1] 
	// then:  if(x<xmax(y) [if the mouse is to the RIGHT of the line xmax(y)]
	// set x=xmax(y).  But set xmax=c if(xmax>c).
	// I would set a=1, b=1, c=16, to start with. 

	int a=1, b=1, c=10;
	double xmax = a*(exp(b*y*y)-1);
	//cout << "xmax = " << xmax << endl;

	if (xmax>c) 
		xmax=c;

	return xmax;
}

inline const int CDasherViewSquare::dasherx2screen(const myint sx)
{
	double x=1.0*sx/(DasherModel().DasherY());
	x=xmap(x);
	return CanvasX-int(x*CanvasX);

}


inline const int CDasherViewSquare::dashery2screen(myint y)
{
	if (KeyControl==false) {
		if (y > m_Y2 )
			y= m_Y2 +  (y-m_Y2)/m_Y1;
		else if (y<m_Y3)
			y= m_Y3+   (y-m_Y3 )/m_Y1;
	} 
	y*=CanvasY;
	y/=DasherModel().DasherY();
	return int(y);
}


inline void CDasherViewSquare::Crosshair(myint sx)
{
	CDasherScreen::point crosshair[2];
	
	crosshair[0].x = dasherx2screen(sx);
	crosshair[0].y = 0;
	crosshair[1].x = crosshair[0].x;
	crosshair[1].y = CanvasY;
	
	MapScreen(&crosshair[0].x, &crosshair[0].y);
	MapScreen(&crosshair[1].x, &crosshair[1].y);
	if (ColourMode==true) {
		Screen().Polyline(crosshair,2,5);
	} else {
		Screen().Polyline(crosshair,2);
	}
	
	crosshair[0].x = dasherx2screen(12*sx/14);
	crosshair[0].y = CanvasY/2;
	crosshair[1].x = dasherx2screen(17*sx/14);
	crosshair[1].y = CanvasY/2;
	
	MapScreen(&crosshair[0].x, &crosshair[0].y);
	MapScreen(&crosshair[1].x, &crosshair[1].y);
	if (ColourMode==true) {
		Screen().Polyline(crosshair,2,5);
	} else {
		Screen().Polyline(crosshair,2);
	}
}





inline double CDasherViewSquare::ixmap(double x)
// invert x non-linearity
{
	if (KeyControl==false) {
		if (x<m_dXmpb*m_dXmpc)	
			return x/m_dXmpc;
		else	
			return m_dXmpb-m_dXmpa + m_dXmpa * exp( (x/m_dXmpc - m_dXmpb) / m_dXmpa);
	} else {
		return x;
	}
}


inline double CDasherViewSquare::xmap(double x)
// x non-linearity
{
	if (KeyControl==false) {
		if (x<m_dXmpb)
			return m_dXmpc*x;
		else
			return m_dXmpc*(m_dXmpa*log((x+m_dXmpa-m_dXmpb)/m_dXmpa) +m_dXmpb);
	} else {
		return x;
	}
}

} // namespace Dasher







