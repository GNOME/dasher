// DasherWindow.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __DasherWindow_h__
#define __DasherWindow_h__

// Abstract interfaces to the Dasher engine
// #define _DASHER_VER_ 0x0300 /* Probably not the way to do things */
#include "../DasherCore/DasherSettingsInterface.h"
#include "../DasherCore/DasherWidgetInterface.h"
#include "../DasherCore/DasherAppInterface.h"

// Stuff to make the window work
#include "WinWrap.h"
#include "resource.h"

// Window components
#include "Widgets/Toolbar.h"
#include "Widgets/Edit.h"
#include "Widgets/Canvas.h"
#include "Widgets/Slidebar.h"
#include "Widgets/Splitter.h"
#include "Widgets/WinOptions.h"
#include "Widgets/Menu.h"
#include "Widgets/SplashScreen.h"


class CDasherWindow : public CWinWrap, public CSplitterOwner,
	public Dasher::CDasherSettingsInterface
{
public:
	CDasherWindow(Dasher::CDasherSettingsInterface* SI,
                  Dasher::CDasherWidgetInterface* WI,
                  Dasher::CDasherAppInterface* AI);
	~CDasherWindow();
	
	void Show(int nCmdShow);
	int MessageLoop();
	
	// Settings Interface members
	void ChangeAlphabet(const std::string& NewAlphabetID);
	void ChangeMaxBitRate(double NewMaxBitRate);
	void ChangeLanguageModel(unsigned int NewLanguageModelID);
	void ChangeView(unsigned int NewViewID);
	void ChangeOrientation(Dasher::Opts::ScreenOrientations Orientation);
	void SetFileEncoding(Dasher::Opts::FileEncodingFormats Encoding);
	
	void ShowToolbar(bool Value);
	void ShowToolbarText(bool Value);
	void ShowToolbarLargeIcons(bool Value);
	void ShowSpeedSlider(bool Value);
	void FixLayout(bool Value);
	void TimeStampNewFiles(bool Value);
	void CopyAllOnStop(bool Value);
	void DrawMouse(bool Value);
	void SetDasherDimensions(bool Value);
	void StartOnLeft(bool Value);
	void StartOnSpace(bool Value);

protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	Dasher::CDasherSettingsInterface* DasherSettingsInterface;
	Dasher::CDasherWidgetInterface* DasherWidgetInterface;
	Dasher::CDasherAppInterface* DasherAppInterface;
	std::string m_CurrentAlphabet;
	
	HACCEL hAccelTable;
	
	// Fiddly Window initialisation
	Tstring CreateMyClass();
	
	// Widgets:
	CToolbar* m_pToolbar;
	CEdit* m_pEdit;
	CCanvas* m_pCanvas;
	CSlidebar* m_pSlidebar;
	CSplitter* m_pSplitter;
	CWinOptions* WinOptions;
	CMenu WinMenu;
	CSplash* Splash;
	bool startonspace;
	bool startonleft;

	// Misc window handling
	void SetMenuCheck(UINT MenuItem, bool Value);
	void Layout();
};


#endif /* #ifdef __MainWindow_h__ */