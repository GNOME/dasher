// Screen.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "Screen.h"

#include "../Common/DasherEncodingToCP.h"
#include "../../DasherCore/CustomColours.h"
using namespace WinLocalisation;
using namespace Dasher;
using namespace Opts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////

CScreen::CScreen(HDC hdc, Dasher::screenint iWidth, Dasher::screenint iHeight)
:CDasherScreen(iWidth, iHeight), m_hdc(hdc), m_FontName(""), Fontsize(Dasher::Opts::FontSize(1)), m_ptrFontStore(new CFontStore(TEXT(""))) {
  // set up the off-screen buffers
//      HDC hdc = GetDC(mainwindow);

  // Create a memory device context compatible with the screen
  m_hDCBufferBackground = CreateCompatibleDC(hdc);      // one for rectangles
  m_hbmBitBackground = CreateCompatibleBitmap(hdc, m_iWidth, m_iHeight);
  SetBkMode(m_hDCBufferBackground, TRANSPARENT);
  m_prevhbmBitBackground = SelectObject(m_hDCBufferBackground, m_hbmBitBackground);

  m_hDCBufferDecorations = CreateCompatibleDC(hdc);
  m_hbmBitDecorations = CreateCompatibleBitmap(hdc, m_iWidth, m_iHeight);
  SetBkMode(m_hDCBufferDecorations, TRANSPARENT);
  m_prevhbmBitDecorations = SelectObject(m_hDCBufferDecorations, m_hbmBitDecorations);

  // FIXME - I'm assuming it's okay just to copy handles like this, can someone do something about this if I'm wrong

  m_hDCBuffer = m_hDCBufferBackground;

  // create the brushes
//      Build_Colours();

  CodePage = GetUserCodePage();
  SetFont("");

//      m_hDCScreen = ::GetDC(m_hwnd);
//      TCHAR debug[256];
//      _stprintf(debug, TEXT("GetDC: hwnd %x hdc %x\n"), m_hwnd, m_hDCScreen);
//      OutputDebugString(debug); 
}

/////////////////////////////////////////////////////////////////////////////

CScreen::~CScreen() {
  // tidy up

  // Select the old bitmaps back into the device contexts (is this really
  // necessary? we're just about to release them?)

  SelectObject(m_hDCBufferDecorations, m_prevhbmBitDecorations);
  SelectObject(m_hDCBufferBackground, m_prevhbmBitBackground);

  // Delete the offscreen bitmaps and device contexts.

  BOOL b;

  b = DeleteObject(m_hbmBitBackground);
  DASHER_ASSERT(b);

  b = DeleteObject(m_hbmBitDecorations);
  DASHER_ASSERT(b);

  b = DeleteDC(m_hDCBufferBackground);
  DASHER_ASSERT(b);

  b = DeleteDC(m_hDCBufferDecorations);
  DASHER_ASSERT(b);

  while(m_Brushes.size() != 0) {
    DeleteObject(m_Brushes.back());
    m_Brushes.pop_back();
  }

  while(m_Pens.size() != 0) {
    DeleteObject(m_Pens.back());
    m_Pens.pop_back();
  }
}

/////////////////////////////////////////////////////////////////////////////

void CScreen::SetInterface(CDasherInterface *DasherInterface) {
  DASHER_ASSERT_VALIDPTR_RW(DasherInterface);

  CDasherScreen::SetInterface(DasherInterface);

  CodePage = EncodingToCP(m_pDasherInterface->GetAlphabetType());
  SetFont(m_FontName);

}

/////////////////////////////////////////////////////////////////////////////

void CScreen::SetFont(string Name) {
  m_FontName = Name;

  Tstring FontName;
  WinUTF8::UTF8string_to_wstring(Name, FontName);

  // damn EVC4 doesn't have a reset
  std::auto_ptr < CFontStore > ptrNewStore(new CFontStore(FontName));
  m_ptrFontStore = ptrNewStore;

}

/////////////////////////////////////////////////////////////////////////////

void CScreen::SetFontSize(FontSize size) {
  Fontsize = size;
  SetFont(m_FontName);
}

/////////////////////////////////////////////////////////////////////////////

