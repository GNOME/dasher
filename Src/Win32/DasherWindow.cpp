// DasherWindow.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#include "../DasherCore/DasherTypes.h"
#include "DasherWindow.h"
#include "WinMenus.h"
#include "Widgets/AboutBox.h"
#include "Widgets/AlphabetBox.h"
#include "Widgets/ColourBox.h"
#include "Widgets/KeyControl.h"
#include "Widgets/SplashScreen.h"
#include "Widgets/Prefs.h"
#include "WinLocalisation.h"
#include "WinUTF8.h"
using namespace Dasher;
using namespace std;


#define IDT_TIMER1 200

CDasherWindow::CDasherWindow(CDasherSettingsInterface* SI, CDasherWidgetInterface* WI, CDasherAppInterface* AI)
	: DasherSettingsInterface(SI), DasherWidgetInterface(WI), DasherAppInterface(AI), Splash(0),
	m_pToolbar(0), m_pEdit(0), m_pCanvas(0), m_pSlidebar(0), m_pSplitter(0), WinOptions(0),
	m_CurrentAlphabet(""), m_CurrentColours("")
{
	hAccelTable = LoadAccelerators(WinHelper::hInstApp, (LPCTSTR)IDC_DASHER);
	
	// Get window title from resource script
	Tstring WindowTitle;
	WinLocalisation::GetResourceString(IDS_APP_TITLE, &WindowTitle);
	
	// Create the Main window
	Tstring WndClassName = CreateMyClass();
	m_hwnd = CreateWindowEx(0, WndClassName.c_str(), WindowTitle.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 500, NULL, NULL, WinHelper::hInstApp, NULL);
	WinWrapMap::add(m_hwnd, this);
	
	// Splash screen (turned off for debugging when it gets in the way)
	// It is deleted when Show() is called.
	/*
	D'oh I've moved things around. Now Splash being here is totally inappropriate;
	Need to make it appear and disappear when training.
#ifndef _DEBUG
	Splash = new CSplash(m_hwnd);
#endif
	*/

	// Create Widgets
	m_pToolbar = new CToolbar(m_hwnd, false, false, false);
	m_pEdit = new CEdit(m_hwnd);
	DasherAppInterface->ChangeEdit(m_pEdit);
	m_pCanvas = new CCanvas(m_hwnd, DasherWidgetInterface, DasherAppInterface, m_pEdit);
	m_pEdit->SetEditCanvas(m_pCanvas);
	m_pSlidebar = new CSlidebar(m_hwnd, DasherSettingsInterface, 1.99, false, m_pCanvas);
	m_pSplitter = new CSplitter(m_hwnd, 100, this);
/*
	DWORD MyTime = GetTickCount();
	DasherAppInterface->TrainFile("Source.txt");
	MyTime = GetTickCount() - MyTime;
*/



	SetTimer(m_hwnd, IDT_TIMER1,               // timer identifier 

    20,                     // 5-second interval 

    (TIMERPROC) NULL); // timer callback

}


CDasherWindow::~CDasherWindow()
{
	delete Splash; // In case Show() was never called.
	delete m_pToolbar;
	delete m_pEdit;
	delete m_pCanvas;
	delete m_pSlidebar;
	delete m_pSplitter;
	
	DestroyIcon(m_hIconSm);

//	SendMessage(m_hwnd, WM_CLOSE, 0, 0);
	WinWrapMap::remove(m_hwnd);
}


/////////////////////////////////////////////////////////////////////////////

int CDasherWindow::MessageLoop()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	return msg.wParam;
}

void CDasherWindow::Show(int nCmdShow)
{
	// Make sure Dasher has started up.
	DasherWidgetInterface->Start();
	
	// Clear SplashScreen
	delete Splash;
	Splash = 0;
	
	// Show Window
	InvalidateRect(m_hwnd, NULL, FALSE);
	ShowWindow(m_hwnd, nCmdShow);
	UpdateWindow(m_hwnd);
}





void CDasherWindow::ChangeAlphabet(const string& NewAlphabetID)
{
	m_CurrentAlphabet = NewAlphabetID;
	DasherAppInterface->AddControlTree(WinMenus::GetWindowMenus()); // Build control tree
}

void CDasherWindow::ChangeColours(const string& NewColourID)
{
	m_CurrentColours = NewColourID;
}

