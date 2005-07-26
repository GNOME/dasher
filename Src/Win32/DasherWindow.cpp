// DasherWindow.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "DasherWindow.h"

#include "Common/WinMenus.h"

#include "../DasherCore/DasherTypes.h"
#include "Widgets/AboutBox.h"
#include "Widgets/AlphabetBox.h"
#include "Widgets/ColourBox.h"
#include "Widgets/KeyControl.h"
#include "Widgets/SplashScreen.h"
#include "Widgets/Prefs.h"
#include "Widgets/Toolbar.h"
#include "Widgets/Slidebar.h"
#include "DasherMouseInput.h"
#include "Dasher.h"
using namespace Dasher;
using namespace std;

#define IDT_TIMER1 200

CDasherWindow::CDasherWindow()
:Splash(0), m_pToolbar(0), m_pEdit(0), m_pSlidebar(0), m_pSplitter(0) {

  hAccelTable = LoadAccelerators(WinHelper::hInstApp, (LPCTSTR) IDC_DASHER);

  // Get window title from resource script
  Tstring WindowTitle;
  WinLocalisation::GetResourceString(IDS_APP_TITLE, &WindowTitle);

  // Create the Main window
  Tstring WndClassName = CreateMyClass();
  m_hwnd = CreateWindowEx(0, WndClassName.c_str(), WindowTitle.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 400, 500, NULL, NULL, WinHelper::hInstApp, NULL);
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
  m_pDasher = new CDasher(m_hwnd);
  m_pEdit = new CEdit(m_hwnd);
  m_pToolbar = new CToolbar(m_hwnd, m_pDasher);

  // Set an object to handle edit events
  m_pDasher->SetEdit(m_pEdit);

  // FIXME - the edit box really shouldn't need access to the interface, 
  // but at the moment it does, for training, blanking the display etc

  m_pEdit->SetInterface(m_pDasher);

  // FIXME - we shouldn't need to know about these outside of CDasher

  m_pCanvas = m_pDasher->GetCanvas();
  m_pSlidebar = m_pDasher->GetSlidebar();
  m_pSplitter = new CSplitter(m_hwnd, m_pDasher, 100, this);
}

