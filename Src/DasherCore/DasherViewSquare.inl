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
	int dashery=*mousey;

	double x=1.0*(CanvasX-*mousex)/CanvasX;
	dashery*=DasherModel().DasherY();
	dashery/=CanvasY;

	if (DasherModel().Dimensions()==false) {
		if (dashery>m_Y2)
			dashery= (dashery-m_Y2)*m_Y1 + m_Y2;
		else if (dashery<m_Y3)
			dashery= (dashery-m_Y3)*m_Y1+m_Y3;
	}

	x=ixmap(x)*DasherModel().DasherY();

	if (DasherModel().Dimensions()==true) {
		double distx, disty;	
		
		distx=2048-x;
		disty=DasherModel().DasherY()/2-dashery;

		if (disty>1500) {
			dashery=548+(548-dashery);
			disty=1298-dashery;
			if(disty<-750) {
				x=2048;
				dashery=2048;
			} else {
				x=2048+pow(pow(750,2)-pow(disty,2),0.5)*10;
			}
			*mousex=int(x);
			*mousey=dashery;
			return;
		}
		else if (disty <-1500) {
			dashery=3548+(3548-dashery);
			disty=2798-dashery;
			if(disty>750) {
				x=2048;
				dashery=2048;
			} else {
				x=2048+pow(pow(750,2)-pow(disty,2),0.5)*10;
			}
			*mousex=int(x);
			*mousey=dashery;
			return;
		} else {
			x=pow(pow(1500,2)-pow(disty,2),0.5);
		}
		x=2048-x;
	}
	*mousex=int(x);
	*mousey=dashery;
}

inline const int CDasherViewSquare::dasherx2screen(const myint sx)
{
	double x=1.0*sx/(DasherModel().DasherY());
	x=xmap(x);
//	return CanvasX-int(x*CanvasY);
	return CanvasX-int(x*CanvasX);

}


inline const int CDasherViewSquare::dashery2screen(myint y)
{
	if (y > m_Y2 )
		y= m_Y2 +  (y-m_Y2)/m_Y1;
	else if (y<m_Y3)
		y= m_Y3+   (y-m_Y3 )/m_Y1;

//	y*=CanvasY*Screen().GetFontSize();
	y*=CanvasY;
	y/=DasherModel().DasherY();
//	y-=(CanvasY*Screen().GetFontSize()-CanvasY)/2;
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
	if (x<m_dXmpb*m_dXmpc)
		return x/m_dXmpc;
	else
		return m_dXmpb-m_dXmpa + m_dXmpa * exp( (x/m_dXmpc - m_dXmpb) / m_dXmpa);
	
}


inline double CDasherViewSquare::xmap(double x)
// x non-linearity
{
	if (x<m_dXmpb)
		return m_dXmpc*x;
	else
		return m_dXmpc*(m_dXmpa*log((x+m_dXmpa-m_dXmpb)/m_dXmpa) +m_dXmpb);
}



} // namespace Dasher