void CDasherWindow::ChangeMaxBitRate(double NewMaxBitRate)
{
	m_pSlidebar->SetValue(NewMaxBitRate);
}


void CDasherWindow::ChangeLanguageModel(unsigned int NewLanguageModelID)
{
}


void CDasherWindow::ChangeView(unsigned int NewViewID)
{
}


void CDasherWindow::ChangeOrientation(Opts::ScreenOrientations Orientation)
{
	using namespace Opts;
	WinMenu.SetStatus(ID_ORIENT_ALPHABET, false, false);
	WinMenu.SetStatus(ID_ORIENT_LR, false, false);
	WinMenu.SetStatus(ID_ORIENT_RL, false, false);
	WinMenu.SetStatus(ID_ORIENT_TB, false, false);
	WinMenu.SetStatus(ID_ORIENT_BT, false, false);
	
	switch(Orientation) {
	case LeftToRight:
		WinMenu.SetStatus(ID_ORIENT_LR, false, true);
		break;
	case RightToLeft:
		WinMenu.SetStatus(ID_ORIENT_RL, false, true);
		break;
	case TopToBottom:
		WinMenu.SetStatus(ID_ORIENT_TB, false, true);
		break;
	case BottomToTop:
		WinMenu.SetStatus(ID_ORIENT_BT, false, true);
		break;
	case Alphabet:
		WinMenu.SetStatus(ID_ORIENT_ALPHABET, false, true);
		break;
	default:
		WinMenu.SetStatus(ID_ORIENT_LR, false, true);
		break;
	}
}


void CDasherWindow::SetFileEncoding(Opts::FileEncodingFormats Encoding)
{
	// !!! Need to make this work. Probably get interface to tell edit box how to save and screen how to display
	using namespace Opts;
	
	WinMenu.SetStatus(ID_SAVE_AS_USER_CODEPAGE, false, false);
	WinMenu.SetStatus(ID_SAVE_AS_ALPHABET_CODEPAGE, false, false);
	WinMenu.SetStatus(ID_SAVE_AS_UTF8, false, false);
	WinMenu.SetStatus(ID_SAVE_AS_UTF16_LITTLE, false, false);
	WinMenu.SetStatus(ID_SAVE_AS_UTF16_BIG, false, false);
	
	switch(Encoding) {
	case UserDefault:
		WinMenu.SetStatus(ID_SAVE_AS_USER_CODEPAGE, false, true);
		break;
	case AlphabetDefault:
		WinMenu.SetStatus(ID_SAVE_AS_ALPHABET_CODEPAGE, false, true);
		break;
	case UTF8:
		WinMenu.SetStatus(ID_SAVE_AS_UTF8, false, true);
		break;
	case UTF16LE:
		WinMenu.SetStatus(ID_SAVE_AS_UTF16_LITTLE, false, true);
		break;
	case UTF16BE:
		WinMenu.SetStatus(ID_SAVE_AS_UTF16_BIG, false, true);
		break;
	default:
		WinMenu.SetStatus(ID_SAVE_AS_USER_CODEPAGE, false, true);
		break;
	}
}


void CDasherWindow::ShowToolbar(bool Value)
{
	if (Value)
		m_pToolbar->SetVisible(CToolbar::SetTrue);
	else
		m_pToolbar->SetVisible(CToolbar::SetFalse);
	Layout();
	WinMenu.SetStatus(ID_TB_SHOW, false, Value);
	WinMenu.SetStatus(ID_TB_TEXT, !Value, WinMenu.GetCheck(ID_TB_TEXT));
	WinMenu.SetStatus(ID_TB_LARGE, !Value, WinMenu.GetCheck(ID_TB_LARGE));
}


void CDasherWindow::ShowToolbarText(bool Value)
{
	if (Value)
		m_pToolbar->ShowText(CToolbar::SetTrue);
	else
		m_pToolbar->ShowText(CToolbar::SetFalse);
	Layout();
	WinMenu.SetStatus(ID_TB_TEXT, false, Value);
}


void CDasherWindow::ShowToolbarLargeIcons(bool Value)
{
	if (Value)
		m_pToolbar->SetLargeIcons(CToolbar::SetTrue);
	else
		m_pToolbar->SetLargeIcons(CToolbar::SetFalse);
	Layout();
	WinMenu.SetStatus(ID_TB_LARGE, false, Value);
}


