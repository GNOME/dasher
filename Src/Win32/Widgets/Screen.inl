// Screen.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

inline void CScreen::DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, int iOutlineColour, Dasher::Opts::ColorSchemes ColorScheme, bool bDrawOutlines, bool bFill, int iThickness) {
//      HBRUSH brush=m_Brushes[ColorScheme][Color%m_Brushes[ColorScheme].size()];
  HBRUSH brush = CScreen::GetBrush(Color);
  RECT Rect;
  Rect.left = x1;
  Rect.top = y1;
  Rect.right = x2;
  Rect.bottom = y2;
  if(bFill)
    FillRect(m_hDCBuffer, &Rect, brush);

#ifndef DASHER_WINCE

  if(bDrawOutlines) {

    point aPoints[5];

    aPoints[0].x=x1; aPoints[0].y=y1;
    aPoints[1].x=x2; aPoints[1].y=y1;
    aPoints[2].x=x2; aPoints[2].y=y2;
    aPoints[3].x=x1; aPoints[3].y=y2;
    aPoints[4].x=x1; aPoints[4].y=y1;

    if(iOutlineColour == -1)
     //FrameRect(m_hDCBuffer, &Rect, CScreen::GetBrush(3));
     Polyline(aPoints, 5, iThickness, 3);
    else
      //FrameRect(m_hDCBuffer, &Rect, CScreen::GetBrush(iOutlineColour));
      Polyline(aPoints, 5, iThickness, iOutlineColour);
  }
#endif

}

inline void CScreen::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iColour, int iFillColour, int iThickness, bool bFill) {
  HGDIOBJ hpOld;
  hpOld = (HPEN) SelectObject(m_hDCBuffer, GetPen(iColour, iThickness));

  if(bFill) {
    HBRUSH hBrush = CScreen::GetBrush(iFillColour);
    HBRUSH hBrushOld;
    hBrushOld = (HBRUSH)SelectObject(m_hDCBuffer, hBrush);
  
    Ellipse(m_hDCBuffer, iCX - iR, iCY - iR, iCX + iR, iCY + iR);

    SelectObject(m_hDCBuffer, hBrushOld);
  }
  // TODO: Fix this on wince
#ifndef _WIN32_WCE
  else
    Arc(m_hDCBuffer, iCX - iR, iCY - iR, iCX + iR, iCY + iR,
                     iCX, iCY - iR, iCX, iCY - iR );
#endif

  SelectObject(m_hDCBuffer, hpOld);
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

/*inline void CScreen::Polyline(point *Points, int Number, int iWidth) {
  Polyline(Points, Number, iWidth, 0);
}*/

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
 // BitBlt(m_hdc, 0, 0, m_iWidth, m_iHeight, m_hDCBuffer, 0, 0, SRCCOPY);
  InvalidateRect(m_hWnd, NULL, false);
}

inline void CScreen::RealDisplay(HDC hDC, RECT r) {
  BitBlt(hDC, r.left, r.top, r.right - r.left, r.bottom - r.top, m_hDCBuffer, r.left, r.top, SRCCOPY);
}

inline const void CScreen::point2POINT(const point *In, POINT *Out, int Number) {
  // Yuck!
  for(int i = 0; i < Number; i++) {
    Out[i].x = In[i].x;
    Out[i].y = In[i].y;
  }
}

inline HPEN& CScreen::GetPen(int iColor, int iWidth) {
  stdext::hash_map <int, HPEN> :: const_iterator hm1_RcIter;
  int key = iColor+iWidth*256;

  hm1_RcIter = m_cPens.find( key );
  if( hm1_RcIter == m_cPens.end() ) {
    HPEN pen = ::CreatePen(PS_SOLID, iWidth, RGB(m_pColours->Reds[iColor], m_pColours->Greens[iColor], m_pColours->Blues[iColor]));
    m_cPens[key] = pen;
  }
  
  return m_cPens[key];
}

inline HBRUSH& CScreen::GetBrush(int iColor) {
  stdext::hash_map <int, HBRUSH> :: const_iterator hm1_RcIter;
  int key = iColor;
  // TODO: fix this hack. Why is iColor sometimes negative (-1)?
  if(key<0)
  {
    // This hack is here to prevent unchecked subcript access with negative
    // index to the colour vectors below,
    // which causes crashes.
    m_cBrushes[key] = (HBRUSH)GetStockObject(GRAY_BRUSH);
  }
  ////////////////////////////////////////////////////////

  hm1_RcIter = m_cBrushes.find( key );
  if( hm1_RcIter == m_cBrushes.end() ) {
    HBRUSH brush = CreateSolidBrush(RGB(m_pColours->Reds[iColor], m_pColours->Greens[iColor], m_pColours->Blues[iColor]));
    m_cBrushes[key] = brush;
  }
  
  return m_cBrushes[key];
}

inline void CScreen::SetFont(const std::string &strFont) {
  if(FontName != strFont) {
    FontName = strFont;
     for(stdext::hash_map<int, HFONT>::const_iterator it(m_cFonts.begin()); it != m_cFonts.end(); ++it)
  	   DeleteObject(it->second);
    m_cFonts.clear();
  }
}

inline HFONT& CScreen::GetFont(int iSize) {
  // TODO: Reimplement
  //if(FontName != m_pDasherInterface->GetStringParameter(SP_DASHER_FONT)) {
  //  FontName = m_pDasherInterface->GetStringParameter(SP_DASHER_FONT);
  //   for(stdext::hash_map<int, HFONT>::const_iterator it(m_cFonts.begin()); it != m_cFonts.end(); ++it)
  //	       DeleteObject(it->second);
  //  m_cFonts.clear();
  //}

  if (iSize > 50) // ???? Is there a limit to size, should it be a setting?
    iSize = 50;

  stdext::hash_map <int, HFONT> :: const_iterator hm1_RcIter;
  int key = iSize;

  std::wstring wstrOutput;
  WinUTF8::UTF8string_to_wstring(FontName, wstrOutput);

  hm1_RcIter = m_cFonts.find( key );
  if( hm1_RcIter == m_cFonts.end() ) {
    HFONT font = CreateFont(int (-iSize), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, LPCWSTR(wstrOutput.c_str())); // DEFAULT_CHARSET => font made just from Size and FontName
    m_cFonts[key] = font;
  }
  
  return m_cFonts[key];
}
