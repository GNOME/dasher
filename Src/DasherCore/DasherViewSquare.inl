// DasherViewSquare.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

namespace Dasher {


inline const myint CDasherViewSquare::screen2dasherx(const int mousex, const int screeny)
{
	//	double x=1.0*(CanvasX-mousex)/CanvasY;
		double x=1.0*(CanvasX-mousex)/CanvasX;
	myint dashery=screen2dashery(screeny);
	x=ixmap(x)*DasherModel().DasherY();

	if (DasherModel().Dimensions()==1) {
		double distx, disty;	
		
		distx=2048-x;
		disty=dashery-DasherModel().DasherY()/2;

		if(disty>=1500 || disty<-1500) {
			x=-(pow(pow(disty,2)-pow(1500,2),0.5));
		} else {
			x=pow(pow(1500,2)-pow(disty,2),0.5);
		}
		
		return int(2048-x);
	} else {
		if (dashery>m_Y2) {	// Slow X expansion if Y is accelerated
		}
		else if (dashery<m_Y3) { // Ditto
		}
	}
	return int (x);
}


inline const myint CDasherViewSquare::screen2dashery(int screeny) 
{
	if (DasherModel().Dimensions()==2) {
		if (screeny>s_Y2)
			screeny= (screeny-s_Y2)*m_Y1 + s_Y2;
		else if (screeny<s_Y3)
			screeny= (screeny-s_Y3)*m_Y1+s_Y3;
	}
	myint dashery=screeny;
	dashery+=(CanvasY*Screen().GetFontSize()-CanvasY)/2;
	dashery*=DasherModel().DasherY();
	dashery/=CanvasY*Screen().GetFontSize();

	return dashery;
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

	y*=CanvasY*Screen().GetFontSize();
	y/=DasherModel().DasherY();
	y-=(CanvasY*Screen().GetFontSize()-CanvasY)/2;
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
