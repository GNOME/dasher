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

#include "../../Common/NoClones.h"

#include "../GDI/FontStore.h"

#include <vector>
#include <map>

#ifndef _WIN32_WCE
#include <cmath>
#endif

using namespace Dasher;

/////////////////////////////////////////////////////////////////////////////

class CScreen:public Dasher::CDasherScreen, private NoClones {
public:

  CScreen(HDC hdc, Dasher::screenint width, Dasher::screenint height);
  ~CScreen();

  void SetInterface(Dasher::CDasherWidgetInterface * DasherInterface);

  void SetFont(std::string Name);
  void SetFontSize(Dasher::Opts::FontSize size);
  Dasher::Opts::FontSize GetFontSize() const;
  void SetColourScheme(const Dasher::CCustomColours * pColours);

  void DrawMousePosBox(int which, int iMousePosDist);
  void DrawOutlines(bool Value) {
    drawoutlines = Value;
  } void TextSize(const std::string & String, Dasher::screenint * Width, Dasher::screenint * Height, int Size) const;

  //! Draw UTF8-encoded string String of size Size positioned at x1 and y1
  void DrawString(const std::string & String, Dasher::screenint x1, Dasher::screenint y1, int Size) const;

  void DrawRectangle(Dasher::screenint x1, Dasher::screenint y1, Dasher::screenint x2, Dasher::screenint y2, int Color, Dasher::Opts::ColorSchemes ColorScheme) const;

  // Draw a line of fixed colour (usually black). Intended for static UI elements such as a cross-hair
  //! Draw a line between each of the points in the array
  //
  //! \param Number the number of points in the array
  void Polyline(point * Points, int Number) const;

  // Draw a line of arbitrary colour.
  //! Draw a line between each of the points in the array
  //
  //! \param Number the number of points in the array
  //! \param Colour the colour to be drawn
  void Polyline(point * Points, int Number, int Colour) const;

  // Draw a filled polygon - given vertices and color id
  // This is not (currently) used in standard Dasher. However, it could be very
  // useful in the future. Please implement unless it will be very difficult,
  // in which case make this function call Polyline.
  //! Draw a filled polygon
  //!
  //! \param Points array of points defining the edge of the polygon
  //! \param Number number of points in the array
  //! \param Color colour of the polygon (numeric)
  virtual void Polygon(point * Points, int Number, int Color) const;

  void DrawPolygon(point * Points, int Number, int Color, Dasher::Opts::ColorSchemes ColorScheme) const;
  void Blank() const;
  void Display();

  void SendMarker(int iMarker);

private:

  const void point2POINT(const point * In, POINT * Out, int Number) const;
  inline GetDisplayTstring(Dasher::symbol Symbol);

  void TextSize_Impl(const std::string & String, Dasher::screenint * Width, Dasher::screenint * Height, int Size) const;

  std::string m_FontName;

  HDC m_hdc;
  HDC m_hDCBuffer;
  HDC m_hDCBufferBackground;
  HDC m_hDCBufferDecorations;
  std::auto_ptr < CFontStore > m_ptrFontStore;
  std::vector < HBRUSH > m_Brushes;
  std::vector < HPEN > m_Pens;
  HBITMAP m_hbmBitBackground;   // Offscreen buffer for background
  HBITMAP m_hbmBitDecorations;  // Offscreen buffer for decorations
  HGDIOBJ m_prevhbmBitBackground;
  HGDIOBJ m_prevhbmBitDecorations;
  UINT CodePage;
  Dasher::Opts::FontSize Fontsize;
  bool drawoutlines;
  int mouseposdist;

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

//      mutable stdext::hash_map< CTextSizeInput, CTextSizeOutput, hash_textsize> m_mapTextSize;
  mutable std::map < CTextSizeInput, CTextSizeOutput > m_mapTextSize;
};

#include "Screen.inl"

#endif /* #ifndef __Screen_h__ */
