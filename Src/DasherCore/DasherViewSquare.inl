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
	// Convert the Y mouse coordinate to one that's based on the canvas size
	double dashery=double(*mousey*DasherModel().DasherY()/CanvasY);
	
	// Convert the X mouse coordinate to one that's based on the canvas size - we want this
	// the opposite way round to the mouse coordinate system, hence the fudging. ixmap gives
	// us the X nonlinearity.	
	double x=ixmap(1.0*(CanvasX-*mousex)/CanvasX)*DasherModel().DasherY();

	// If we're in standard mode, fudge things for the vertical acceleration
	if (DasherModel().Dimensions()==false && KeyControl==false) {
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

	// If we're in one-dimensional mode, we need to use the Y coordinate to generate a new
	// and exciting X coordinate
	if (DasherModel().Dimensions()==true) {
		double disty,circlesize;	

		// The x coordinate of the crosshairs
		myint dasherOX=DasherModel().DasherOX();

		// Make needed Y less than DasherY
		// dashery=2*dashery-DasherModel().DasherY()/2;
		
		// The distance between the Y coordinate and the centreline
		disty=DasherModel().DasherY()/2-dashery;


		// This is the radius of the circle transcribed by the one-dimensional mapping
		circlesize=DasherModel().DasherY()/2.5;

		if (disty>circlesize) {
			dashery=2*(dasherOX-circlesize)-dashery;
			disty=dasherOX-(circlesize/2)-dashery;
			if(disty<-(circlesize/2)) {
				x=double(dasherOX);
				dashery=double(DasherModel().DasherY()/2);
			} else {
				x=dasherOX+pow(pow(circlesize/2,2)-pow(disty,2),0.5)*10;
			}
			*mousex=int(x);
			*mousey=int(dashery);
			return;
		}
		else if (disty <-(circlesize)) {
			dashery=2*(dasherOX+circlesize)-dashery;
			disty=dasherOX+circlesize/2-dashery;			
			if(disty>circlesize/2) {
				x=double(dasherOX);
				dashery=double(DasherModel().DasherY()/2);
			} else {
				x=dasherOX+pow(pow(circlesize/2,2)-pow(disty,2),0.5)*10;
			}
			*mousex=int(x);
			*mousey=int(dashery);
			return;
		} else {
			x=pow(pow(circlesize,2)-pow(disty,2),0.5);
		}
		x=dasherOX-x;
	}
	*mousex=int(x);
	*mousey=int(dashery);
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
	Screen().Polyline(crosshair,2);
	
	crosshair[0].x = dasherx2screen(12*sx/14);
	crosshair[0].y = CanvasY/2;
	crosshair[1].x = dasherx2screen(17*sx/14);
	crosshair[1].y = CanvasY/2;
	
	MapScreen(&crosshair[0].x, &crosshair[0].y);
	MapScreen(&crosshair[1].x, &crosshair[1].y);
	Screen().Polyline(crosshair,2);
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







