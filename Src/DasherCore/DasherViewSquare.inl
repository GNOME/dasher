// DasherViewSquare.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

namespace Dasher {

inline const void CDasherViewSquare::screen2dasher(int *mousex, int *mousey)
{
	bool eyetracker=DasherModel().Eyetracker();
	// Convert the Y mouse coordinate to one that's based on the canvas size
	double dashery=double(*mousey*DasherModel().DasherY()/CanvasY);

	// Convert the X mouse coordinate to one that's based on the canvas size 
	// - we want this the opposite way round to the mouse coordinate system, 
	// hence the fudging. ixmap gives us the X nonlinearity.	
	double x=ixmap(1.0*(CanvasX-*mousex)/CanvasX)*DasherModel().DasherY();

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

	// If we're in one-dimensional mode, we need to use the Y coordinate to 
	// generate a new and exciting X coordinate
	if (DasherModel().Dimensions()==true || eyetracker==true) {
		if (eyetracker==true && !(x<DasherModel().DasherOX() && pow(pow(DasherModel().DasherY()/2-dashery,2)+pow(x-DasherModel().DasherOX(),2),0.5)>DasherModel().DasherY()/2.5)) {
			*mousex=int(x);
			*mousey=int(dashery);
			return;
		}
      
		double disty,circlesize,yfullrange,yforwardrange,angle,ellipse_eccentricity,ybackrange,yb;	

		// The X and Y origins.
		myint dasherOX=DasherModel().DasherOX();
		myint dasherOY=DasherModel().DasherOY();
        
		// The distance between the Y coordinate and the centreline
		disty=dasherOY-dashery;
        
		double rel_dashery=dashery+1726;
		double rel_dasherOY=dasherOY+1726;
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
			// For eyetracker mode.
			if (eyetracker==true&&(disty>circlesize||disty < -(circlesize))) {

				// double x_prime = eyetracker_get_x(x, dashery);
				// double y_prime = eyetracker_get_y(x, dashery);

				double double_x = -((x-dasherOX)/dasherOX);                
				double double_y = -((rel_dashery-rel_dasherOY)/rel_dasherOY);

				//cout << "double_y: " << double_y << endl;
				//cout << "double_x: " << double_x << endl;

				double xmax_y = xmax(double_x, double_y);
              
				if(double_x < xmax_y) {
					double_x = xmax_y;
				}
                  
				disty = circlesize;

			}
		    
			else { 
				// Going forwards.
				angle=((disty*3.14159/2)/yforwardrange);
				x=cos(angle)*circlesize;
				dashery=-sin(angle)*circlesize+dasherOY;
			}
		}
		x=dasherOX-x;
	}
	*mousex=int(x);
	*mousey=int(dashery);
}

inline double CDasherViewSquare::xmax(double x, double y) {
	// DJCM -- define a function xmax(y) thus:
	// xmax(y) = a*[exp(b*y*y)-1] 
	// then:  if(x<xmax(y) [if the mouse is to the RIGHT of the line xmax(y)]
	// set x=xmax(y).  But set xmax=c if(xmax>c).
	// I would set a=1, b=1, c=16, to start with. 

	int a=1, b=1, c=16;
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







