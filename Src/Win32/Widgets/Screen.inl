//////////////////////////////////////////////////////////////////////
// Screen.inl: implementation of inline Screen class.functions for MS Windows
// Copyright 2002 David Ward
//////////////////////////////////////////////////////////////////////


inline void CScreen::TextSize(Dasher::symbol Character, int* Width, int* Height, int Size) const
{
	// TODO This function could be improved. The height of an "o" is returned as the
	// same as the height of an "O". Perhaps GetGlyphOutline could help.
	// Remember if it gets complicted, the height of each symbol could be pre-calculated
	
	
	if (m_DasherInterface==0)
		return;
	
	Tstring OutputText = DisplayStrings[Character];
	
	// Choose the closest font we have
	if (Size <= 11) {
		Size = 2;
	} else {
		if (Size <= 14)
			Size = 1;
		else
			Size = 0;
	}

	HFONT old = (HFONT) SelectObject(m_hDCText, m_vhfFonts[Size]);
	
	// Get the dimensions of the text in pixels
	SIZE OutSize;
	GetTextExtentPoint32(m_hDCText, OutputText.c_str(), OutputText.size(), &OutSize);
	SelectObject(m_hDCText,old);
	*Width = OutSize.cx;
	*Height = OutSize.cy;
}


inline void CScreen::DrawText(Dasher::symbol Character, int x1, int y1, int Size) const
{
	if (m_DasherInterface==0)
		return;
	
	Tstring OutputText = DisplayStrings[Character];
	
	RECT Rect;
	Rect.left = x1;
	Rect.top = y1;
	Rect.right = x1+50;
	Rect.bottom = y1+50;
	
	if (Size <= 11) {
		Size = 2;
	} else {
		if (Size <= 14)
			Size = 1;
		else
			Size = 0;
	}

	HFONT old= (HFONT) SelectObject(m_hDCText, m_vhfFonts[Size]);
	// The Windows API dumps all its function names in the global namespace, ::
	//::DrawText(m_hDCText, OutputText.c_str(), OutputText.size(), &Rect, DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE );
	::DrawText(m_hDCText, OutputText.c_str(), OutputText.size(), &Rect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE );
}


inline void CScreen::DrawRectangle(int x1, int y1, int x2, int y2, int Color, Dasher::Opts::ColorSchemes ColorScheme) const
{
	HBRUSH brush=m_Brushes[ColorScheme][Color%m_Brushes[ColorScheme].size()];
	RECT Rect;
	Rect.left = x1;
	Rect.top = y1;
	Rect.right = x2;
	Rect.bottom = y2;
	FillRect(m_hDCBuffer, &Rect, brush);
}


inline void CScreen::Polyline(point* Points, int Number) const
{
	HGDIOBJ hpOld;
	hpOld = (HPEN)SelectObject(m_hDCBuffer, (HPEN)GetStockObject(BLACK_PEN));
	POINT* WinPoints = new POINT[Number];
	point2POINT(Points, WinPoints, Number);
	::Polyline(m_hDCBuffer, WinPoints, Number);
	delete[] WinPoints;
	SelectObject(m_hDCBuffer, hpOld);
}


inline void CScreen::DrawPolygon(point* Points, int Number, int Color, Dasher::Opts::ColorSchemes ColorScheme) const
{
	HPEN pen=(HPEN)GetStockObject(NULL_PEN);
	HGDIOBJ hold;
	hold=SelectObject (m_hDCBuffer, pen);
	SelectObject (m_hDCBuffer, m_Brushes[ColorScheme][Color%m_Brushes[ColorScheme].size()]);
	POINT* WinPoints = new POINT[Number];
	point2POINT(Points, WinPoints, Number);
	::Polygon(m_hDCBuffer, WinPoints, Number);
	delete[] WinPoints;
	SelectObject (m_hDCBuffer, hold);
}


inline void CScreen::Blank() const
{
	RECT rect;
	rect.top = 0;
	rect.right = long(m_iWidth);
	rect.bottom = long(m_iHeight);
	rect.left=0;
	FillRect(m_hDCBuffer, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	FillRect(m_hDCText , &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
}


inline void CScreen::Display()
{
	BitBlt(m_hDCBuffer, 0, 0, m_iWidth,m_iHeight,m_hDCText, 0, 0, SRCAND);
	
	if (RealHDC==0) {
		RealHDC = GetDC(m_hwnd);
		BitBlt(RealHDC, 0, 0, m_iWidth,m_iHeight, m_hDCBuffer, 0, 0, SRCCOPY);
		ReleaseDC(m_hwnd,RealHDC);
	} else
		BitBlt(RealHDC, 0, 0, m_iWidth,m_iHeight, m_hDCBuffer, 0, 0, SRCCOPY);
	
	RealHDC = 0;
}


inline const void CScreen::point2POINT(const point* In, POINT* Out, int Number) const
{
	// Yuck!
	for (int i=0; i<Number; i++) {
		Out[i].x = In[i].x;
		Out[i].y = In[i].y;
	}
}