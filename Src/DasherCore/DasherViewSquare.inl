// DasherViewSquare.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

namespace Dasher {

/////////////////////////////////////////////////////////////////////////////

inline double CDasherViewSquare::xmax(double x, double y) const
{
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

/////////////////////////////////////////////////////////////////////////////

inline screenint CDasherViewSquare::dasherx2screen(myint sx) const 
{
	double x=double(sx)/double(DasherModel().DasherY());
	x=xmap(x);
	return CanvasX-int(x*CanvasX);

}


/////////////////////////////////////////////////////////////////////////////

inline Cint32 CDasherViewSquare::dashery2screen(myint y1, myint y2, screenint& s1, screenint& s2) const
{
	if (KeyControl==false) 
	{
		y1=m_ymap.map(y1);
		y2=m_ymap.map(y2);
	} 

	if (y1>DasherModel().DasherY()) 
	{
		return 0;
	}
	if (y2 < 0)
	{
		return 0;
	}

	if (y1<0)  // "highest" legal coordinate to draw is 0.
	{ 
		y1=0;
	}
	
	// Is this square actually on the screen? Check bottom
	if (y2 > DasherModel().DasherY() )
		y2=DasherModel().DasherY();

	Cint32 iSize = y2-y1;
	DASHER_ASSERT(iSize>=0);

 	s1= screenint(y1 * CanvasY /DasherModel().DasherY() );
	s2= screenint(y2 * CanvasY /DasherModel().DasherY() );

	DASHER_ASSERT(s2>=s1);
	return iSize;

}


/////////////////////////////////////////////////////////////////////////////

inline screenint CDasherViewSquare::dashery2screen(myint y) const
{
	if (KeyControl==false) 
	{
		y=m_ymap.map(y);
	} 
	
	y= (y * CanvasY /DasherModel().DasherY() );

	// Stop overflow when converting to screen coords
	if (y> myint(INT_MAX))
		return INT_MAX;
	else if (y< myint(INT_MIN))
		return INT_MIN;
	return int(y);
}

/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////

inline double CDasherViewSquare::ixmap(double x) const
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

/////////////////////////////////////////////////////////////////////////////

inline double CDasherViewSquare::xmap(double x) const
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

/////////////////////////////////////////////////////////////////////////////

inline myint CDasherViewSquare::Cymap::map(myint y) const
{
	if (y > m_Y2 )
		return  m_Y2 +  (y-m_Y2)/m_Y1;
	else if (y<m_Y3)
		return m_Y3+   (y-m_Y3 )/m_Y1;
	else
		return y;
}

/////////////////////////////////////////////////////////////////////////////

inline myint CDasherViewSquare::Cymap::unmap(myint ydash) const
{
	if (ydash > m_Y2)
		return (ydash-m_Y2)*m_Y1 + m_Y2;
	else if (ydash<m_Y3)
		return (ydash-m_Y3)*m_Y1+m_Y3;
	else
 		return ydash;
}

/////////////////////////////////////////////////////////////////////////////

} // namespace Dasher




