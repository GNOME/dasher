// DasherScreen.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherScreen_h_
#define __DasherScreen_h_

#include "DasherWidgetInterface.h"
#include "DasherTypes.h"

namespace Dasher {class CDasherScreen;}
class Dasher::CDasherScreen
{
public:
	CDasherScreen(int width,int height)
		: m_iWidth(width), m_iHeight(height) {}
	
	virtual void SetInterface(CDasherWidgetInterface* DasherInterface) {m_DasherInterface = DasherInterface;}
	
	int GetWidth() const { return m_iWidth; }
	int GetHeight() const { return m_iHeight; }
	
	typedef struct tagpoint { int x; int y; } point;
	
	virtual void SetFont(std::string Name)=0;
	virtual void SetFontSize(Dasher::Opts::FontSize fontsize)=0;
	
	// DasherView asks for the width and height of the given symbol at a requested height,
	// then it is able to sensibly specify the upper left corner in DrawText.
	virtual void TextSize(symbol Character, int* Width, int* Height, int Size, bool Control) const=0;
	virtual void DrawText(symbol Character, int x1, int y1, int Size, bool Control) const=0;

	
	// Draw a filled rectangle - given position and color id
	virtual void DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const=0;
	
	// Draw a line of fixed colour (usually black). Intended for static UI elements such as a cross-hair
	virtual void Polyline(point* Points, int Number) const=0;
	
	// Draw a filled polygon - given vertices and color id
	// This is not (currently) used in standard Dasher. However, it could be very
	// useful in the future. Please implement unless it will be very difficult,
	// in which case make this function call Polyline.
	virtual void DrawPolygon(point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const=0;
	
	// Signal the screen when a frame is started and finished
	virtual void Blank() const=0;
	virtual void Display()=0;
	virtual void Pause()=0;
protected:
	const int m_iWidth, m_iHeight;
	CDasherWidgetInterface* m_DasherInterface;
};


#endif /* #ifndef __DasherScreen_h_ */
