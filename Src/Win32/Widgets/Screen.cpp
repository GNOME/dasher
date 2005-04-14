// Screen.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "Screen.h"

#include "../WinHelper.h"
#include "DasherEncodingToCP.h"
#include "../WinLocalisation.h"
#include "../../DasherCore/DasherWidgetInterface.h"
#include "../../DasherCore/CustomColours.h"
using namespace WinLocalisation;
using namespace Dasher;
using namespace Opts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////

CScreen::CScreen(HDC hdc, Dasher::screenint iWidth,Dasher::screenint iHeight)
  : CDasherScreen(iWidth, iHeight), m_hdc(hdc), m_FontName(""), Fontsize(Dasher::Opts::FontSize(1)), 
	m_ptrFontStore( new CFontStore(TEXT("")))
{
	// set up the off-screen buffers
//	HDC hdc = GetDC(mainwindow);
	m_hDCBuffer = CreateCompatibleDC(hdc);  // one for rectangles
	m_hDCText = CreateCompatibleDC(hdc);    // the other for text
	m_hbmBit = CreateCompatibleBitmap(hdc,m_iWidth,m_iHeight);
	m_hbmText = CreateCompatibleBitmap(hdc,m_iWidth,m_iHeight);
//	::ReleaseDC(mainwindow, hdc); // Wasn't here before. Should be needed? (IAM)
	m_prevhbmText = SelectObject(m_hDCText,m_hbmText);
	SetBkMode(m_hDCBuffer,TRANSPARENT);
	m_prevhbmBit = SelectObject(m_hDCBuffer,m_hbmBit);
	
	// create the brushes
//	Build_Colours();
	
	CodePage = GetUserCodePage();
	SetFont("");

//	m_hDCScreen = ::GetDC(m_hwnd);
//	TCHAR debug[256];
//	_stprintf(debug, TEXT("GetDC: hwnd %x hdc %x\n"), m_hwnd, m_hDCScreen);
//	OutputDebugString(debug); 
}

/////////////////////////////////////////////////////////////////////////////

CScreen::~CScreen() 
{
	// tidy up

	SelectObject(m_hDCBuffer,m_prevhbmBit);
	SelectObject(m_hDCText,m_prevhbmText);

	BOOL b = DeleteObject(m_hbmBit);
	DASHER_ASSERT(b);
	b = DeleteObject(m_hbmText);
	DASHER_ASSERT(b);
	
	b = DeleteDC(m_hDCBuffer);
	DASHER_ASSERT(b);
	b = DeleteDC(m_hDCText);
	DASHER_ASSERT(b);
	
/*	while (m_vhfFonts.size()) {
		DeleteObject(m_vhfFonts.back());
		m_vhfFonts.pop_back();
	}*/
	
	while (m_Brushes.size()!=0) {
		DeleteObject(m_Brushes.back());
		m_Brushes.pop_back();
	}

	while (m_Pens.size()!=0) {
		DeleteObject(m_Pens.back());
		m_Pens.pop_back();
	}

//	::ReleaseDC(m_hwnd,m_hDCScreen);
//	TCHAR debug[256];
//	_stprintf(debug, TEXT("ReleaseDC: hwnd %x hdc %x\n"), m_hwnd, m_hDCScreen);
//	OutputDebugString(debug); 

}

/////////////////////////////////////////////////////////////////////////////

