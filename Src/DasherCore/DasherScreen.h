//! Class definition for the Dasher screen
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
        //! \param width Width of the screen
        //! \param height Height of the screen
	CDasherScreen(int width,int height)
		: m_iWidth(width), m_iHeight(height) {}
	
	//! Set the widget interface used for communication with the core
	virtual void SetInterface(CDasherWidgetInterface* DasherInterface) {m_DasherInterface = DasherInterface;}
	
	//! Return the width of the screen
	int GetWidth() const { return m_iWidth; }

	//! Return the height of the screen
	int GetHeight() const { return m_iHeight; }

	//! Structure defining a point on the screen
	typedef struct tagpoint { int x; int y; } point;

	//! Set the Dasher font (ie, the screen font) to Name
	//!
	//! This is the font used to render letters in the main screen
	virtual void SetFont(std::string Name)=0;

	//! Set the Dasher font to Normal, Big or VBig
	virtual void SetFontSize(Dasher::Opts::FontSize fontsize)=0;
	
	//! Return the Dasher font size
	virtual Dasher::Opts::FontSize GetFontSize()=0;

	// DasherView asks for the width and height of the given symbol at a requested height,
	// then it is able to sensibly specify the upper left corner in DrawText.
	//! Set Width and Height to those of the character Character at size Size
	virtual void TextSize(symbol Character, int* Width, int* Height, int Size) const=0;
	//! Draw character Character of size Size positioned at x1 and y1
	virtual void DrawText(symbol Character, int x1, int y1, int Size) const=0;

	
	// Draw a filled rectangle - given position and color id
	//! Draw a filled rectangle
	//
	//! Draw a coloured rectangle on the screen
	//! \param x1 top left of rectangle (x coordinate)
	//! \param y1 top left corner of rectangle (y coordinate)
	//! \param x2 bottom right of rectangle (x coordinate)
	//! \param y2 bottom right of rectangle (y coordinate)
	//! \param Color the colour to be used (numeric)
	//! \param ColorScheme Which colourscheme is to be used
	virtual void DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const=0;
	
	// Draw a line of fixed colour (usually black). Intended for static UI elements such as a cross-hair
	//! Draw a line between each of the points in the array
	//
	//! \param Number the number of points in the array
	virtual void Polyline(point* Points, int Number) const=0;
	
	// Draw a filled polygon - given vertices and color id
	// This is not (currently) used in standard Dasher. However, it could be very
	// useful in the future. Please implement unless it will be very difficult,
	// in which case make this function call Polyline.
	//! Draw a filled polygon
	//
	//! \param Points array of points defining the edge of the polygon
	//! \param Number number of points in the array
	//! \param Color colour of the polygon (numeric)
	//! \param Colorscheme Which colourscheme is to be used
	virtual void DrawPolygon(point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const=0;
	
	// Signal the screen when a frame is started and finished
	//! Signal that a frame is being started
	virtual void Blank() const=0;

	//! Signal that a frame is finished - the screen should be updated
	virtual void Display()=0;

protected:
	//! Width and height of the screen
	const int m_iWidth, m_iHeight;

	//! Pointer to a widget interface for communication with the core
	CDasherWidgetInterface* m_DasherInterface;
};


#endif /* #ifndef __DasherScreen_h_ */
