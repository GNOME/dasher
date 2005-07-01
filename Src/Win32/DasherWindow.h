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
#include "Widgets/WinOptions.h"
#include "Widgets/Menu.h"
#include "Widgets/SplashScreen.h"
#include "Widgets/WindowSelect.h"
#include "DasherMouseInput.h"

class CToolbar;
class CSlidebar;
namespace Dasher 
{
	class CDasherWidgetInterface;
	class CDasherAppInterface;
};

// Abstract interfaces to the Dasher engine
#include "../DasherCore/DasherSettingsInterface.h"

// Used to signal our message loop to do our periodic work, the value
// shouldn't collide with anything else in our code.
#define WM_DASHER_TIMER WM_USER + 128

class CDasherWindow : public CWinWrap, public CSplitterOwner,
	public Dasher::CDasherSettingsInterface
{
public:
	CDasherWindow(Dasher::CDasherSettingsInterface* SI,
                  Dasher::CDasherWidgetInterface* WI,
				  Dasher::CDasherAppInterface* AI, CWinOptions& WO);
	~CDasherWindow();
	
	void Show(int nCmdShow);
	int MessageLoop();
	
	// Settings Interface members
	void ChangeAlphabet(const std::string& NewAlphabetID);
	void ChangeColours(const std::string& NewColoursID);
	void ChangeMaxBitRate(double NewMaxBitRate);
	void ChangeLanguageModel(unsigned int NewLanguageModelID);
	void ChangeView(unsigned int NewViewID);
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
	void TimeStampNewFiles(bool Value);
	bool GetTimeStamp() {return timestampnewfiles;}
	void CopyAllOnStop(bool Value);
	bool GetCopyAll() {return copyallonstop;}
	void DrawMouse(bool Value);

	bool GetDrawMouse() {return drawmouse;}

	void DrawMouseLine(bool Value);

	bool GetDrawMouseLine() {return drawmouseline;}

	void SetDasherDimensions(bool Value);

	bool GetDasherDimensions() {return oned;}

	void StartOnLeft(bool Value);

	bool GetStartOnLeft() {return startonleft;}

	void StartOnSpace(bool Value);

	bool GetStartOnSpace() {return startonspace;}

	void KeyControl(bool Value);

	bool GetKeyControl() {return keycontrol;}

	void WindowPause(bool Value);

	bool GetWindowPause() {return windowpause;}

	void ControlMode(bool Value);
	bool GetControlMode() {return controlmode;}
	void ColourMode(bool Value);
	bool GetColourMode() {return colourmode;}
	void KeyboardMode(bool Value);
	bool GetKeyboardMode() {return keyboardmode;}
	void MouseposStart(bool Value);
	bool GetMouseposStart() {return mouseposstart;}
	void Speech(bool Value);
	bool GetSpeech() {return speech;}
	void OutlineBoxes(bool Value);
	bool GetOutlines() {return outlines;}
	void SetDasherEyetracker(bool Value);
	bool GetDasherEyetracker() {return eyetracker;}
	void PaletteChange(bool Value);
	bool GetPaletteChange() {return palettechange;}
	void SetYScale(int Value);
	void SetMousePosDist(int Value);
	void SetUniform(int Value);

protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

private:
	Dasher::CDasherSettingsInterface* DasherSettingsInterface;
	Dasher::CDasherWidgetInterface* DasherWidgetInterface;
	Dasher::CDasherAppInterface* DasherAppInterface;
	std::string m_CurrentAlphabet;
	std::string m_CurrentColours;
	
	HACCEL hAccelTable;

	// Fiddly Window initialisation
	Tstring CreateMyClass();
	
	// Widgets:
	CToolbar* m_pToolbar;
	CEdit* m_pEdit;
	CCanvas* m_pCanvas;
	CSlidebar* m_pSlidebar;
	CSplitter* m_pSplitter;
	CWinOptions& WinOptions;
	CMenu WinMenu;
	CSplash* Splash;

	

	HICON m_hIconSm;

	LPCWSTR AutoOffset;
	LPCWSTR DialogCaption;
	char tmpAutoOffset[25];

	bool startonspace;

	bool startonleft;

	bool timestampnewfiles;
	bool drawmouse;
	bool drawmouseline;
	bool oned;
	bool windowpause;
	bool keycontrol;
	bool copyallonstop;
	bool eyetracker;
	bool outlines;
	bool mousepos;
	bool controlmode;
	bool colourmode;
	bool keyboardmode;
	bool mouseposstart;
	bool speech;
	bool palettechange;
	

	// Misc window handling
	void SetMenuCheck(UINT MenuItem, bool Value);
	void Layout();

    HANDLE          m_workerThread;    // Handle to our worker thread that periodically checks on user's activities
    bool            m_bWorkerShutdown; // Set to true when the worker should terminate

    static DWORD  WINAPI    WorkerThread(LPVOID lpParam);   // Spins around and sends WM_DASHER_TIMER message
    void                    ShutdownWorkerThread();         // Called when we want the worker thread to stop
    void                    OnTimer();                      // Does the periodic work

};



#endif /* #ifdef __MainWindow_h__ */
