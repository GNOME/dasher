// Screen.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

inline void CScreen::DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, Dasher::Opts::ColorSchemes ColorScheme, bool bDrawOutlines) {
//      HBRUSH brush=m_Brushes[ColorScheme][Color%m_Brushes[ColorScheme].size()];
  HBRUSH brush = CScreen::GetBrush(Color);
  RECT Rect;
  Rect.left = x1;
  Rect.top = y1;
  Rect.right = x2;
  Rect.bottom = y2;
  FillRect(m_hDCBuffer, &Rect, brush);

#ifndef DASHER_WINCE
  if(bDrawOutlines) {
    FrameRect(m_hDCBuffer, &Rect, CScreen::GetBrush(3));
  }
#endif

}

inline void CScreen::Polyline(point *Points, int Number, int iWidth, int iColour) {
  HGDIOBJ hpOld;
  hpOld = (HPEN) SelectObject(m_hDCBuffer, GetPen(iColour, iWidth));
  POINT *WinPoints = new POINT[Number];
  point2POINT(Points, WinPoints, Number);
  ::Polyline(m_hDCBuffer, WinPoints, Number);
  delete[]WinPoints;
  SelectObject(m_hDCBuffer, hpOld);
}

inline void CScreen::Polyline(point *Points, int Number, int iWidth) {
  Polyline(Points, Number, iWidth, 0);
}

inline void CScreen::DrawPolygon(point *Points, int Number, int Color, Dasher::Opts::ColorSchemes ColorScheme) {
  HPEN pen = (HPEN) GetStockObject(NULL_PEN);
  HPEN hpold = (HPEN) SelectObject(m_hDCBuffer, pen);
  HBRUSH hbold = (HBRUSH) SelectObject(m_hDCBuffer, CScreen::GetBrush(ColorScheme));
  POINT *WinPoints = new POINT[Number];
  point2POINT(Points, WinPoints, Number);
  ::Polygon(m_hDCBuffer, WinPoints, Number);
  delete[]WinPoints;
  SelectObject(m_hDCBuffer, hpold);
  SelectObject(m_hDCBuffer, hbold);
}

inline void CScreen::Blank() {
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

inline const void CScreen::point2POINT(const point *In, POINT *Out, int Number) {
  // Yuck!
  for(int i = 0; i < Number; i++) {
    Out[i].x = In[i].x;
    Out[i].y = In[i].y;
  }
}

inline HPEN& CScreen::GetPen(int iColor, int iWidth) {
  HASH_MAP <int, HPEN> :: const_iterator hm1_RcIter;
  int key = iColor+iWidth*256;

  hm1_RcIter = m_cPens.find( key );
  if( hm1_RcIter == m_cPens.end() ) {
    HPEN pen = CreatePen(PS_SOLID, iWidth, RGB(m_pColours->GetRed(iColor), m_pColours->GetGreen(iColor), m_pColours->GetBlue(iColor)));
    m_cPens[key] = pen;
  }
  
  return m_cPens[key];
}

inline HBRUSH& CScreen::GetBrush(int iColor) {
  HASH_MAP <int, HBRUSH> :: const_iterator hm1_RcIter;
  int key = iColor;

  hm1_RcIter = m_cBrushes.find( key );
  if( hm1_RcIter == m_cBrushes.end() ) {
    HBRUSH brush = CreateSolidBrush(RGB(m_pColours->GetRed(iColor), m_pColours->GetGreen(iColor), m_pColours->GetBlue(iColor)));
    m_cBrushes[key] = brush;
  }
  
  return m_cBrushes[key];
}

inline HFONT& CScreen::GetFont(int iSize) {
  if(FontName != m_pDasherInterface->GetStringParameter(SP_DASHER_FONT)) {
    FontName == m_pDasherInterface->GetStringParameter(SP_DASHER_FONT);
    m_cFonts.clear();
  }

  if (iSize > 50) // ???? Is there a limit to size, should it be a setting?
    iSize = 50;

  HASH_MAP <int, HFONT> :: const_iterator hm1_RcIter;
  int key = iSize;

  hm1_RcIter = m_cFonts.find( key );
  if( hm1_RcIter == m_cFonts.end() ) {
    HFONT font = CreateFont(int (-iSize), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, LPCWSTR(FontName.c_str())); // DEFAULT_CHARSET => font made just from Size and FontName
    m_cFonts[key] = font;
  }
  
  return m_cFonts[key];
}