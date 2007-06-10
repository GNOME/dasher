// Screen.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "..\Common\WinCommon.h"

#include "Screen.h"

#include "../Common/DasherEncodingToCP.h"
// #include "../../DasherCore/CustomColours.h"
using namespace WinLocalisation;
using namespace Dasher;
using namespace Opts;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////

CScreen::CScreen(HDC hdc, HWND hWnd, Dasher::screenint iWidth, Dasher::screenint iHeight)
:CDasherScreen(iWidth, iHeight), m_hdc(hdc) {
  // set up the off-screen buffers
  // HDC hdc = GetDC(mainwindow);

  m_hWnd = hWnd;

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

  CodePage = GetUserCodePage();

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
}

/////////////////////////////////////////////////////////////////////////////

void CScreen::SetInterface(CDasherInterfaceBase *DasherInterface) {
#ifndef _WIN32_WCE
  DASHER_ASSERT_VALIDPTR_RW(DasherInterface);
#endif

 // CDasherScreen::SetInterface(DasherInterface);

//  CodePage = EncodingToCP(m_pDasherInterface->GetAlphabetType());
}

void CScreen::SetColourScheme(const Dasher::CColourIO::ColourInfo *pColours) {
  m_cPens.clear();
  m_cBrushes.clear();
  m_pColours = pColours;
}


// Handler for redraw markers;
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

void CScreen::DrawMousePosBox(int which, int iMousePosDist,int layer) {
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

void CScreen::DrawString(const std::string &OutputString, Dasher::screenint x1, Dasher::screenint y1, int iSize) {

  Tstring OutputText;

  WinUTF8::UTF8string_to_wstring(OutputString, OutputText);

  RECT Rect;
  Rect.left = x1;
  Rect.top = y1;
  Rect.right = x1 + 50;
  Rect.bottom = y1 + 50;

  HFONT old = (HFONT) SelectObject(m_hDCBuffer, CScreen::GetFont(iSize));

  COLORREF iCRefOld;
  COLORREF iCRefNew;

  // TODO: Hardcoded numbers
  iCRefNew = RGB(m_pColours->Reds[4], m_pColours->Greens[4], m_pColours->Blues[4]);
  
  iCRefOld = SetTextColor(m_hDCBuffer, iCRefNew);

  // The Windows API dumps all its function names in the global namespace, ::
  ::DrawText(m_hDCBuffer, OutputText.c_str(), OutputText.size(), &Rect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE);
  
  SetTextColor(m_hDCBuffer, iCRefOld);
  SelectObject(m_hDCBuffer, old);
}

void CScreen::TextSize(const std::string &String, screenint *Width, screenint *Height, int iSize) {
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

void CScreen::TextSize_Impl(const std::string &String, screenint *Width, screenint *Height, int iSize) {
  // TODO This function could be improved. The height of an "o" is returned as the
  // same as the height of an "O". Perhaps GetGlyphOutline could help.
  // Remember if it gets complicted, the height of each symbol could be pre-calculated

  wstring OutputText;
  WinUTF8::UTF8string_to_wstring(String, OutputText);

  HFONT old = (HFONT) SelectObject(m_hDCBuffer, CScreen::GetFont(iSize));

  // Get the dimensions of the text in pixels
  SIZE OutSize;
  GetTextExtentPoint32(m_hDCBuffer, OutputText.c_str(), OutputText.size(), &OutSize);
  SelectObject(m_hDCBuffer, old);
  *Width = OutSize.cx;
  *Height = OutSize.cy;
}

/////////////////////////////////////////////////////////////////////////////

/*void CScreen::Polygon(point *Points, int Number, int iColour) {
  CScreen::Polygon( Points, Number, iColour, 1);
}*/

void CScreen::Polygon(point *Points, int Number, int iColour, int iWidth) {
  HGDIOBJ hpOld;
  hpOld = (HPEN) SelectObject(m_hDCBuffer, CScreen::GetPen(iColour, iWidth));
  POINT *WinPoints = new POINT[Number];
  point2POINT(Points, WinPoints, Number);
  ::Polygon(m_hDCBuffer, WinPoints, Number);
  delete[]WinPoints;
  SelectObject(m_hDCBuffer, hpOld);
}

/////////////////////////////////////////////////////////////////////////////
