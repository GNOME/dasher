// Screen.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////


#include "../WinHelper.h"
#include "Screen.h"
#include "DasherEncodingToCP.h"
#include "../WinLocalisation.h"
using namespace WinLocalisation;
using namespace Dasher;
using namespace Opts;
using namespace std;

CScreen::CScreen(HWND mainwindow, int iWidth,int iHeight)
  : CDasherScreen(iWidth, iHeight), m_hwnd(mainwindow), RealHDC(0), m_FontName(""), Fontsize(Dasher::Opts::FontSize(1))
{
	// set up the off-screen buffers
	HDC hdc = GetWindowDC(mainwindow);
	m_hDCBuffer = CreateCompatibleDC(hdc);  // one for rectangles
	m_hDCText = CreateCompatibleDC(hdc);    // the other for text
	m_hbmBit = CreateCompatibleBitmap(hdc,m_iWidth,m_iHeight);
	m_hbmText = CreateCompatibleBitmap(hdc,m_iWidth,m_iHeight);
	ReleaseDC(mainwindow, hdc); // Wasn't here before. Should be needed? (IAM)
	SelectObject(m_hDCText,m_hbmText);
	SetBkMode(m_hDCText,TRANSPARENT);
	SelectObject(m_hDCBuffer,m_hbmBit);
	
	// create the brushes
	Build_Colours();
	
	CodePage = GetUserCodePage();
	SetFont("");
}


CScreen::~CScreen() {
	// tidy up
	
	DeleteDC(m_hDCBuffer);
	DeleteDC(m_hDCText);
	DeleteObject(m_hbmBit);
	DeleteObject(m_hbmText);
	while (m_vhfFonts.size()) {
		DeleteObject(m_vhfFonts.back());
		m_vhfFonts.pop_back();
	}
	
	Free_Colours();
}


void CScreen::SetInterface(CDasherWidgetInterface* DasherInterface)
{
	CDasherScreen::SetInterface(DasherInterface);
	
	CodePage = EncodingToCP(m_DasherInterface->GetAlphabetType());
	SetFont(m_FontName);
	
	unsigned int NumberSymbols = m_DasherInterface->GetNumberSymbols();
	
	DisplayStrings.resize(NumberSymbols);
	
	for (unsigned int i=0; i<NumberSymbols; i++) {
		WinUTF8::UTF8string_to_Tstring(m_DasherInterface->GetDisplayText(i), &DisplayStrings[i], CodePage);
	}
}


void CScreen::SetFont(string Name)
{
	m_FontName = Name;
	
	Tstring FontName;
	WinUTF8::UTF8string_to_Tstring(Name, &FontName);
	
	// Destroy old fonts
	while (m_vhfFonts.size()!=0) {
		DeleteObject(m_vhfFonts[m_vhfFonts.size()-1]);
		m_vhfFonts.pop_back();
	}
	
	if (Name=="") {
		m_vhfFonts.push_back(GetCodePageFont(CodePage, -20*sqrt(GetFontSize())));
		m_vhfFonts.push_back(GetCodePageFont(CodePage, -14*sqrt(GetFontSize())));
		m_vhfFonts.push_back(GetCodePageFont(CodePage, -11*sqrt(GetFontSize())));
	} else {
		HFONT Font;
		Font = CreateFont(-20*sqrt(GetFontSize()), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,
		                  FontName.c_str()); // DEFAULT_CHARSET => font made just from Size and FontName
		m_vhfFonts.push_back(Font);
		Font = CreateFont(-14*sqrt(GetFontSize()), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,
		                  FontName.c_str()); // DEFAULT_CHARSET => font made just from Size and FontName
		m_vhfFonts.push_back(Font);
		Font = CreateFont(-11*sqrt(GetFontSize()), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		                  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,
		                  FontName.c_str()); // DEFAULT_CHARSET => font made just from Size and FontName
		m_vhfFonts.push_back(Font);
	}
}

void CScreen::SetFontSize(FontSize size)
{
  Fontsize=size;
  SetFont(m_FontName);
}

FontSize CScreen::GetFontSize()
{
  return Fontsize;
}

void CScreen::Build_Colours ()
{
	m_Brushes.resize(6);
	
	// Currently white and gray. Intended for use by a space character, placed last in alphabet
	m_Brushes[Special1].push_back(CreateSolidBrush(RGB(240,240,240))); // making lighter for djcm
	m_Brushes[Special2].push_back(CreateSolidBrush(RGB(255,255,255)));
	
	m_Brushes[Objects].push_back(CreateSolidBrush(RGB(0,0,0)));

	m_Brushes[Groups].push_back(CreateSolidBrush(RGB(255,255,0)));
	m_Brushes[Groups].push_back(CreateSolidBrush(RGB(0,255,0)));
	m_Brushes[Groups].push_back(CreateSolidBrush(RGB(255,100,100)));
	
	m_Brushes[Nodes1].push_back(CreateSolidBrush(RGB(180,245,180)));
	m_Brushes[Nodes1].push_back(CreateSolidBrush(RGB(160,200,160)));
	m_Brushes[Nodes1].push_back(CreateSolidBrush(RGB(0,255,255)));
	
	m_Brushes[Nodes2].push_back(CreateSolidBrush(RGB(255,185,255)));
	m_Brushes[Nodes2].push_back(CreateSolidBrush(RGB(140,200,255)));
	m_Brushes[Nodes2].push_back(CreateSolidBrush(RGB(255,175,175)));
}


void CScreen::Free_Colours ()
{
	// tidy up
	while (m_Brushes.size()) {
		while (m_Brushes.back().size()) {
			DeleteObject(m_Brushes.back().back());
			m_Brushes.back().pop_back();
		}
		m_Brushes.pop_back();
	}
}