void CScreen::SetColourScheme(const Dasher::CCustomColours *pColours) {
  // DJW - must delete brushes ala free_colours. Would be nice to encapsuted this into a Brush Container
  while(m_Brushes.size() != 0) {
    DeleteObject(m_Brushes.back());
    DeleteObject(m_Pens.back());
    m_Brushes.pop_back();
    m_Pens.pop_back();
  }

  DASHER_ASSERT(pColours);
  int numcolours = pColours->GetNumColours();

  DASHER_ASSERT(numcolours > 0);

  for(int i = 0; i < numcolours; i++) {
    // DJW 20031029 - something fishy is going on - i think calls to CreateSolidBrush start to fail
    HBRUSH hb = CreateSolidBrush(RGB(pColours->GetRed(i), pColours->GetGreen(i), pColours->GetBlue(i)));
    DASHER_ASSERT(hb != 0);
    m_Brushes.push_back(hb);
    HPEN hp = CreatePen(PS_SOLID, 1, RGB(pColours->GetRed(i), pColours->GetGreen(i), pColours->GetBlue(i)));
    DASHER_ASSERT(hp != 0);
    m_Pens.push_back(hp);
  }
}

//void CScreen::Build_Colours ()
//{
//      m_Brushes.resize(6);

        // Currently white and gray. Intended for use by a space character, placed last in alphabet
//      m_Brushes[Special1].push_back(CreateSolidBrush(RGB(240,240,240))); // making lighter for djcm
//      m_Brushes[Special2].push_back(CreateSolidBrush(RGB(255,255,255)));

//      m_Brushes[Objects].push_back(CreateSolidBrush(RGB(0,0,0)));

//      m_Brushes[Groups].push_back(CreateSolidBrush(RGB(255,255,0)));
//      m_Brushes[Groups].push_back(CreateSolidBrush(RGB(0,255,0)));
//      m_Brushes[Groups].push_back(CreateSolidBrush(RGB(255,100,100)));

//      m_Brushes[Nodes1].push_back(CreateSolidBrush(RGB(180,245,180)));
//      m_Brushes[Nodes1].push_back(CreateSolidBrush(RGB(160,200,160)));
//      m_Brushes[Nodes1].push_back(CreateSolidBrush(RGB(0,255,255)));

//      m_Brushes[Nodes2].push_back(CreateSolidBrush(RGB(255,185,255)));
//      m_Brushes[Nodes2].push_back(CreateSolidBrush(RGB(140,200,255)));
//      m_Brushes[Nodes2].push_back(CreateSolidBrush(RGB(255,175,175)));
//}

//void CScreen::Free_Colours ()
//{
        // tidy up
//      while (m_Brushes.size()) {
//              while (m_Brushes.back().size()) {
//                      DeleteObject(m_Brushes.back().back());
//                      m_Brushes.back().pop_back();
//              }
//              m_Brushes.pop_back();
//      }
//}

/////////////////////////////////////////////////////////////////////////////

/// Handler for redraw markers;

void CScreen::SendMarker(int iMarker) {
  switch (iMarker) {
  case 0:
    m_hDCBuffer = m_hDCBufferBackground;
    break;
  case 1:
    BitBlt(m_hDCBufferDecorations, 0, 0, m_iWidth, m_iHeight, m_hDCBufferBackground, 0, 0, SRCCOPY);
    m_hDCBuffer = m_hDCBufferDecorations;
    break;
  }
}

void CScreen::DrawMousePosBox(int which, int iMousePosDist) {

//      HBRUSH brush=m_Brushes[ColorScheme][Color%m_Brushes[ColorScheme].size()];
  RECT Rect;
  HBRUSH brush;
  switch (which) {
  case 0:
    Rect.left = 0;
    Rect.top = m_iHeight / 2 - iMousePosDist - 50;
    Rect.right = m_iWidth;
    Rect.bottom = Rect.top + 100;
    brush = CreateSolidBrush(RGB(255, 0, 0));
    break;
  case 1:
    Rect.left = 0;
    Rect.bottom = m_iHeight / 2 + iMousePosDist + 50;
    Rect.right = m_iWidth;
    Rect.top = Rect.bottom - 100;
    brush = CreateSolidBrush(RGB(255, 255, 0));
    break;
  default:
    DASHER_ASSERT(0);
  }
  FillRect(m_hDCBuffer, &Rect, brush);
  DeleteObject(brush);
  Display();
}

