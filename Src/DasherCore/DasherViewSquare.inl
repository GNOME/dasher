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
	return int ( x*(1<<DasherModel().Shift()) );
}


inline const myint CDasherViewSquare::screen2dashery(const int mousey) 
{
	double y=1.0*mousey/CanvasY;
	y=iymap(y);
	return int(y*(1<<DasherModel().Shift()));
}


inline const int CDasherViewSquare::dasherx2screen(const myint sx)
{
	double x=1.0*sx/(1<<DasherModel().Shift());
	x=xmap(x);
//	return CanvasX-int(x*CanvasY);
	return CanvasX-int(x*CanvasX);

}


inline const int CDasherViewSquare::dashery2screen(const myint sy)
{
	//double 
	double y=1.0*sy/(1<<DasherModel().Shift());
	y=ymap(y);
	return int(y*CanvasY);
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


} // namespace Dasher