void CScreen::SetInterface(CDasherWidgetInterface* DasherInterface)
{
	DASHER_ASSERT_VALIDPTR_RW(DasherInterface);

	CDasherScreen::SetInterface(DasherInterface);
	
	CodePage = EncodingToCP(m_DasherInterface->GetAlphabetType());
	SetFont(m_FontName);
	
	unsigned int NumberSymbols = m_DasherInterface->GetNumberSymbols();
	
	DisplayStrings.resize(NumberSymbols);
	
	for (unsigned int i=0; i<NumberSymbols; i++) {
		WinUTF8::UTF8string_to_Tstring(m_DasherInterface->GetDisplayText(i), &DisplayStrings[i], CodePage);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CScreen::SetFont(string Name)
{
	m_FontName = Name;
	
	Tstring FontName;
	WinUTF8::UTF8string_to_Tstring(Name, &FontName);
	
	m_ptrFontStore.reset( new CFontStore(FontName) );

	/*
	// Destroy old fonts
	while (m_vhfFonts.size()!=0) {
		DeleteObject(m_vhfFonts[m_vhfFonts.size()-1]);
		m_vhfFonts.pop_back();
	}
	
	if (Name=="") {
		m_vhfFonts.push_back(GetCodePageFont(CodePage, LONG(-20.0*sqrt(GetFontSize()))));
		m_vhfFonts.push_back(GetCodePageFont(CodePage, LONG(-14.0*sqrt(GetFontSize()))));
		m_vhfFonts.push_back(GetCodePageFont(CodePage, LONG(-11.0*sqrt(GetFontSize()))));
	} else {
		HFONT Font;
		Font = CreateFont(int(-20.0*sqrt(GetFontSize())), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,
		                  FontName.c_str()); // DEFAULT_CHARSET => font made just from Size and FontName
		m_vhfFonts.push_back(Font);
		Font = CreateFont(int(-14.0*sqrt(GetFontSize())), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,
		                  FontName.c_str()); // DEFAULT_CHARSET => font made just from Size and FontName
		m_vhfFonts.push_back(Font);
		Font = CreateFont(int(-11.0*sqrt(GetFontSize())), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,
		                  FontName.c_str()); // DEFAULT_CHARSET => font made just from Size and FontName
		m_vhfFonts.push_back(Font);
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////

void CScreen::SetFontSize(FontSize size)
{
  Fontsize=size;
  SetFont(m_FontName);
}

/////////////////////////////////////////////////////////////////////////////


void CScreen::SetColourScheme(const Dasher::CCustomColours* pColours)
{
	// DJW - must delete brushes ala free_colours. Would be nice to encapsuted this into a Brush Container
	while (m_Brushes.size()!=0) {
		DeleteObject(m_Brushes.back());
		DeleteObject(m_Pens.back());
		m_Brushes.pop_back();
		m_Pens.pop_back();
	}

	int numcolours=pColours->GetNumColours();
	
	DASHER_ASSERT(numcolours>0);

	for (int i=0; i<numcolours; i++) 
	{
		// DJW 20031029 - something fishy is going on - i think calls to CreateSolidBrush start to fail
		HBRUSH hb = CreateSolidBrush(RGB(pColours->GetRed(i),pColours->GetGreen(i),pColours->GetBlue(i)));
		DASHER_ASSERT(hb!=0);
		m_Brushes.push_back(hb);
		HPEN hp = CreatePen(PS_SOLID, 1, RGB(pColours->GetRed(i),pColours->GetGreen(i),pColours->GetBlue(i)));
		DASHER_ASSERT(hp!=0);
		m_Pens.push_back(hp);
	}
}

//void CScreen::Build_Colours ()
//{
//	m_Brushes.resize(6);
	
	// Currently white and gray. Intended for use by a space character, placed last in alphabet
//	m_Brushes[Special1].push_back(CreateSolidBrush(RGB(240,240,240))); // making lighter for djcm
//	m_Brushes[Special2].push_back(CreateSolidBrush(RGB(255,255,255)));
	
//	m_Brushes[Objects].push_back(CreateSolidBrush(RGB(0,0,0)));

//	m_Brushes[Groups].push_back(CreateSolidBrush(RGB(255,255,0)));
//	m_Brushes[Groups].push_back(CreateSolidBrush(RGB(0,255,0)));
//	m_Brushes[Groups].push_back(CreateSolidBrush(RGB(255,100,100)));
	
//	m_Brushes[Nodes1].push_back(CreateSolidBrush(RGB(180,245,180)));
//	m_Brushes[Nodes1].push_back(CreateSolidBrush(RGB(160,200,160)));
//	m_Brushes[Nodes1].push_back(CreateSolidBrush(RGB(0,255,255)));
	
//	m_Brushes[Nodes2].push_back(CreateSolidBrush(RGB(255,185,255)));
//	m_Brushes[Nodes2].push_back(CreateSolidBrush(RGB(140,200,255)));
//	m_Brushes[Nodes2].push_back(CreateSolidBrush(RGB(255,175,175)));
//}


//void CScreen::Free_Colours ()
//{
	// tidy up
//	while (m_Brushes.size()) {
//		while (m_Brushes.back().size()) {
//			DeleteObject(m_Brushes.back().back());
//			m_Brushes.back().pop_back();
//		}
//		m_Brushes.pop_back();
//	}
//}

/////////////////////////////////////////////////////////////////////////////

void CScreen::DrawMousePosBox(int which)
{
//	HBRUSH brush=m_Brushes[ColorScheme][Color%m_Brushes[ColorScheme].size()];
	RECT Rect;
	HBRUSH brush;
	switch (which) {
		case 0:
			Rect.left=0;
			Rect.top=m_iHeight/2-mouseposdist-50;
			Rect.right=m_iWidth;
			Rect.bottom=Rect.top+100;
			brush=CreateSolidBrush(RGB(255,0,0));
			break;
		case 1:
			Rect.left=0;
			Rect.bottom=m_iHeight/2+mouseposdist+50;
			Rect.right=m_iWidth;
			Rect.top=Rect.bottom-100;
			brush=CreateSolidBrush(RGB(255,255,0));
			break;
		default:
			DASHER_ASSERT(0);
	}
	FillRect(m_hDCText, &Rect, brush);
	DeleteObject(brush);
	Display();
}

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

	/*
	if (Size <= 11) {
		Size = 2;
	} else {
		if (Size <= 14)
			Size = 1;
		else
			Size = 0;
	}

	HFONT old= (HFONT) SelectObject(m_hDCText, m_vhfFonts[Size]);*/

	HFONT old= (HFONT) SelectObject(m_hDCBuffer, m_ptrFontStore->GetFont(iSize));

	// The Windows API dumps all its function names in the global namespace, ::
	//::DrawText(m_hDCText, OutputText.c_str(), OutputText.size(), &Rect, DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE );
	::DrawText(m_hDCBuffer, OutputText.c_str(), OutputText.size(), &Rect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE );

	// DJW - need to select the old object back into the DC
	SelectObject(m_hDCBuffer, old);
}

void CScreen::DrawText(std::string OutputString, Dasher::screenint x1, Dasher::screenint y1, int iSize) const
{
	if (m_DasherInterface==0)
		return;
	
	Tstring OutputText;

	WinUTF8::UTF8string_to_Tstring(OutputString,&OutputText);
	
	RECT Rect;
	Rect.left = x1;
	Rect.top = y1;
	Rect.right = x1+50;
	Rect.bottom = y1+50;
	
	/*
	if (Size <= 11) {
		Size = 2;
	} else {
		if (Size <= 14)
			Size = 1;
		else
			Size = 0;
	}

	HFONT old= (HFONT) SelectObject(m_hDCText, m_vhfFonts[Size]);
*/
	HFONT old= (HFONT) SelectObject(m_hDCBuffer, m_ptrFontStore->GetFont(iSize));


	// The Windows API dumps all its function names in the global namespace, ::
	//::DrawText(m_hDCText, OutputText.c_str(), OutputText.size(), &Rect, DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE );
	::DrawText(m_hDCBuffer, OutputText.c_str(), OutputText.size(), &Rect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE );
	
	SelectObject(m_hDCBuffer,old);
}

/////////////////////////////////////////////////////////////////////////////