/*
void CScreen::DrawText(Dasher::symbol Character, screenint x1, screenint y1, int iSize) const
{
	if (m_DasherInterface==0)
		return;
	
	Tstring OutputText = DisplayStrings[Character];
	
	RECT Rect;
	Rect.left = x1;
	Rect.top = y1;
	Rect.right = x1+50;
	Rect.bottom = y1+50;

	HFONT old= (HFONT) SelectObject(m_hDCBuffer, m_ptrFontStore->GetFont(iSize));

	// The Windows API dumps all its function names in the global namespace, ::
	::DrawText(m_hDCBuffer, OutputText.c_str(), OutputText.size(), &Rect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE );

	// DJW - need to select the old object back into the DC
	SelectObject(m_hDCBuffer, old);
}
*/

void CScreen::DrawString(const std::string &OutputString, Dasher::screenint x1, Dasher::screenint y1, int iSize) const {

  Tstring OutputText;

  WinUTF8::UTF8string_to_wstring(OutputString, OutputText);

  RECT Rect;
  Rect.left = x1;
  Rect.top = y1;
  Rect.right = x1 + 50;
  Rect.bottom = y1 + 50;

  HFONT old = (HFONT) SelectObject(m_hDCBuffer, m_ptrFontStore->GetFont(iSize));

  // The Windows API dumps all its function names in the global namespace, ::
  ::DrawText(m_hDCBuffer, OutputText.c_str(), OutputText.size(), &Rect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE);

  SelectObject(m_hDCBuffer, old);
}

/////////////////////////////////////////////////////////////////////////////

void CScreen::TextSize(const std::string &String, screenint *Width, screenint *Height, int iSize) const {
  CTextSizeInput in;
  in.m_String = String;
  in.m_iSize = iSize;

//      stdext::hash_map< CTextSizeInput, CTextSizeOutput, hash_textsize>::const_iterator it;
  std::map < CTextSizeInput, CTextSizeOutput >::const_iterator it;
  it = m_mapTextSize.find(in);

  if(it == m_mapTextSize.end()) {
    CTextSizeOutput out;
    TextSize_Impl(String, &out.m_iWidth, &out.m_iHeight, iSize);
//              m_mapTextSize.insert( stdext::hash_map< CTextSizeInput, CTextSizeOutput, hash_textsize>::value_type( in,out) );
    m_mapTextSize.insert(std::map < CTextSizeInput, CTextSizeOutput >::value_type(in, out));
    *Width = out.m_iWidth;
    *Height = out.m_iHeight;
    return;
  }

  const CTextSizeOutput & out = it->second;
  *Width = out.m_iWidth;
  *Height = out.m_iHeight;
}

/////////////////////////////////////////////////////////////////////////////

void CScreen::TextSize_Impl(const std::string &String, screenint *Width, screenint *Height, int iSize) const {
  // TODO This function could be improved. The height of an "o" is returned as the
  // same as the height of an "O". Perhaps GetGlyphOutline could help.
  // Remember if it gets complicted, the height of each symbol could be pre-calculated

  wstring OutputText;
  WinUTF8::UTF8string_to_wstring(String, OutputText);

  HFONT old = (HFONT) SelectObject(m_hDCBuffer, m_ptrFontStore->GetFont(iSize));

  // Get the dimensions of the text in pixels
  SIZE OutSize;
  GetTextExtentPoint32(m_hDCBuffer, OutputText.c_str(), OutputText.size(), &OutSize);
  SelectObject(m_hDCBuffer, old);
  *Width = OutSize.cx;
  *Height = OutSize.cy;
}

/////////////////////////////////////////////////////////////////////////////

void CScreen::Polygon(point *Points, int Number, int iColour) const {
  HGDIOBJ hpOld;
  hpOld = (HPEN) SelectObject(m_hDCBuffer, (HPEN) m_Pens[iColour]);
  POINT *WinPoints = new POINT[Number];
  point2POINT(Points, WinPoints, Number);
  ::Polygon(m_hDCBuffer, WinPoints, Number);
  delete[]WinPoints;
  SelectObject(m_hDCBuffer, hpOld);
}

/////////////////////////////////////////////////////////////////////////////