void CDasherWindow::ShowSpeedSlider(bool Value)
{
	m_pSlidebar->SetVisible(Value);
	Layout();
	WinMenu.SetStatus(ID_SHOW_SLIDE, false, Value);
}


void CDasherWindow::FixLayout(bool Value)
{
	m_pSplitter->SetVisible(!Value);
	Layout();
	WinMenu.SetStatus(ID_FIX_SPLITTER, false, Value);
}


void CDasherWindow::TimeStampNewFiles(bool Value)
{
	timestampnewfiles=Value;
}


void CDasherWindow::DrawMouse(bool Value)
{
	drawmouse=Value;
}


void CDasherWindow::DrawMouseLine(bool Value)
{
	drawmouseline=Value;
}



void CDasherWindow::SetDasherDimensions(bool Value)
{
	oned=Value;
	m_pCanvas->onedimensional(Value);
}



void CDasherWindow::StartOnLeft(bool Value)
{
	startonleft=Value;
	m_pCanvas->StartOnLeftClick(Value);
}



void CDasherWindow::StartOnSpace(bool Value)
{
	startonspace=Value;
	m_pCanvas->StartOnSpace(Value);
}

void CDasherWindow::SetDasherFontSize(Opts::FontSize fontsize)
{
	using namespace Opts;

	switch(fontsize) {
	case Normal:
		WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_NORMAL, false, true);
		WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_LARGE, false, false);
		WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_VERYLARGE, false, false);
		break;
	case Big:
		WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_NORMAL, false, false);
		WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_LARGE, false, true);
		WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_VERYLARGE, false, false);
	break;
	case VBig:
		WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_NORMAL, false, false);
		WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_LARGE, false, false);
		WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_VERYLARGE, false, true);
		break;
	default:
			break;
			//assert(0);
	}
}


void CDasherWindow::WindowPause(bool Value)
{
	windowpause=Value;
	m_pCanvas->WindowPause(Value);
}



void CDasherWindow::KeyControl(bool Value)
{
	keycontrol=Value;
	m_pCanvas->KeyControl(Value);
}


void CDasherWindow::CopyAllOnStop(bool Value)
{
	copyallonstop=Value;
}

void CDasherWindow::Speech(bool Value)
{
	speech=Value;
	m_pCanvas->SpeakOnStop(Value);
}

void CDasherWindow::PaletteChange(bool Value)
{
	palettechange=Value;
}

void CDasherWindow::OutlineBoxes(bool Value)
{
	outlines=Value;
	m_pCanvas->DrawOutlines(Value);
	DasherWidgetInterface->Redraw();
}

void CDasherWindow::MouseposStart(bool Value)
{
	mouseposstart=Value;
	m_pCanvas->MousePosStart(Value);
}

void CDasherWindow::SetYScale(int Value)
{
	m_pCanvas->setyscale(Value);
}

void CDasherWindow::SetMousePosDist(int Value)
{
	m_pCanvas->setmouseposdist(Value);
}

void CDasherWindow::SetUniform(int Value)
{
	m_pCanvas->setuniform(Value);
}

void CDasherWindow::KeyboardMode(bool Value)
{
	keyboardmode=Value;
}

void CDasherWindow::ControlMode(bool Value)
{
	controlmode=Value;
	WinMenu.SetStatus(ID_OPTIONS_CONTROLMODE, false, Value);
	m_pCanvas->SetScreenInterface(DasherWidgetInterface);

	// The edit control caches the symbols so we need to refresh
	m_pEdit->SetInterface(DasherWidgetInterface);
}

void CDasherWindow::SetDasherEyetracker(bool Value)
{
	eyetracker=Value;
}

void CDasherWindow::ColourMode(bool Value)
{
	// Do nothing - colour mode is fixed
}