CDasherWindow::~CDasherWindow() {
  delete Splash;                // In case Show() was never called.
  delete m_pToolbar;
  delete m_pEdit;
  delete m_pSplitter;
  delete m_pDasher;

  DestroyIcon(m_hIconSm);
  WinWrapMap::remove(m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////

int CDasherWindow::MessageLoop() {
  MSG msg;
  while(GetMessage(&msg, NULL, 0, 0)) {
    if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return msg.wParam;
}

void CDasherWindow::Show(int nCmdShow) {
  // Make sure Dasher has started up.
  m_pDasher->Start();

  // Clear SplashScreen
  delete Splash;
  Splash = 0;

  // Show Window
  InvalidateRect(m_hwnd, NULL, FALSE);
  if(!LoadWindowState())
    ShowWindow(m_hwnd, nCmdShow);       // Now set up. Kill splash screen and display main window
  UpdateWindow(m_hwnd);
}

void CDasherWindow::SaveWindowState() const {
  WINDOWPLACEMENT wp;
  wp.length = sizeof(WINDOWPLACEMENT);
}

bool CDasherWindow::LoadWindowState() {
  return false;
}

LRESULT CDasherWindow::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  RECT windowsize;
  switch (message) {
  case MY_LAYOUT:
    Layout();
    break;
  case WM_COMMAND:
    {
      const int wmId = LOWORD(wParam);
      const int wmEvent = HIWORD(wParam);

      // Tell edit box if it has changed. It should know itself really, but this is easier
      if(((HWND) lParam == m_pEdit->GetHwnd()) && (HIWORD(wParam) == EN_CHANGE)) {
        m_pEdit->SetDirty();
        break;
      }

      // Parse the menu selections:
      switch (wmId) {
      case ID_OPTIONS_ENTERTEXT:
        GetWindowRect(m_hwnd, &windowsize);
        if(m_pEdit->GetTextEntry() == false) {
          SetWindowPos(m_hwnd, HWND_TOPMOST, windowsize.left, windowsize.top, (windowsize.right - windowsize.left), (windowsize.bottom - windowsize.top), NULL);
          m_pEdit->TextEntry(true);
        }
        else {
          SetWindowPos(m_hwnd, HWND_NOTOPMOST, windowsize.left, windowsize.top, (windowsize.right - windowsize.left), (windowsize.bottom - windowsize.top), NULL);
          m_pEdit->TextEntry(false);
        }
        break;
      case ID_OPTIONS_CONTROLMODE:
        m_pDasher->SetBoolParameter(BP_CONTROL_MODE, !WinMenu.GetCheck(ID_OPTIONS_CONTROLMODE));
        m_pDasher->RequestFullRedraw();
        break;
      case ID_OPTIONS_FONTSIZE_NORMAL:
        m_pDasher->SetLongParameter(LP_DASHER_FONTSIZE, Dasher::Opts::FontSize(1));
        m_pDasher->RequestFullRedraw();
        break;
      case ID_OPTIONS_FONTSIZE_LARGE:
        m_pDasher->SetLongParameter(LP_DASHER_FONTSIZE, Dasher::Opts::FontSize(2));
        m_pDasher->RequestFullRedraw();
        break;
      case ID_OPTIONS_FONTSIZE_VERYLARGE:
        m_pDasher->SetLongParameter(LP_DASHER_FONTSIZE, Dasher::Opts::FontSize(4));
        m_pDasher->RequestFullRedraw();
        break;
      case ID_OPTIONS_EDITFONT:{
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
          WinUTF8::wstring_to_UTF8string(lf.lfFaceName, FontName);
          m_pDasher->SetLongParameter(LP_EDIT_FONT_SIZE, lf.lfHeight);
          m_pDasher->SetStringParameter(SP_EDIT_FONT, FontName);
        }
        break;
      case ID_OPTIONS_DASHERFONT:{
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
          WinUTF8::wstring_to_UTF8string(lf.lfFaceName, FontName);
          m_pDasher->SetStringParameter(SP_DASHER_FONT, FontName);
        }
        break;
      case ID_OPTIONS_RESETFONT:
        m_pDasher->SetLongParameter(LP_EDIT_FONT_SIZE, 0);
        m_pDasher->SetStringParameter(SP_EDIT_FONT, "");
        m_pDasher->SetStringParameter(SP_DASHER_FONT, "");
        break;
      case IDM_ABOUT:{
          CAboutbox Aboutbox(m_hwnd);
        }
        break;
      case ID_OPTIONS_EDITKEYS:{
          CKeyBox KeyBox(m_hwnd, m_pDasher);
        }
        break;
      case ID_OPTIONS_ALPHABET:{
          CAlphabetBox AlphabetBox(m_hwnd, m_pDasher);
        }
        break;
      case ID_OPTIONS_COLOURS:{
          CColourBox ColourBox(m_hwnd, m_pDasher);
        }
        break;
      case ID_OPTIONS_PREFS:{
          CPrefs Prefs(m_hwnd, m_pDasher);
        }
        break;
      case ID_HELP_CONTENTS:
        WinHelp(m_hwnd, TEXT("dasher.hlp"), HELP_FINDER, 0);
        break;
      case IDM_EXIT:
        SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        break;

      // FIXME - These options shouldn't pass through the interface
      case ID_TB_SHOW:
        m_pDasher->SetBoolParameter(BP_SHOW_TOOLBAR, !WinMenu.GetCheck(ID_TB_SHOW));
        m_pToolbar->ShowToolbar(m_pDasher->GetBoolParameter(BP_SHOW_TOOLBAR));
        break;
      case ID_TB_TEXT:
        m_pDasher->SetBoolParameter(BP_SHOW_TOOLBAR_TEXT, !WinMenu.GetCheck(ID_TB_TEXT));
        m_pToolbar->ShowToolbar(m_pDasher->GetBoolParameter(BP_SHOW_TOOLBAR));
        break;
      case ID_TB_LARGE:
        m_pDasher->SetBoolParameter(BP_SHOW_LARGE_ICONS, !WinMenu.GetCheck(ID_TB_LARGE));
        m_pToolbar->ShowToolbar(m_pDasher->GetBoolParameter(BP_SHOW_TOOLBAR));
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
        if(!m_pEdit->Save())
          m_pEdit->SaveAs();
        break;
      case ID_FILE_SAVE_AS:
        m_pEdit->SaveAs();
        break;
      case ID_IMPORT_TRAINFILE:
        m_pDasher->TrainFile(m_pEdit->Import());
        break;

        // FIXME - This options shouldn't passs through the interface

      case ID_FIX_SPLITTER:
        m_pDasher->SetBoolParameter(BP_FIX_LAYOUT, !WinMenu.GetCheck(ID_FIX_SPLITTER));
        break;
      case ID_SHOW_SLIDE:
        m_pDasher->SetBoolParameter(BP_SHOW_SLIDER, !WinMenu.GetCheck(ID_SHOW_SLIDE));
        break;

        // FIXME - These options shouldn't pass through the interface
      case ID_TIMESTAMP:
        m_pDasher->SetBoolParameter(BP_TIME_STAMP, !WinMenu.GetCheck(ID_TIMESTAMP));
        break;
      case ID_COPY_ALL_ON_STOP:
        m_pDasher->SetBoolParameter(BP_COPY_ALL_ON_STOP, !WinMenu.GetCheck(ID_COPY_ALL_ON_STOP));
        break;

      case ID_ORIENT_ALPHABET:
      case ID_ORIENT_LR:
      case ID_ORIENT_RL:
      case ID_ORIENT_TB:
      case ID_ORIENT_BT:
        if(wmId==ID_ORIENT_ALPHABET) {
          m_pDasher->SetLongParameter(LP_ORIENTATION, Opts::Alphabet);
        } else if(wmId==ID_ORIENT_LR) {
          m_pDasher->SetLongParameter(LP_ORIENTATION, Opts::LeftToRight);
        } else if(wmId==ID_ORIENT_RL) {
          m_pDasher->SetLongParameter(LP_ORIENTATION, Opts::RightToLeft);
        } else if(wmId==ID_ORIENT_TB) {
          m_pDasher->SetLongParameter(LP_ORIENTATION, Opts::TopToBottom);
        } else if(wmId==ID_ORIENT_BT) {
          m_pDasher->SetLongParameter(LP_ORIENTATION, Opts::BottomToTop);
        } else {  // If not any of these, this is the default setting
          m_pDasher->SetLongParameter(LP_ORIENTATION, Opts::LeftToRight);
        }
        break;
        
      // FIXME - These options shouldn't pass through the interface
      // !!! Need to make this work. Probably get interface to tell edit box how to save and screen how to display
      case ID_SAVE_AS_USER_CODEPAGE:
      case ID_SAVE_AS_ALPHABET_CODEPAGE:
      case ID_SAVE_AS_UTF8:
      case ID_SAVE_AS_UTF16_LITTLE:
      case ID_SAVE_AS_UTF16_BIG:
        if(wmId==ID_SAVE_AS_USER_CODEPAGE) {
          m_pDasher->SetLongParameter(LP_FILE_ENCODING, Opts::UserDefault);
        } else if(wmId==ID_SAVE_AS_ALPHABET_CODEPAGE) {
          m_pDasher->SetLongParameter(LP_FILE_ENCODING, Opts::AlphabetDefault);
        } else if(wmId==ID_SAVE_AS_UTF8) {
          m_pDasher->SetLongParameter(LP_FILE_ENCODING, Opts::UTF8);
        } else if(wmId==ID_SAVE_AS_UTF16_LITTLE) {
          m_pDasher->SetLongParameter(LP_FILE_ENCODING, Opts::UTF16LE);
        } else if(wmId==ID_SAVE_AS_UTF16_BIG) {
          m_pDasher->SetLongParameter(LP_FILE_ENCODING, Opts::UTF16BE);
        } else {// If not any of these, this is the default setting
          m_pDasher->SetLongParameter(LP_FILE_ENCODING, Opts::UserDefault);
        }
        break;
      default:
        return DefWindowProc(m_hwnd, message, wParam, lParam);
      }
    }
    PopulateSettings();
    Layout();
    break;
  case WM_DESTROY:
    OutputDebugString(TEXT("DasherWindow WM_DESTROY\n"));

    if(m_pEdit != 0) {
      m_pEdit->write_to_file();
    }

    PostQuitMessage(0);
    break;
  case WM_GETMINMAXINFO:
    LPPOINT lppt;
    lppt = (LPPOINT) lParam;    // lParam points to array of POINTs
    lppt[3].x = 100;            // Set minimum width (arbitrary)
    // Set minimum height:
    lppt[3].y = m_pToolbar->Resize() + m_pSplitter->GetPos()
      + m_pSplitter->GetHeight() + m_pSlidebar->GetHeight() + GetSystemMetrics(SM_CYEDGE) * 10;
    break;
  case WM_INITMENUPOPUP:
    PopulateSettings();
    WinMenu.SortOut((HMENU) wParam);
    break;
  case WM_CLOSE:
    SaveWindowState();
    return DefWindowProc(m_hwnd, message, wParam, lParam);
  case WM_SIZE:
    if(wParam == SIZE_MINIMIZED)
      break;
    m_pDasher->SetLongParameter(LP_SCREEN_WIDTH, LOWORD(lParam));
    m_pDasher->SetLongParameter(LP_SCREEN_HEIGHT, HIWORD(lParam));
    Layout();
    break;
  default:
    return DefWindowProc(m_hwnd, message, wParam, lParam);
  }
  return 0;
}

Tstring CDasherWindow::CreateMyClass() {
  TCHAR *WndClassName = TEXT("DASHER");

  if(WinWrapMap::Register(WndClassName)) {
    WNDCLASSEX wndclass;
    memset(&wndclass, 0, sizeof(WNDCLASSEX));
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = 0;         //CS_HREDRAW | CS_VREDRAW; // these causes a lot of flickering
    wndclass.lpfnWndProc = WinWrapMap::WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = WinHelper::hInstApp;
    wndclass.hIcon = LoadIcon(wndclass.hInstance, (LPCTSTR) IDI_DASHER);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) (COLOR_ACTIVEBORDER + 1); // Must add one to the value we want for some unknown reason
    wndclass.lpszMenuName = (LPCTSTR) IDC_DASHER;
    wndclass.lpszClassName = WndClassName;      // Not in a resource - does not require translation
    //wndclass.hIconSm        = LoadIcon(wndclass.hInstance, (LPCTSTR)IDI_DASHER);
    // This gives a better small icon:

    m_hIconSm = (HICON) LoadImage(wndclass.hInstance, (LPCTSTR) IDI_DASHER, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

    wndclass.hIconSm = m_hIconSm;

    RegisterClassEx(&wndclass);
  }

  return Tstring(WndClassName);
}

void CDasherWindow::Layout() {
  RECT ClientRect;
  GetClientRect(m_hwnd, &ClientRect);
  const int Width = ClientRect.right;
  const int Height = ClientRect.bottom;

  int ToolbarHeight;
  if(m_pToolbar != 0)
    ToolbarHeight = m_pToolbar->Resize();
  else
    ToolbarHeight = 0;
  int CurY = ToolbarHeight;

  int SlidebarHeight;
  if(m_pSlidebar != 0)
    SlidebarHeight = m_pSlidebar->Resize(Width, Height);
  else
    SlidebarHeight = 0;
  int MaxCanvas = Height - SlidebarHeight;

  int SplitterPos = m_pSplitter->GetPos();
  int SplitterHeight = m_pSplitter->GetHeight();
  SplitterPos = max(CurY + 2 * SplitterHeight, SplitterPos);
  SplitterPos = min(SplitterPos, MaxCanvas - 3 * SplitterHeight);
  m_pSplitter->Move(SplitterPos, Width);

  m_pEdit->Move(0, CurY, Width, SplitterPos - CurY);

  CurY = SplitterPos + SplitterHeight;
  int CanvasHeight = Height - CurY - SlidebarHeight - GetSystemMetrics(SM_CYEDGE);

  m_pCanvas->Move(0, CurY, Width, CanvasHeight);

}

void CDasherWindow::PopulateSettings() {
  WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_NORMAL, false, m_pDasher->GetLongParameter(LP_DASHER_FONTSIZE)==1);
  WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_LARGE, false, m_pDasher->GetLongParameter(LP_DASHER_FONTSIZE)==2);
  WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_VERYLARGE, false, m_pDasher->GetLongParameter(LP_DASHER_FONTSIZE)==4);

  WinMenu.SetStatus(ID_TB_SHOW, false, m_pDasher->GetBoolParameter(BP_SHOW_TOOLBAR));
  WinMenu.SetStatus(ID_TB_TEXT, !m_pDasher->GetBoolParameter(BP_SHOW_TOOLBAR), m_pDasher->GetBoolParameter(BP_SHOW_TOOLBAR_TEXT));
  WinMenu.SetStatus(ID_TB_LARGE, !m_pDasher->GetBoolParameter(BP_SHOW_TOOLBAR), m_pDasher->GetBoolParameter(BP_SHOW_LARGE_ICONS));

  WinMenu.SetStatus(ID_TB_TEXT, false, m_pDasher->GetBoolParameter(BP_SHOW_TOOLBAR_TEXT));
  WinMenu.SetStatus(ID_TB_LARGE, false, m_pDasher->GetBoolParameter(BP_SHOW_LARGE_ICONS));
  WinMenu.SetStatus(ID_FIX_SPLITTER, false, m_pDasher->GetBoolParameter(BP_FIX_LAYOUT));
  WinMenu.SetStatus(ID_SHOW_SLIDE, false, m_pDasher->GetBoolParameter(BP_SHOW_SLIDER));
  WinMenu.SetStatus(ID_TIMESTAMP, false, m_pDasher->GetBoolParameter(BP_TIME_STAMP));
  WinMenu.SetStatus(ID_COPY_ALL_ON_STOP, false, m_pDasher->GetBoolParameter(BP_COPY_ALL_ON_STOP));
  WinMenu.SetStatus(ID_OPTIONS_CONTROLMODE, false, m_pDasher->GetBoolParameter(BP_CONTROL_MODE));

  WinMenu.SetStatus(ID_ORIENT_ALPHABET, false, m_pDasher->GetLongParameter(LP_ORIENTATION)==Opts::Alphabet);
  WinMenu.SetStatus(ID_ORIENT_LR,       false, m_pDasher->GetLongParameter(LP_ORIENTATION)==Opts::LeftToRight);
  WinMenu.SetStatus(ID_ORIENT_RL,       false, m_pDasher->GetLongParameter(LP_ORIENTATION)==Opts::RightToLeft);
  WinMenu.SetStatus(ID_ORIENT_TB,       false, m_pDasher->GetLongParameter(LP_ORIENTATION)==Opts::TopToBottom);
  WinMenu.SetStatus(ID_ORIENT_BT,       false, m_pDasher->GetLongParameter(LP_ORIENTATION)==Opts::BottomToTop);

  WinMenu.SetStatus(ID_SAVE_AS_USER_CODEPAGE,     false, m_pDasher->GetLongParameter(LP_FILE_ENCODING)==Opts::UserDefault);
  WinMenu.SetStatus(ID_SAVE_AS_ALPHABET_CODEPAGE, false, m_pDasher->GetLongParameter(LP_FILE_ENCODING)==Opts::AlphabetDefault);
  WinMenu.SetStatus(ID_SAVE_AS_UTF8,              false, m_pDasher->GetLongParameter(LP_FILE_ENCODING)==Opts::UTF8);
  WinMenu.SetStatus(ID_SAVE_AS_UTF16_LITTLE,      false, m_pDasher->GetLongParameter(LP_FILE_ENCODING)==Opts::UTF16LE);
  WinMenu.SetStatus(ID_SAVE_AS_UTF16_BIG,         false, m_pDasher->GetLongParameter(LP_FILE_ENCODING)==Opts::UTF16BE);
}