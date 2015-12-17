// Screen.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "..\Common\WinCommon.h"

#include "Screen.h"

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

CScreen::CScreen(HDC hdc, HWND hWnd, Dasher::screenint iWidth, Dasher::screenint iHeight, const std::string &strFont)
:CLabelListScreen(iWidth, iHeight), m_hdc(hdc) {
  m_hWnd = hWnd;

  CreateBuffers();
  SetFont(strFont);
}

void CScreen::CreateBuffers() {
  // Create a memory device context compatible with the screen
  m_hDCBufferBackground = CreateCompatibleDC(m_hdc);      // one for rectangles
  m_hbmBitBackground = CreateCompatibleBitmap(m_hdc, GetWidth(), GetHeight());
  SetBkMode(m_hDCBufferBackground, TRANSPARENT);
  m_prevhbmBitBackground = SelectObject(m_hDCBufferBackground, m_hbmBitBackground);

  m_hDCBufferDecorations = CreateCompatibleDC(m_hdc);
  m_hbmBitDecorations = CreateCompatibleBitmap(m_hdc, GetWidth(), GetHeight());
  SetBkMode(m_hDCBufferDecorations, TRANSPARENT);
  m_prevhbmBitDecorations = SelectObject(m_hDCBufferDecorations, m_hbmBitDecorations);

  // FIXME - I'm assuming it's okay just to copy handles like this, can someone do something about this if I'm wrong

  m_hDCBuffer = m_hDCBufferBackground;

}
/////////////////////////////////////////////////////////////////////////////

CScreen::~CScreen() {
  DeleteBuffers();
}