LRESULT CDasherWindow::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND testwindow;
	RECT windowsize;
	switch (message)
	{
	case MY_LAYOUT:
		Layout();
		break;
	case WM_SETFOCUS:
		SetFocus(m_pCanvas->getwindow());
		break;
	case WM_TIMER:
		// Ugh. Can't find a desperately nicer way of doing this, though
		testwindow=GetForegroundWindow();
		if (testwindow!=m_hwnd) {
			m_pEdit->SetWindow(testwindow);
		}
		SendMessage( m_pCanvas->getwindow(), message, wParam, lParam);
		break;
	case WM_COMMAND:
		{
			const int wmId    = LOWORD(wParam);
			const int wmEvent = HIWORD(wParam);
			
			// Tell edit box if it has changed. It should know itself really, but this is easier
			if (((HWND)lParam==m_pEdit->GetHwnd()) && (HIWORD(wParam)==EN_CHANGE)) {
				m_pEdit->SetDirty();
				break;
			}
			
			// Parse the menu selections:
			switch (wmId)
			{

			case ID_OPTIONS_ENTERTEXT:
				GetWindowRect(m_hwnd,&windowsize);
				if (m_pEdit->GetTextEntry()==false) {
					SetWindowPos(m_hwnd, HWND_TOPMOST, windowsize.left, windowsize.top, (windowsize.right-windowsize.left), (windowsize.bottom-windowsize.top), NULL);
					WinMenu.SetStatus(ID_OPTIONS_ENTERTEXT, false, true);
					m_pEdit->TextEntry(true);
				} else {
					SetWindowPos(m_hwnd, HWND_NOTOPMOST, windowsize.left, windowsize.top, (windowsize.right-windowsize.left), (windowsize.bottom-windowsize.top), NULL);
					WinMenu.SetStatus(ID_OPTIONS_ENTERTEXT, false, false);
					m_pEdit->TextEntry(false);
				}
				break;
			case ID_OPTIONS_CONTROLMODE:
				DasherSettingsInterface->ControlMode(!WinMenu.GetCheck(ID_OPTIONS_CONTROLMODE));
				break;
			case ID_OPTIONS_FONTSIZE_NORMAL: {
			        DasherSettingsInterface->SetDasherFontSize(Dasher::Opts::FontSize(1));
				WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_NORMAL, false, true);
				WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_LARGE, false, false);
				WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_VERYLARGE, false, false);
				break;
			}
			case ID_OPTIONS_FONTSIZE_LARGE: {
			        DasherSettingsInterface->SetDasherFontSize(Dasher::Opts::FontSize(2));
				WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_NORMAL, false, false);
				WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_LARGE, false, true);
				WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_VERYLARGE, false, false);

				break;
			}
			case ID_OPTIONS_FONTSIZE_VERYLARGE: {
			        DasherSettingsInterface->SetDasherFontSize(Dasher::Opts::FontSize(4));
				WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_NORMAL, false, false);
				WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_LARGE, false, false);
				WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_VERYLARGE, false, true);

				break;
			}
			case ID_OPTIONS_EDITFONT: {
				CHOOSEFONT Data;
				LOGFONT lf;
				HFONT Font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
				GetObject(Font, sizeof(LOGFONT), &lf);
				Data.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
				Data.lStructSize = sizeof(CHOOSEFONT);
				Data.hwndOwner = m_hwnd;
				Data.lpLogFont = &lf;
				ChooseFont(&Data);
				string FontName;
				WinUTF8::Tstring_to_UTF8string(lf.lfFaceName, &FontName, GetACP());
				DasherSettingsInterface->SetEditFont(FontName, lf.lfHeight);
				break;
			}
			case ID_OPTIONS_DASHERFONT: {
				CHOOSEFONT Data;
				LOGFONT lf;
				HFONT Font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
				GetObject(Font, sizeof(LOGFONT), &lf);
				Data.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
				Data.lStructSize = sizeof(CHOOSEFONT);
				Data.hwndOwner = m_hwnd;
				Data.lpLogFont = &lf;
				ChooseFont(&Data);
				string FontName;
				WinUTF8::Tstring_to_UTF8string(lf.lfFaceName, &FontName, GetACP());
				DasherSettingsInterface->SetDasherFont(FontName);
				break;
			}
			case ID_OPTIONS_RESETFONT:
				DasherSettingsInterface->SetEditFont("",0);
				DasherSettingsInterface->SetDasherFont("");
				break;
			case IDM_ABOUT:
				{ CAboutbox Aboutbox(m_hwnd); }
				break;
			case ID_OPTIONS_EDITKEYS:
				{ CKeyBox KeyBox(m_hwnd,m_pCanvas, DasherSettingsInterface); }
				break;
			case ID_OPTIONS_ALPHABET:
				{ CAlphabetBox AlphabetBox(m_hwnd, DasherAppInterface, DasherSettingsInterface, m_CurrentAlphabet); }
				break;
			case ID_OPTIONS_COLOURS:
				{ CColourBox ColourBox(m_hwnd, DasherAppInterface, DasherSettingsInterface, m_CurrentColours); }
				break;
			case ID_OPTIONS_PREFS:
				{ CPrefs Prefs(m_hwnd,m_pCanvas,this,DasherSettingsInterface,DasherWidgetInterface); }
				break;
			case ID_HELP_CONTENTS:
				WinHelp(m_hwnd, TEXT("dasher.hlp"), HELP_FINDER, 0);
				break;
			case IDM_EXIT:
				SendMessage(m_hwnd, WM_CLOSE, 0, 0);
				break;
			case ID_TB_SHOW:
				DasherSettingsInterface->ShowToolbar(!WinMenu.GetCheck(ID_TB_SHOW));
				break;
			case ID_TB_TEXT:
				DasherSettingsInterface->ShowToolbarText(!WinMenu.GetCheck(ID_TB_TEXT));
				break;
			case ID_TB_LARGE:
				DasherSettingsInterface->ShowToolbarLargeIcons(!WinMenu.GetCheck(ID_TB_LARGE));
				break;
			case ID_EDIT_SELECTALL:
				m_pEdit->SelectAll();
				break;
			case ID_EDIT_CUT:
				m_pEdit->Cut();
				break;
			case ID_EDIT_COPY:
				m_pEdit->Copy();
				break;
			case ID_EDIT_COPY_ALL:
				m_pEdit->CopyAll();
				break;
			case ID_EDIT_PASTE:
				m_pEdit->Paste();
				break;
			case ID_FILE_NEW:
				m_pEdit->New();
				break;
			case ID_FILE_OPEN:
				m_pEdit->Open();
				break;
			case ID_FILE_SAVE:
				if (!m_pEdit->Save())
					m_pEdit->SaveAs();
				break;
			case ID_FILE_SAVE_AS:
				m_pEdit->SaveAs();
				break;
			case ID_IMPORT_TRAINFILE:
			        DasherAppInterface->TrainFile(m_pEdit->Import());
				break;
			case ID_FIX_SPLITTER:
				DasherSettingsInterface->FixLayout(!WinMenu.GetCheck(ID_FIX_SPLITTER));
				break;
			case ID_SHOW_SLIDE:
				DasherSettingsInterface->ShowSpeedSlider(!WinMenu.GetCheck(ID_SHOW_SLIDE));
				break;
			case ID_TIMESTAMP:
				DasherSettingsInterface->TimeStampNewFiles(!WinMenu.GetCheck(ID_TIMESTAMP));
				break;
			case ID_COPY_ALL_ON_STOP:
				DasherSettingsInterface->CopyAllOnStop(!WinMenu.GetCheck(ID_COPY_ALL_ON_STOP));
				break;
			case ID_ORIENT_ALPHABET:
				DasherSettingsInterface->ChangeOrientation(Opts::Alphabet);
				break;
			case ID_ORIENT_LR:
				DasherSettingsInterface->ChangeOrientation(Opts::LeftToRight);
				break;
			case ID_ORIENT_RL:
				DasherSettingsInterface->ChangeOrientation(Opts::RightToLeft);
				break;
			case ID_ORIENT_TB:
				DasherSettingsInterface->ChangeOrientation(Opts::TopToBottom);
				break;
			case ID_ORIENT_BT:
				DasherSettingsInterface->ChangeOrientation(Opts::BottomToTop);
				break;
			case ID_SAVE_AS_USER_CODEPAGE:
				DasherSettingsInterface->SetFileEncoding(Opts::UserDefault);
				break;
			case ID_SAVE_AS_ALPHABET_CODEPAGE:
				DasherSettingsInterface->SetFileEncoding(Opts::AlphabetDefault);
				break;
			case ID_SAVE_AS_UTF8:
				DasherSettingsInterface->SetFileEncoding(Opts::UTF8);
				break;
			case ID_SAVE_AS_UTF16_LITTLE:
				DasherSettingsInterface->SetFileEncoding(Opts::UTF16LE);
				break;
			case ID_SAVE_AS_UTF16_BIG:
				DasherSettingsInterface->SetFileEncoding(Opts::UTF16BE);
				break;
			default:
				return DefWindowProc(m_hwnd, message, wParam, lParam);
			}
		}
		break;
	case WM_DESTROY:
		if (m_pEdit!=0) {
			m_pEdit->write_to_file();
		}
		
		// Canvas needs to free its resources before its window is deleted
		if (m_pCanvas!=0)
		{
			delete m_pCanvas;
			m_pCanvas=0;
		}		
		PostQuitMessage(0);
		break;
	case WM_GETMINMAXINFO:
		LPPOINT lppt;
			lppt= (LPPOINT)lParam; // lParam points to array of POINTs
			lppt[3].x = 100;       // Set minimum width (arbitrary)
			// Set minimum height:
			lppt[3].y = m_pToolbar->Resize() + m_pSplitter->GetPos()
				+ m_pSplitter->GetHeight() + m_pSlidebar->GetHeight() + GetSystemMetrics(SM_CYEDGE) * 10;
		break;
	case WM_INITMENUPOPUP:
		WinMenu.SortOut((HMENU) wParam);
		break;
	case WM_SIZE:
		if (wParam==SIZE_MINIMIZED)
			break;
		Layout();
		break;
	default:
		return DefWindowProc(m_hwnd, message, wParam, lParam);
	}
	return 0;
}


