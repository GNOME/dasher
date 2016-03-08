// Screen.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Screen_h__
#define __Screen_h__

#include "../../DasherCore/DasherScreen.h"
#include "../../DasherCore/DasherInterfaceBase.h"
#include "../../Common/NoClones.h"

#include "../GDI/FontStore.h"

#include <vector>
#include <map>
#include <unordered_map>

/////////////////////////////////////////////////////////////////////////////

class CScreen:public Dasher::CLabelListScreen, private NoClones {
public:
  //Saves a lot of typing; typedefs are equal to their declaration & do not create distinct types.
  typedef Dasher::screenint screenint;
  CScreen(HDC hdc, HWND hWnd, screenint width, screenint height, const std::string &strFont);
  ~CScreen();

  void SetColourScheme(const Dasher::CColourIO::ColourInfo *pColours) override;

  void SetFont(const std::string &strFont);

  /// Make label from UTF8-encoded string
  /// \param iWrapSize 0 => single-line label (for nodes); any other value => wrapped to screen width
  /// (we wrap the text in whatever fontsize it's DrawString/TextSize'd in, even tho we don't have to)
  CDasherScreen::Label *MakeLabel(const std::string &strText, unsigned int iWrapSize = 0) override;

  std::pair<screenint, screenint> TextSize(CDasherScreen::Label *label, unsigned int Size) override;

  //! Draw label at size Size positioned at x1 and y1
  void DrawString(CDasherScreen::Label *label, screenint x1, screenint y1, unsigned int Size, int Colour) override;

  void DrawRectangle(Dasher::screenint x1, Dasher::screenint y1, Dasher::screenint x2, Dasher::screenint y2, int Colour, int iOutlineColour, int iThickness) override;

  void DrawCircle(screenint iCX, screenint iCY, screenint iR, int iFillColour, int iLineColour, int iThickness) override;

  // Draw a line of arbitrary colour.
  //! Draw a line between each of the points in the array
  //
  //! \param Number the number of points in the array
  //! \param Colour the colour to be drawn
  void Polyline(point * Points, int Number, int iWidth, int Colour) override;

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
  virtual void Polygon(point * Points, int Number, int fillColour, int OutlineColour, int iWidth) override;

  void Display() override;

  void SendMarker(int iMarker) override;

  // Returns true if cursor is over visible part of this window.
  bool IsWindowUnderCursor() override;

  void RealDisplay(HDC hDC, RECT r);
  void resize(screenint w, screenint h);

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

  const Dasher::CColourIO::ColourInfo *m_pColours;

  // USE GET() FUNCTIONS AS THEY CACHE TO AVOID RECREATION OF PENS/BRUSHES/FONT SIZES
  HPEN& GetPen(int iColor, int iWidth);
  HBRUSH& GetBrush(int iColor);
  HFONT& GetFont(int iSize);
  std::unordered_map <int, HPEN> m_cPens;  // Holds cached pens
  std::unordered_map <int, HBRUSH> m_cBrushes; // Holds cached brushes
  std::unordered_map <int, HFONT> m_cFonts;  // Holds cached font sizes for current font
  std::string FontName; // Shouldn't need to cache, should work on events to reset font cache

  class Label : public CLabelListScreen::Label {
  public:
    const Tstring m_OutputText;
    Label(CScreen *pScreen, const std::string &strText,unsigned int iWrapSize);
    map<unsigned int,pair<screenint,screenint> > m_sizeCache;
  };

  std::pair<screenint,screenint> TextSize_Impl(CScreen::Label *label, unsigned int Size);
};

#include "Screen.inl"

#endif /* #ifndef __Screen_h__ */