void CScreen::DeleteBuffers() {
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

void CScreen::SetColourScheme(const CColourIO::ColourInfo *pColours) {
  m_cPens.clear();
  m_cBrushes.clear();
  m_pColours = pColours;
}

void CScreen::SetFont(const string &strFont) {
  if(FontName == strFont) return;
  FontName = strFont;
  for(auto it(m_cFonts.begin()); it != m_cFonts.end(); ++it)
    DeleteObject(it->second);
  m_cFonts.clear();
  for (set<CLabelListScreen::Label*>::iterator it=LabelsBegin(); it!=LabelsEnd(); it++)
	  static_cast<CScreen::Label*>(*it)->m_sizeCache.clear();
}

// Handler for redraw markers;
void CScreen::SendMarker(int iMarker) {
  switch (iMarker) {
  case 0:
    m_hDCBuffer = m_hDCBufferBackground;
    break;
  case 1:
    BitBlt(m_hDCBufferDecorations, 0, 0, GetWidth(), GetHeight(), m_hDCBufferBackground, 0, 0, SRCCOPY);
    m_hDCBuffer = m_hDCBufferDecorations;
    break;
  }
}

void CScreen::resize(screenint width, screenint height) {
  DeleteBuffers();
  CLabelListScreen::resize(width,height);
  CreateBuffers();
}

CScreen::Label::Label(CScreen *pScreen, const string &strText, unsigned int iWrapSize) : CLabelListScreen::Label(pScreen, strText, iWrapSize), m_OutputText(WinUTF8::UTF8string_to_wstring(m_strText)) {
}

CDasherScreen::Label *CScreen::MakeLabel(const string &strText, unsigned int iWrapSize) {
  return new Label(this,strText,iWrapSize);
}

void CScreen::DrawString(CDasherScreen::Label *lab, screenint x1, screenint y1, unsigned int iSize, int Colour) {
  Label *label(static_cast<Label *>(lab));
  RECT Rect;
  Rect.left = x1;
  Rect.top = y1;
  Rect.right = x1 + (lab->m_iWrapSize ? GetWidth() : 50); //if not wrapping, will extend beyond RHS because of DT_NOCLIP
  Rect.bottom = y1 + 50; //and beyond bottom in either case

  HFONT old = (HFONT) SelectObject(m_hDCBuffer, CScreen::GetFont(iSize));

  COLORREF iCRefOld;
  COLORREF iCRefNew;

  // Colour was hardcoded to 4
  iCRefNew = RGB(m_pColours->Reds[Colour], m_pColours->Greens[Colour], m_pColours->Blues[Colour]);
  
  iCRefOld = SetTextColor(m_hDCBuffer, iCRefNew);

  // The Windows API dumps all its function names in the global namespace, ::
  ::DrawText(m_hDCBuffer, label->m_OutputText.c_str(), label->m_OutputText.size(), &Rect, (label->m_iWrapSize ? DT_CENTER | DT_WORDBREAK : DT_LEFT | DT_SINGLELINE) | DT_TOP | DT_NOCLIP | DT_NOPREFIX);
  
  SetTextColor(m_hDCBuffer, iCRefOld);
  SelectObject(m_hDCBuffer, old);
}

pair<screenint,screenint> CScreen::TextSize(CDasherScreen::Label *lab, unsigned int iSize) {
  Label *label(static_cast<Label *>(lab));

  map<unsigned int,pair<screenint,screenint> >::const_iterator it = label->m_sizeCache.find(iSize);
  if (it!=label->m_sizeCache.end()) return it->second;

  pair<screenint,screenint> res = TextSize_Impl(label, iSize);
  label->m_sizeCache.insert(map<unsigned int,pair<screenint,screenint> >::value_type(iSize,res));
  return res;
}

/////////////////////////////////////////////////////////////////////////////

pair<screenint,screenint> CScreen::TextSize_Impl(CScreen::Label *label, unsigned int iSize) {
  // TODO This function could be improved. The height of an "o" is returned as the
  // same as the height of an "O". Perhaps GetGlyphOutline could help.
  // Remember if it gets complicted, the height of each symbol could be pre-calculated

  //wstring OutputText; //ACL assuming wstring is same as Tstring?
  //WinUTF8::UTF8string_to_wstring(String, OutputText);

  HFONT old = (HFONT) SelectObject(m_hDCBuffer, CScreen::GetFont(iSize));

  // Get the dimensions of the text in pixels
  if (label->m_iWrapSize) {
    RECT Rect;
    Rect.left = 0;
    Rect.top = 0;
    Rect.right = GetWidth(); //will be wrapped to this
    Rect.bottom = 0; //this'll be overwritten
    //This is the only way to get size of text w/ wrapping:
    // -the DT_WORDBREAK flag says to wrap (no way to pass this to GetTextExtentPoint32)
    // -the DT_CALCRECT flag says to modify Rect with the size, but not to draw the string.
    ::DrawText(m_hDCBuffer, label->m_OutputText.c_str(), label->m_OutputText.size(), &Rect, DT_LEFT | DT_WORDBREAK | DT_TOP | DT_NOPREFIX | DT_CALCRECT);
    return pair<screenint,screenint>(Rect.right,Rect.bottom);
  }
  SIZE OutSize;
  GetTextExtentPoint32(m_hDCBuffer, label->m_OutputText.c_str(), label->m_OutputText.size(), &OutSize);
  SelectObject(m_hDCBuffer, old);
  return pair<screenint,screenint>(OutSize.cx,OutSize.cy);
}

/////////////////////////////////////////////////////////////////////////////
void CScreen::Polygon(point *Points, int Number, int fillColour, int outlineColour, int iWidth) {
  HGDIOBJ hpOld;
  hpOld = (HPEN) SelectObject(m_hDCBuffer, CScreen::GetPen(fillColour, iWidth));
  POINT *WinPoints = new POINT[Number];
  point2POINT(Points, WinPoints, Number);
  ::Polygon(m_hDCBuffer, WinPoints, Number);
  delete[]WinPoints;
  SelectObject(m_hDCBuffer, hpOld);
  if (iWidth>0) {
    Polyline(Points, Number, iWidth, outlineColour==-1 ? 3 : outlineColour);
  }
}

/////////////////////////////////////////////////////////////////////////////
bool CScreen::IsWindowUnderCursor() {
  POINT pt;
  GetCursorPos(&pt);
  HWND h = WindowFromPoint(pt);
  return h == m_hWnd;
}
