// Screen.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Screen_h__
#define __Screen_h__

#include "../../DasherCore/DasherScreen.h"
#include "../WinHelper.h"
#include "../WinUTF8.h"

#include "../../Common/NoClones.h"
#include "../../Common/MSVC_Unannoy.h"
#include <vector>
#ifndef _WIN32_WCE
#include <cmath>
#endif


class CScreen : public Dasher::CDasherScreen, private NoClones
{
public:
	CScreen(HWND mainwindow, int width,int height);
	~CScreen();
	
	void SetInterface(Dasher::CDasherWidgetInterface* DasherInterface);
	
	void SetFont(std::string Name);
	void SetFontSize(Dasher::Opts::FontSize size);
	Dasher::Opts::FontSize GetFontSize();
	void SetColourScheme(Dasher::CCustomColours *);
	void DrawMousePosBox(int which);
	void DrawOutlines(bool Value) {drawoutlines=Value;}
	inline void TextSize(Dasher::symbol Character, int* Width, int* Height, int Size) const;
	inline void DrawText(Dasher::symbol Character, int x1, int y1, int Size) const;
	inline void DrawText(std::string, int x1, int y1, int Size) const;
	inline void DrawRectangle(int x1, int y1, int x2, int y2, int Color, Dasher::Opts::ColorSchemes ColorScheme) const;
	
	// Draw a line of fixed colour (usually black). Intended for static UI elements such as a cross-hair
	//! Draw a line between each of the points in the array
	//
	//! \param Number the number of points in the array
	inline void Polyline(point* Points, int Number) const;
	
	// Draw a line of arbitrary colour.
	//! Draw a line between each of the points in the array
	//
	//! \param Number the number of points in the array
	//! \param Colour the colour to be drawn
	inline void Polyline(point* Points, int Number, int Colour) const;
	
	inline void DrawPolygon(point* Points, int Number, int Color, Dasher::Opts::ColorSchemes ColorScheme) const;
	inline void Blank() const;
	inline void Display();
	
	inline void SetNextHDC(HDC NewHDC) { RealHDC = NewHDC; }
private:
	void Build_Colours();
	void Free_Colours();
	
	inline const void point2POINT(const point* In, POINT* Out, int Number) const;
	inline GetDisplayTstring(Dasher::symbol Symbol);
	
	std::string m_FontName;
	
	HWND m_hwnd;
	HDC m_hDCBuffer,m_hDCText, RealHDC;
	std::vector<HFONT> m_vhfFonts;
	//vector<HBRUSH> m_vhbBrushes;
	std::vector<HBRUSH> m_Brushes;
	HBITMAP m_hbmBit,m_hbmText;
	std::vector<Tstring> DisplayStrings;
	UINT CodePage;
	Dasher::Opts::FontSize Fontsize;
	bool drawoutlines;
};


#include "Screen.inl"


#endif /* #ifndef __Screen_h__ */