Tstring CDasherWindow::CreateMyClass()
{
	TCHAR* WndClassName = TEXT("DASHER");
	
	if (WinWrapMap::Register(WndClassName)) {
		WNDCLASSEX wndclass;
		memset(&wndclass, 0, sizeof(WNDCLASSEX));
		wndclass.cbSize = sizeof(WNDCLASSEX);
		wndclass.style          = 0; //CS_HREDRAW | CS_VREDRAW; // these causes a lot of flickering
		wndclass.lpfnWndProc    = WinWrapMap::WndProc;
		wndclass.cbClsExtra     = 0;
		wndclass.cbWndExtra     = 0;
		wndclass.hInstance      = WinHelper::hInstApp;
		wndclass.hIcon          = LoadIcon(wndclass.hInstance, (LPCTSTR)IDI_DASHER);
		wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground  = (HBRUSH)(COLOR_ACTIVEBORDER+1); // Must add one to the value we want for some unknown reason
		wndclass.lpszMenuName   = (LPCTSTR)IDC_DASHER;
		wndclass.lpszClassName  = WndClassName; // Not in a resource - does not require translation
		//wndclass.hIconSm        = LoadIcon(wndclass.hInstance, (LPCTSTR)IDI_DASHER);
		// This gives a better small icon:

		m_hIconSm = (HICON) LoadImage(wndclass.hInstance, (LPCTSTR) IDI_DASHER, IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

		wndclass.hIconSm        = m_hIconSm;
		
		RegisterClassEx(&wndclass);
	}
	
	return Tstring(WndClassName);
}


void CDasherWindow::SetMenuCheck(UINT MenuItem, bool Value)
{
	// TODO: Should be using SetMenuItemInfo
	if (Value)
		CheckMenuItem(GetMenu(m_hwnd), MenuItem, MF_CHECKED);
	else
		CheckMenuItem(GetMenu(m_hwnd), MenuItem, MF_UNCHECKED);
}


void CDasherWindow::Layout()
{
	RECT ClientRect;
	GetClientRect(m_hwnd, &ClientRect);
	const int Width = ClientRect.right;
	const int Height = ClientRect.bottom;
	
	int ToolbarHeight;
	if (m_pToolbar!=0)
		ToolbarHeight = m_pToolbar->Resize();
	else
		ToolbarHeight = 0;
	int CurY = ToolbarHeight;
	
	int SlidebarHeight;
	if (m_pSlidebar!=0)
		SlidebarHeight = m_pSlidebar->Resize(Width, Height);
	else
		SlidebarHeight = 0;
	int MaxCanvas = Height-SlidebarHeight;
	
	int SplitterPos = m_pSplitter->GetPos();
	int SplitterHeight = m_pSplitter->GetHeight();
	SplitterPos = max(CurY+2*SplitterHeight, SplitterPos);
	SplitterPos = min(SplitterPos, MaxCanvas-3*SplitterHeight);
	m_pSplitter->Move(SplitterPos, Width);
	
	m_pEdit->Move(0, CurY, Width, SplitterPos-CurY);
	
	CurY = SplitterPos + SplitterHeight;
	int CanvasHeight = Height - CurY - SlidebarHeight - GetSystemMetrics(SM_CYEDGE);
	
	m_pCanvas->Move(0, CurY, Width, CanvasHeight);
}
