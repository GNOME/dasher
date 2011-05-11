// Screen.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Screen_h__
#define __Screen_h__

#include "../../Common/Hash.h"

#include "../../DasherCore/DasherScreen.h"
#include "../../DasherCore/DasherInterfaceBase.h"
#include "../../Common/NoClones.h"

#include "../GDI/FontStore.h"

#include <vector>
#include <map>
#include <hash_map>

#ifndef _WIN32_WCE
#include <cmath>
#endif

/////////////////////////////////////////////////////////////////////////////

class CScreen:public Dasher::CLabelListScreen, private NoClones {
public:
  //Saves a lot of typing; typedefs are equal to their declaration & do not create distinct types.
  typedef Dasher::screenint screenint;
  CScreen(HDC hdc, HWND hWnd, screenint width, screenint height);
  ~CScreen();

  void SetColourScheme(const Dasher::CColourIO::ColourInfo *pColours);

  void SetFont(const std::string &strFont);

  void DrawMousePosBox(int which, int iMousePosDist,int layer=0);

  //! Make label from UTF8-encoded string
  CDasherScreen::Label *MakeLabel(const std::string &strText);

  std::pair<screenint,screenint> TextSize(CDasherScreen::Label *label, unsigned int Size);

  //! Draw label at size Size positioned at x1 and y1
  void DrawString(CDasherScreen::Label *label, screenint x1, screenint y1, unsigned int Size, int Colour);

  void DrawRectangle(Dasher::screenint x1, Dasher::screenint y1, Dasher::screenint x2, Dasher::screenint y2, int Colour, int iOutlineColour, int iThickness);

  void CScreen::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iFillColour, int iLineColour, int iThickness);

  // Draw a line of fixed colour (usually black). Intended for static UI elements such as a cross-hair
  //! Draw a line between each of the points in the array
  //
  //! \param Number the number of points in the array
  //void Polyline(point * Points, int Number, int iWidth);

  // Draw a line of arbitrary colour.
  //! Draw a line between each of the points in the array
  //
  //! \param Number the number of points in the array
  //! \param Colour the colour to be drawn
  //void Polyline(point * Points, int Number, int iWidth, int Colour,int layer=0);
  void Polyline(point * Points, int Number, int iWidth, int Colour);

  // Draw a filled polygon - given vertices and color id
  // This is not (currently) used in standard Dasher. However, it could be very
  // useful in the future. Please implement unless it will be very difficult,
  // in which case make this function call Polyline.
  //! Draw a filled polygon
  //!
  //! \param Points array of points defining the edge of the polygon
  //! \param Number number of points in the array
  //! \param fillColour colour to fill polygon (numeric); -1 for don't fill
  //! \param outlineColour colour to draw polygon outline (right the way around, i.e. repeating first point)
   //virtual void Polygon(point * Points, int Number, int Color);
  //virtual void Polygon(point * Points, int Number, int Color, int iWidth,int layer);
  virtual void Polygon(point * Points, int Number, int fillColour, int OutlineColour, int iWidth);

  void Blank();
  void Display();

  void RealDisplay(HDC hDC, RECT r);

  void SendMarker(int iMarker);

  void resize(screenint w,screenint h);
private:
  const void point2POINT(const point * In, POINT * Out, int Number);

  HWND m_hWnd;

  void DeleteBuffers();
  void CreateBuffers();
  HDC m_hdc;
  HDC m_hDCBuffer;
  HDC m_hDCBufferBackground;
  HDC m_hDCBufferDecorations;
  HBITMAP m_hbmBitBackground;   // Offscreen buffer for background
  HBITMAP m_hbmBitDecorations;  // Offscreen buffer for decorations
  HGDIOBJ m_prevhbmBitBackground;
  HGDIOBJ m_prevhbmBitDecorations;
  UINT CodePage;

  const Dasher::CColourIO::ColourInfo *m_pColours;

  // USE GET() FUNCTIONS AS THEY CACHE TO AVOID RECREATION OF PENS/BRUSHES/FONT SIZES
  HPEN& GetPen(int iColor, int iWidth);
  HBRUSH& GetBrush(int iColor);
  HFONT& GetFont(int iSize);
  stdext::hash_map <int, HPEN> m_cPens;  // Holds cached pens
  stdext::hash_map <int, HBRUSH> m_cBrushes; // Holds cached brushes
  stdext::hash_map <int, HFONT> m_cFonts;  // Holds cached font sizes for current font
  std::string FontName; // Shouldn't need to cache, should work on events to reset font cache

  class Label : public CLabelListScreen::Label {
  public:
    const Tstring m_OutputText;
    Label(CScreen *pScreen, const std::string &strText);
    map<unsigned int,pair<screenint,screenint> > m_sizeCache;
  };

  std::pair<screenint,screenint> TextSize_Impl(CScreen::Label *label, unsigned int Size);
};

#include "Screen.inl"

#endif /* #ifndef __Screen_h__ */
