// Screen.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Screen_h__
#define __Screen_h__


#include "../WinHelper.h"
#include "../WinUTF8.h"
#include "../../DasherCore/DasherScreen.h"

#include "../../Common/NoClones.h"

#include "../GDI/FontStore.h"

#include <vector>
#ifndef _WIN32_WCE
#include <cmath>
#endif

using namespace Dasher;

/////////////////////////////////////////////////////////////////////////////

class CScreen : public Dasher::CDasherScreen, private NoClones
{
public:

	CScreen(HDC hdc, Dasher::screenint width,Dasher::screenint height);
	~CScreen();
	
	void SetInterface(Dasher::CDasherWidgetInterface* DasherInterface);
	
	void SetFont(std::string Name);
	void SetFontSize(Dasher::Opts::FontSize size);
	Dasher::Opts::FontSize GetFontSize() const;
	void SetColourScheme(Dasher::CCustomColours *);
	void DrawMousePosBox(int which);
	void SetMousePosDist(int y) {mouseposdist=y;}
	int GetMousePosDist() {return mouseposdist;}
	void DrawOutlines(bool Value) {drawoutlines=Value;}
	
	void TextSize(Dasher::symbol Character, Dasher::screenint* Width, Dasher::screenint* Height, int Size) const;
	void DrawText(Dasher::symbol Character, Dasher::screenint x1, Dasher::screenint y1, int Size) const;
	void DrawText(std::string, Dasher::screenint x1, Dasher::screenint y1, int Size) const;
	void DrawRectangle(Dasher::screenint x1, Dasher::screenint y1, Dasher::screenint x2, Dasher::screenint y2, int Color, Dasher::Opts::ColorSchemes ColorScheme) const;
	
	// Draw a line of fixed colour (usually black). Intended for static UI elements such as a cross-hair
	//! Draw a line between each of the points in the array
	//
	//! \param Number the number of points in the array
	void Polyline(point* Points, int Number) const;
	
	// Draw a line of arbitrary colour.
	//! Draw a line between each of the points in the array
	//
	//! \param Number the number of points in the array
	//! \param Colour the colour to be drawn
	void Polyline(point* Points, int Number, int Colour) const;
	
	void DrawPolygon(point* Points, int Number, int Color, Dasher::Opts::ColorSchemes ColorScheme) const;
	void Blank() const;
	void Display();
	
private:
	
	const void point2POINT(const point* In, POINT* Out, int Number) const;
	inline GetDisplayTstring(Dasher::symbol Symbol);
	
	std::string m_FontName;
	
//	HWND m_hwnd;
	HDC m_hdc;
	HDC m_hDCBuffer,m_hDCText;
	std::auto_ptr<CFontStore> m_ptrFontStore;
	//std::vector<HFONT> m_vhfFonts;
	//vector<HBRUSH> m_vhbBrushes;
	std::vector<HBRUSH> m_Brushes;
	std::vector<HPEN> m_Pens;
	HBITMAP m_hbmBit,m_hbmText;
	HGDIOBJ m_prevhbmBit,m_prevhbmText;
	std::vector<Tstring> DisplayStrings;
	UINT CodePage;
	Dasher::Opts::FontSize Fontsize;
	bool drawoutlines;
	int mouseposdist;
};


#include "Screen.inl"


#endif /* #ifndef __Screen_h__ */
