// Screen.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

inline Dasher::Opts::FontSize CScreen::GetFontSize() const {
  return Fontsize;
}

inline void CScreen::DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, Dasher::Opts::ColorSchemes ColorScheme, bool bDrawOutlines) const {
//      HBRUSH brush=m_Brushes[ColorScheme][Color%m_Brushes[ColorScheme].size()];
  HBRUSH brush = m_Brushes[Color];
  RECT Rect;
  Rect.left = x1;
  Rect.top = y1;
  Rect.right = x2;
  Rect.bottom = y2;
  FillRect(m_hDCBuffer, &Rect, brush);

#ifndef DASHER_WINCE
  if(bDrawOutlines) {
    FrameRect(m_hDCBuffer, &Rect, m_Brushes[3]);
  }
#endif

}

inline void CScreen::Polyline(point *Points, int Number, int iWidth, int iColour) const {
  HGDIOBJ hpOld;
  hpOld = (HPEN) SelectObject(m_hDCBuffer, (HPEN) m_Pens[iColour]);
  POINT *WinPoints = new POINT[Number];
  point2POINT(Points, WinPoints, Number);
  ::Polyline(m_hDCBuffer, WinPoints, Number);
  delete[]WinPoints;
  SelectObject(m_hDCBuffer, hpOld);
}

inline void CScreen::Polyline(point *Points, int Number, int iWidth) const {
  Polyline(Points, Number, iWidth, 0);
}

inline void CScreen::DrawPolygon(point *Points, int Number, int Color, Dasher::Opts::ColorSchemes ColorScheme) const {
  HPEN pen = (HPEN) GetStockObject(NULL_PEN);
  HPEN hpold = (HPEN) SelectObject(m_hDCBuffer, pen);
  HBRUSH hbold = (HBRUSH) SelectObject(m_hDCBuffer, m_Brushes[ColorScheme]);
  POINT *WinPoints = new POINT[Number];
  point2POINT(Points, WinPoints, Number);
  ::Polygon(m_hDCBuffer, WinPoints, Number);
  delete[]WinPoints;
  SelectObject(m_hDCBuffer, hpold);
  SelectObject(m_hDCBuffer, hbold);
}

inline void CScreen::Blank() const {
  RECT rect;
  rect.top = 0;
  rect.right = long (m_iWidth);
  rect.bottom = long (m_iHeight);
  rect.left = 0;
  FillRect(m_hDCBuffer, &rect, (HBRUSH) GetStockObject(WHITE_BRUSH));
}

inline void CScreen::Display() {
  BitBlt(m_hdc, 0, 0, m_iWidth, m_iHeight, m_hDCBuffer, 0, 0, SRCCOPY);
}

inline const void CScreen::point2POINT(const point *In, POINT *Out, int Number) const {
  // Yuck!
  for(int i = 0; i < Number; i++) {
    Out[i].x = In[i].x;
    Out[i].y = In[i].y;
  }
}
