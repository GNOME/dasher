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

// Should this using-directive really be in a header file?
using namespace Dasher;

/////////////////////////////////////////////////////////////////////////////

class CScreen:public Dasher::CDasherScreen, private NoClones {
public:

  CScreen(HDC hdc, HWND hWnd, Dasher::screenint width, Dasher::screenint height);
  ~CScreen();

  void SetInterface(CDasherInterfaceBase * DasherInterface);
  void SetColourScheme(const Dasher::CColourIO::ColourInfo *pColours);

  void SetFont(const std::string &strFont);

  void DrawMousePosBox(int which, int iMousePosDist,int layer=0);

  void TextSize(const std::string & String, Dasher::screenint * Width, Dasher::screenint * Height, int Size);

  //! Draw UTF8-encoded string String of size Size positioned at x1 and y1
  
  //void DrawString(const std::string & String, Dasher::screenint x1, Dasher::screenint y1, int Size,int layer=0);
  void DrawString(const std::string & String, Dasher::screenint x1, Dasher::screenint y1, int Size);

  //void DrawRectangle(Dasher::screenint x1, Dasher::screenint y1, Dasher::screenint x2, Dasher::screenint y2, int Color, int iOutlineColour, Dasher::Opts::ColorSchemes ColorScheme, bool bDrawOutlines, bool bFill, int iThickness,int layer=0);
  void DrawRectangle(Dasher::screenint x1, Dasher::screenint y1, Dasher::screenint x2, Dasher::screenint y2, int Color, int iOutlineColour, Dasher::Opts::ColorSchemes ColorScheme, bool bDrawOutlines, bool bFill, int iThickness);

  ///WHY is this defined like that?
  //void CScreen::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iColour, int iFillColour, int iThickness, bool bFill,int layer=0);
  void CScreen::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iColour, int iFillColour, int iThickness, bool bFill);

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
  //! \param Color colour of the polygon (numeric)
  //virtual void Polygon(point * Points, int Number, int Color);
  //virtual void Polygon(point * Points, int Number, int Color, int iWidth,int layer);
  virtual void Polygon(point * Points, int Number, int Color, int iWidth);

  //void DrawPolygon(point * Points, int Number, int Color, Dasher::Opts::ColorSchemes ColorScheme,int layer=0);
  void DrawPolygon(point * Points, int Number, int Color, Dasher::Opts::ColorSchemes ColorScheme);
  void Blank();
  void Display();

  void RealDisplay(HDC hDC, RECT r);

  void SendMarker(int iMarker);

  // TODO: These need to be implemented properly
  virtual void SetLoadBackground (bool value) {};
  virtual void SetCaptureBackground (bool value) {};

private:
  const void point2POINT(const point * In, POINT * Out, int Number);

  void TextSize_Impl(const std::string & String, Dasher::screenint * Width, Dasher::screenint * Height, int Size);

  HWND m_hWnd;

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

  struct CTextSizeInput {
    std::string m_String;
    int m_iSize;

    bool operator<(const CTextSizeInput & rhs) const {
      if(m_iSize < rhs.m_iSize)
        return true;
      if(m_iSize > rhs.m_iSize)
        return false;
      return m_String < rhs.m_String;
    } bool operator!=(const CTextSizeInput & rhs)const {
      return ((m_iSize != rhs.m_iSize) || (m_String != rhs.m_String));
  }};
  struct CTextSizeOutput {
    screenint m_iWidth;
    screenint m_iHeight;
  };

  struct hash_textsize {
    enum {
      bucket_size = 4,          // 0 < bucket_size
      min_buckets = 8           // min_buckets = 2 ^^ N, 0 < N
    };

    size_t operator() (const CTextSizeInput & x)const {
      return hash_string(x.m_String.c_str()) ^ x.m_iSize;
    } bool operator() (const CTextSizeInput & x, const CTextSizeInput & y)const {
      return (x != y);
  }};

  mutable std::map < CTextSizeInput, CTextSizeOutput > m_mapTextSize;
};

#include "Screen.inl"

#endif /* #ifndef __Screen_h__ */
