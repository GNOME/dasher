// DasherViewSquare.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

namespace Dasher {


inline const myint CDasherViewSquare::screen2dasherx(const int mousex)
{
	//	double x=1.0*(CanvasX-mousex)/CanvasY;
		double x=1.0*(CanvasX-mousex)/CanvasX;
	x=ixmap(x);
	return int ( x* DasherModel().DasherY()) ;
}


inline const myint CDasherViewSquare::screen2dashery(int screeny) 
{
	myint dashery=screeny;
	dashery*=DasherModel().DasherY();
	dashery/=CanvasY;

	if (dashery>m_Y2)
		dashery= (dashery-m_Y2)*m_Y1 + m_Y2;
	else if (dashery<m_Y3)
		dashery= (dashery-m_Y3)*m_Y1+m_Y3;

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
