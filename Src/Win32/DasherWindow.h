// DasherWindow.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __DasherWindow_h__
#define __DasherWindow_h__


#include "resource.h"


#include "Widgets/Splitter.h"
#include "Widgets/Menu.h"
#include "Widgets/SplashScreen.h"
#include "Widgets/WindowSelect.h"
#include "DasherMouseInput.h"

#include "../../DasherCore/Win32/WinOptions.h"

class CToolbar;
class CSlidebar;
namespace Dasher 
{
	class CDasherWidgetInterface;
	class CDasherAppInterface;
  class CDasher;
};

// Abstract interfaces to the Dasher engine
#include "../DasherCore/DasherSettingsInterface.h"


class CDasherWindow : public CWinWrap, public CSplitterOwner,
	public Dasher::CDasherSettingsInterface
{
public:
	CDasherWindow();
	~CDasherWindow();
	
	void Show(int nCmdShow);
	int MessageLoop();
	
	// Settings Interface members
	void ChangeAlphabet(const std::string& NewAlphabetID);
	void ChangeOrientation(Dasher::Opts::ScreenOrientations Orientation);
	void SetFileEncoding(Dasher::Opts::FileEncodingFormats Encoding);
	
	void SaveWindowState() const;
	bool LoadWindowState();

	void ShowToolbar(bool Value);
	void ShowToolbarText(bool Value);
	void ShowToolbarLargeIcons(bool Value);
	void ShowSpeedSlider(bool Value);

    void SetDasherFontSize(Dasher::Opts::FontSize fontsize);

	void FixLayout(bool Value);
	void ControlMode(bool Value);

protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

private:
	Dasher::CDasherSettingsInterface* DasherSettingsInterface;
	Dasher::CDasherWidgetInterface* DasherWidgetInterface;
	Dasher::CDasherAppInterface* DasherAppInterface;

    Dasher::CDasher *m_pDasher;
	
	HACCEL hAccelTable;

	// Fiddly Window initialisation
	Tstring CreateMyClass();
	
	// Widgets:
	CToolbar* m_pToolbar;
	CEdit* m_pEdit;
	CCanvas *m_pCanvas;
	CSplitter* m_pSplitter;
    CSlidebar* m_pSlidebar;
    CMenu WinMenu;
	CSplash* Splash;

	HICON m_hIconSm;

	LPCWSTR AutoOffset;
	LPCWSTR DialogCaption;
	char tmpAutoOffset[25];

	// Misc window handling
	void SetMenuCheck(UINT MenuItem, bool Value);
	void Layout();

  
};



#endif /* #ifdef __MainWindow_h__ */
