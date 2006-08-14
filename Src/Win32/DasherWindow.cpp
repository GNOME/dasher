// DasherWindow.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"
#include <guiddef.h>
#include <Oleacc.h>
// Don't seem to be needed and not in VC8 platform SDK, kdv
//#include <Textstor.h>
//#include <Msctf.h>
//#include <Msaatext.h>
#include <Htmlhelp.h>

#include "DasherWindow.h"

#include "Common/WinMenus.h"

#include "../DasherCore/DasherTypes.h"
#include "../DasherCore/ControlManager.h"
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

#ifdef PJC_EXPERIMENTAL

// Bits an pieces from pjc playing with text service framework stuff

bool g_bInCallback(false);
HWND g_hWnd;
IAccClientDocMgr *g_pMgr;
VOID CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

#endif

CDasherWindow::CDasherWindow()
:Splash(0), m_pToolbar(0), m_pEdit(0), m_pSlidebar(0), m_pSplitter(0), m_pDasher(0), m_pCanvas(0) 
{

	m_hIconSm = (HICON) LoadImage(WinHelper::hInstApp, (LPCTSTR) IDI_DASHER, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

	ATL::CWndClassInfo& wc = CDasherWindow::GetWndClassInfo();
	wc.m_wc.hIcon = LoadIcon(WinHelper::hInstApp, (LPCTSTR) IDI_DASHER);
	wc.m_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.m_wc.hbrBackground = (HBRUSH) (COLOR_ACTIVEBORDER + 1); // Must add one to the value we want for some unknown reason
    wc.m_wc.lpszMenuName = (LPCTSTR) IDC_DASHER;
//    wc.m_wc.lpszClassName = WndClassName;      // Not in a resource - does not require translation
	wc.m_wc.hIconSm        = m_hIconSm;
	
#ifdef PJC_EXPERIMENTAL

// Create an acc manager foobar

  CLSID MyClsid;

  MyClsid.Data1 = 0xFC48CC30;
  MyClsid.Data2 = 0x4F3E;
  MyClsid.Data3 = 0x4fa1;
  MyClsid.Data4[0] = 0x80;
  MyClsid.Data4[1] = 0x3B;
  MyClsid.Data4[2] = 0xAD;
  MyClsid.Data4[3] = 0x0E;
  MyClsid.Data4[4] = 0x19;
  MyClsid.Data4[5] = 0x6A;
  MyClsid.Data4[6] = 0x83;
  MyClsid.Data4[7] = 0xB1;

  IID MyIid;
 
  MyIid.Data1 = 0x4C896039;
  MyIid.Data2 = 0x7B6D;
  MyIid.Data3 = 0x49e6;
  MyIid.Data4[0] = 0xA8;
  MyIid.Data4[1] = 0xC1;
  MyIid.Data4[2] = 0x45;
  MyIid.Data4[3] = 0x11;
  MyIid.Data4[4] = 0x6A;
  MyIid.Data4[5] = 0x98;
  MyIid.Data4[6] = 0x29;
  MyIid.Data4[7] = 0x2B;

  
  HRESULT Foo = CoCreateInstance(MyClsid, 0, CLSCTX_ALL, MyIid, (void**)&g_pMgr);
  g_pMgr->AddRef();
  // Set up callback for WinEvents
  
  HWINEVENTHOOK hEventHook;

  hEventHook = SetWinEventHook(EVENT_OBJECT_FOCUS,             // Get all events.
                               EVENT_OBJECT_FOCUS,
                               GetModuleHandle(NULL), // Use this module
                               WinEventProc,
                               0,                     // All processes
                               0,                     // All threads
                               WINEVENT_OUTOFCONTEXT);
  if(!hEventHook) {
    MessageBox(NULL, L"Initialisation of WinEvent hook failed");
  }
#endif

}

HWND CDasherWindow::Create()
{
  hAccelTable = LoadAccelerators(WinHelper::hInstApp, (LPCTSTR) IDC_DASHER);

  // Get window title from resource script
  Tstring WindowTitle;
  WinLocalisation::GetResourceString(IDS_APP_TITLE, &WindowTitle);

  m_pAppSettings = new CAppSettings(0, 0);
  int iStyle(m_pAppSettings->GetLongParameter(APP_LP_STYLE));

  HWND hWnd;
  if((iStyle == 1) || (iStyle == 2))
    hWnd = CWindowImpl<CDasherWindow >::Create(NULL, NULL, WindowTitle.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,  WS_EX_NOACTIVATE | WS_EX_APPWINDOW | WS_EX_TOPMOST);
  else
    hWnd = CWindowImpl<CDasherWindow >::Create(NULL, NULL, WindowTitle.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN);

  // Create Widgets
  m_pDasher = new CDasher(hWnd);

  // Create a CAppSettings
  m_pAppSettings->SetHwnd(hWnd);
  m_pAppSettings->SetDasher(m_pDasher);

  m_pEdit = new CEdit(m_pAppSettings);
  m_pEdit->Create(hWnd, m_pAppSettings->GetBoolParameter(APP_BP_TIME_STAMP));
  m_pEdit->SetFont(m_pAppSettings->GetStringParameter(APP_SP_EDIT_FONT), m_pAppSettings->GetLongParameter(APP_LP_EDIT_FONT_SIZE));

#ifdef PJC_EXPERIMENTAL
  g_hWnd = m_pEdit->GetHwnd();
#endif

  m_pToolbar = new CToolbar(hWnd, m_pAppSettings->GetBoolParameter(APP_BP_SHOW_TOOLBAR));

  // FIXME - the edit box really shouldn't need access to the interface, 
  // but at the moment it does, for training, blanking the display etc

  m_pEdit->SetInterface(m_pDasher);

  // FIXME - we shouldn't need to know about these outside of CDasher

  m_pCanvas = m_pDasher->GetCanvas();

  m_pSlidebar = new CSlidebar(hWnd, m_pDasher);


  m_pSplitter = new CSplitter(this,100);
  
  HWND hSplitter =  m_pSplitter->Create(hWnd);
  
  if (!hSplitter)
	  return 0;

  // Add extra control nodes

  m_pDasher->RegisterNode( Dasher::CControlManager::CTL_USER, "Speak", -1 );
  m_pDasher->RegisterNode( Dasher::CControlManager::CTL_USER+1, "All", -1 );
  m_pDasher->RegisterNode( Dasher::CControlManager::CTL_USER+2, "New", -1 );
  m_pDasher->RegisterNode( Dasher::CControlManager::CTL_USER+3, "Repeat", -1 );

  m_pDasher->ConnectNode(Dasher::CControlManager::CTL_USER, Dasher::CControlManager::CTL_ROOT, -2);

  m_pDasher->ConnectNode(Dasher::CControlManager::CTL_USER+1, Dasher::CControlManager::CTL_USER, -2);
  m_pDasher->ConnectNode(Dasher::CControlManager::CTL_USER+2, Dasher::CControlManager::CTL_USER, -2);
  m_pDasher->ConnectNode(Dasher::CControlManager::CTL_USER+3, Dasher::CControlManager::CTL_USER, -2);
 
  m_pDasher->ConnectNode(-1, Dasher::CControlManager::CTL_USER+1, -2);
  m_pDasher->ConnectNode(Dasher::CControlManager::CTL_ROOT, Dasher::CControlManager::CTL_USER+1, -2);
  
  m_pDasher->ConnectNode(-1, Dasher::CControlManager::CTL_USER+2, -2);
  m_pDasher->ConnectNode(Dasher::CControlManager::CTL_ROOT, Dasher::CControlManager::CTL_USER+2, -2);

  m_pDasher->ConnectNode(-1, Dasher::CControlManager::CTL_USER+3, -2);
  m_pDasher->ConnectNode(Dasher::CControlManager::CTL_ROOT, Dasher::CControlManager::CTL_USER+3, -2);

  m_pGameModeHelper = 0;

  return hWnd;
}

CDasherWindow::~CDasherWindow() {
  delete Splash;                // In case Show() was never called.
  delete m_pToolbar;
  delete m_pEdit;
  delete m_pSplitter;
  delete m_pDasher;
  delete m_pAppSettings;
  DestroyIcon(m_hIconSm);
}



void CDasherWindow::Main()
{
	DASHER_ASSERT_VALIDPTR_RW(m_pDasher);
	m_pDasher->Main();
	Sleep(20); // limits framerate to 50fps
}

int CDasherWindow::MessageLoop() 
{
	MSG msg;

	while( 1 )
	{
		
		while (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if (msg.message == WM_QUIT)
				return msg.wParam;

			if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		Main();
    } 

}

/////////////////////////////////////////////////////////////////////////////

void CDasherWindow::Show(int nCmdShow) 
{
  // Make sure Dasher has started up.
  m_pDasher->Start();

  // Clear SplashScreen
  delete Splash;
  Splash = 0;

  // Show Window
  InvalidateRect( NULL, FALSE);
  if(!LoadWindowState())
    ShowWindow(nCmdShow);       // Now set up. Kill splash screen and display main window

}

void CDasherWindow::SaveWindowState() const 
{
#ifndef DASHER_WINCE
  WINDOWPLACEMENT wp;
  wp.length = sizeof(WINDOWPLACEMENT);
  
  if(GetWindowPlacement( &wp)) {//function call succeeds
    m_pAppSettings->SaveSetting("WindowState", &wp);
  }
#endif
}

bool CDasherWindow::LoadWindowState() {
#ifndef DASHER_WINCE
  WINDOWPLACEMENT wp;
  
  if(m_pAppSettings->LoadSetting("WindowState", &wp)) 
  {
	if(SetWindowPlacement(&wp))
      return true;
  }
#endif
  return false;
}

void CDasherWindow::HandleParameterChange(int iParameter) {
  switch(iParameter) {
   case APP_BP_SHOW_TOOLBAR:
     m_pToolbar->ShowToolbar(m_pAppSettings->GetBoolParameter(APP_BP_SHOW_TOOLBAR));
     break;
   case APP_LP_STYLE:
     Layout();
     break;
   case APP_BP_TIME_STAMP:
     // TODO: reimplement
     // m_pEdit->TimeStampNewFiles(m_pAppSettings->GetBoolParameter(APP_BP_TIME_STAMP));
     break;
	 case LP_MAX_BITRATE:
    // TODO: reimplement
	  break;
   case APP_SP_EDIT_FONT:
   case APP_LP_EDIT_FONT_SIZE:
     m_pEdit->SetFont(m_pAppSettings->GetStringParameter(APP_SP_EDIT_FONT), m_pAppSettings->GetLongParameter(APP_LP_EDIT_FONT_SIZE));
     break;
  }
}

void CDasherWindow::HandleControlEvent(int iID) {
  switch(iID) {
  case Dasher::CControlManager::CTL_MOVE_FORWARD_CHAR:
    if(m_pEdit)
      m_pEdit->Move(EDIT_FORWARDS, EDIT_CHAR);
    break;
  case Dasher::CControlManager::CTL_MOVE_FORWARD_WORD:
    if(m_pEdit)
      m_pEdit->Move(EDIT_FORWARDS, EDIT_WORD);
    break;
  case Dasher::CControlManager::CTL_MOVE_FORWARD_LINE:
    if(m_pEdit)
      m_pEdit->Move(EDIT_FORWARDS, EDIT_LINE);
    break;
  case Dasher::CControlManager::CTL_MOVE_FORWARD_FILE: 
    if(m_pEdit)
      m_pEdit->Move(EDIT_FORWARDS, EDIT_FILE);
    break;
  case Dasher::CControlManager::CTL_MOVE_BACKWARD_CHAR:
    if(m_pEdit)
      m_pEdit->Move(EDIT_BACKWARDS, EDIT_CHAR);
    break;
  case Dasher::CControlManager::CTL_MOVE_BACKWARD_WORD:
    if(m_pEdit)
      m_pEdit->Move(EDIT_BACKWARDS, EDIT_WORD);
    break;
  case Dasher::CControlManager::CTL_MOVE_BACKWARD_LINE:
    if(m_pEdit)
      m_pEdit->Move(EDIT_BACKWARDS, EDIT_LINE);    
    break;
  case Dasher::CControlManager::CTL_MOVE_BACKWARD_FILE:
    if(m_pEdit)
      m_pEdit->Move(EDIT_BACKWARDS, EDIT_FILE);
    break;
  case Dasher::CControlManager::CTL_DELETE_FORWARD_CHAR:
    if(m_pEdit)
      m_pEdit->Delete(EDIT_FORWARDS, EDIT_CHAR);
    break;
  case Dasher::CControlManager::CTL_DELETE_FORWARD_WORD:
    if(m_pEdit)
      m_pEdit->Delete(EDIT_FORWARDS, EDIT_WORD);
    break;
  case Dasher::CControlManager::CTL_DELETE_FORWARD_LINE:
    if(m_pEdit)
      m_pEdit->Delete(EDIT_FORWARDS, EDIT_LINE);
    break;
  case Dasher::CControlManager::CTL_DELETE_FORWARD_FILE:
    if(m_pEdit)
      m_pEdit->Delete(EDIT_FORWARDS, EDIT_FILE);
    break;
  case Dasher::CControlManager::CTL_DELETE_BACKWARD_CHAR:
    if(m_pEdit)
      m_pEdit->Delete(EDIT_BACKWARDS, EDIT_CHAR);
    break;
  case Dasher::CControlManager::CTL_DELETE_BACKWARD_WORD:
    if(m_pEdit)
      m_pEdit->Delete(EDIT_BACKWARDS, EDIT_WORD);
    break;
  case Dasher::CControlManager::CTL_DELETE_BACKWARD_LINE:
    if(m_pEdit)
      m_pEdit->Delete(EDIT_BACKWARDS, EDIT_LINE);
    break;
  case Dasher::CControlManager::CTL_DELETE_BACKWARD_FILE:
    if(m_pEdit)
      m_pEdit->Delete(EDIT_BACKWARDS, EDIT_FILE);
    break;
  case Dasher::CControlManager::CTL_USER+1: // Speak all
    if(m_pEdit)
      m_pEdit->speak(1);
    break;
  case Dasher::CControlManager::CTL_USER+2: // Speak new
    if(m_pEdit)
      m_pEdit->speak(2);
    break;
  case Dasher::CControlManager::CTL_USER+3: // Repeat speech
    if(m_pEdit)
      m_pEdit->speak(3);
    break;
  }
}



LRESULT CDasherWindow::OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	const int wmId = LOWORD(wParam);
	const int wmEvent = HIWORD(wParam);

	// Tell edit box if it has changed. It should know itself really, but this is easier
	if( m_pEdit && ((HWND) lParam == m_pEdit->GetHwnd()) && (HIWORD(wParam) == EN_CHANGE)) {
		m_pEdit->SetDirty();
		return 0;
	}

	RECT windowsize;

	// Parse the menu selections:
	switch (wmId) 
	{
	  case ID_OPTIONS_FONTSIZE_NORMAL:
		  m_pDasher->SetLongParameter(LP_DASHER_FONTSIZE, Dasher::Opts::FontSize(1));
		  break;
	  case ID_OPTIONS_FONTSIZE_LARGE:
		  m_pDasher->SetLongParameter(LP_DASHER_FONTSIZE, Dasher::Opts::FontSize(2));
		  break;
	  case ID_OPTIONS_FONTSIZE_VERYLARGE:
		  m_pDasher->SetLongParameter(LP_DASHER_FONTSIZE, Dasher::Opts::FontSize(4));
		  break;
	  case ID_OPTIONS_EDITFONT:{
		  CHOOSEFONT Data;
		  LOGFONT lf;
      HFONT Font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
      GetObject(Font, sizeof(LOGFONT), &lf);
      Tstring tstrFaceName;
      WinUTF8::UTF8string_to_wstring(m_pAppSettings->GetStringParameter(APP_SP_EDIT_FONT), tstrFaceName);
      _tcscpy(lf.lfFaceName, tstrFaceName.c_str());
      lf.lfHeight = m_pAppSettings->GetLongParameter(APP_LP_EDIT_FONT_SIZE);
		  Data.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
		  Data.lStructSize = sizeof(CHOOSEFONT);
		  Data.hwndOwner = m_hWnd;
		  Data.lpLogFont = &lf;
      if(ChooseFont(&Data)) {
		    string FontName;
		    WinUTF8::wstring_to_UTF8string(lf.lfFaceName, FontName);
		    m_pAppSettings->SetStringParameter(APP_SP_EDIT_FONT, FontName);
        m_pAppSettings->SetLongParameter(APP_LP_EDIT_FONT_SIZE, lf.lfHeight);
      }
	    }
			break;
	  case ID_OPTIONS_DASHERFONT:
		  {
		  CHOOSEFONT Data;
		  LOGFONT lf;
		  HFONT Font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
		  GetObject(Font, sizeof(LOGFONT), &lf);
      Tstring tstrFaceName;
      WinUTF8::UTF8string_to_wstring(m_pAppSettings->GetStringParameter(SP_DASHER_FONT), tstrFaceName);
      _tcscpy(lf.lfFaceName, tstrFaceName.c_str());
		  Data.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
		  Data.lStructSize = sizeof(CHOOSEFONT);
		  Data.hwndOwner = m_hWnd;
		  Data.lpLogFont = &lf;
      if(ChooseFont(&Data)) {
		    string FontName;
 		    WinUTF8::wstring_to_UTF8string(lf.lfFaceName, FontName);
  		  m_pAppSettings->SetStringParameter(SP_DASHER_FONT, FontName);
      }
		 }
		 return 0;
	  case ID_OPTIONS_RESETFONT:
		  m_pAppSettings->ResetParamater(SP_DASHER_FONT);
		  m_pAppSettings->ResetParamater(APP_SP_EDIT_FONT);
		  return 0;
	  case IDM_ABOUT: {
			  CAboutBox Aboutbox;
			  Aboutbox.DoModal(m_hWnd);
			  return 0;
		  }
	  case ID_OPTIONS_PREFS: {
		    CPrefs Prefs(m_hWnd, m_pDasher, m_pAppSettings);
		    return 0;
		  }
	  case ID_HELP_CONTENTS:
      HtmlHelp(m_hWnd, L"Dasher.chm", HH_DISPLAY_INDEX, NULL);
		  return 0;
    case ID_HELP_DASHERTUTORIAL:
      m_pGameModeHelper = new CGameModeHelper(m_pDasher);
      return 0;
	  case IDM_EXIT:
		  DestroyWindow();
		  return 0;
	  case ID_EDIT_SELECTALL:
		  if(m_pEdit)
			  m_pEdit->SelectAll();
		  return 0;
	  case ID_EDIT_CUT:
		  if(m_pEdit)
			  m_pEdit->Cut();
		  return 0;
	  case ID_EDIT_COPY:
		  if(m_pEdit)
			  m_pEdit->Copy();
		  return 0;
	  case ID_EDIT_COPY_ALL:
		  if(m_pEdit)
			  m_pEdit->CopyAll();
		  return 0;
	  case ID_EDIT_PASTE:
		  if(m_pEdit)
			  m_pEdit->Paste();
		  return 0;
	  case ID_FILE_NEW:
		  if(m_pEdit)
			  m_pEdit->New();
		  // Selecting file->new indicates a new trial to our user logging object
		  if (m_pDasher != NULL) {
			  CUserLog* pUserLog = m_pDasher->GetUserLogPtr();
			  if (pUserLog != NULL)
				  pUserLog->NewTrial();
		  }
		  return 0;
	  case ID_FILE_OPEN:
		  if(m_pEdit)
			  m_pEdit->Open();
		  return 0;
	  case ID_FILE_SAVE:
		  if(m_pEdit)
			  if(!m_pEdit->Save())
				  m_pEdit->SaveAs();
		  return 0;
	  case ID_FILE_SAVE_AS:
		  if(m_pEdit)
			  m_pEdit->SaveAs();
		  return 0;
	  case ID_IMPORT_TRAINFILE:
      // TODO: Fix dummy arguments
		  m_pDasher->TrainFile(m_pEdit->Import(),0,0);
		  return 0;
    default:
		  return DefWindowProc(message, wParam, lParam);
	}

	PopulateSettings();
	Layout();
	return 0;
}

LRESULT CDasherWindow::OnDasherEvent(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// Apparently putting the typecast directly in the switch doesn't work
  CEvent *pEvent( (CEvent *)lParam );

  switch(pEvent->m_iEventType) {
    case EV_PARAM_NOTIFY:
      HandleParameterChange(((CParameterNotificationEvent *)pEvent)->m_iParameter);
      break;
    case EV_CONTROL:
      HandleControlEvent(((CControlEvent *)pEvent)->m_iID);
      break;
    case EV_EDIT: 
      if(m_pGameModeHelper) {
        Dasher::CEditEvent * pEvt(static_cast< Dasher::CEditEvent * >(pEvent));

        switch (pEvt->m_iEditType) {
          case 1:
            m_pGameModeHelper->Output(pEvt->m_sText);
            break;
          case 2:
            m_pGameModeHelper->Delete(pEvt->m_sText.size());
            break;
        }
      }
      break;


    default:
      break;
  }
  
  if(m_pEdit) 
     m_pEdit->HandleEvent(pEvent);
  
	return 0;
}

LRESULT CDasherWindow::OnDasherFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	::SetFocus(m_pEdit->GetHwnd());

  // TODO: Is this obsolete?
  HWND *pHwnd((HWND *)lParam);
  m_pEdit->SetKeyboardTarget(*pHwnd);
	return 0;
}

LRESULT CDasherWindow::OnDestroy(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(m_pEdit != 0) {
		m_pEdit->write_to_file();
	}

  PostQuitMessage(0);
	return 0;
}

LRESULT CDasherWindow::OnGetMinMaxInfo(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// not yet created
	if (m_pToolbar == 0 || m_pSplitter == 0 || m_pSlidebar == 0)
		return 0;

	bHandled = TRUE;
	LPPOINT lppt;
    lppt = (LPPOINT) lParam;    // lParam points to array of POINTs
    lppt[3].x = 100;            // Set minimum width (arbitrary)
    // Set minimum height:
    if(m_pAppSettings->GetBoolParameter(APP_BP_SHOW_TOOLBAR))
      lppt[3].y = m_pToolbar->GetHeight() + m_pSplitter->GetPos()
        + m_pSplitter->GetHeight() + m_pSlidebar->GetHeight() + GetSystemMetrics(SM_CYEDGE) * 10;
    else
      lppt[3].y = m_pSplitter->GetPos()
        + m_pSplitter->GetHeight() + m_pSlidebar->GetHeight() + GetSystemMetrics(SM_CYEDGE) * 10;

	return 0;
}

LRESULT CDasherWindow::OnInitMenuPopup(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	PopulateSettings();
	WinMenu.SortOut((HMENU) wParam);
	return 0;
}

LRESULT CDasherWindow::OnClose(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  // TODO: Prompt for confirmation here
	SaveWindowState();
  DestroyWindow();
	return 0;
}

LRESULT CDasherWindow::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(wParam == SIZE_MINIMIZED)
    return 0;
    
  m_pToolbar->Resize();
  m_pSlidebar->Resize();

  Layout();

	return 0;
}


LRESULT CDasherWindow::OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	::SetFocus(m_pCanvas->getwindow());
 	return 0;
}

LRESULT CDasherWindow::OnOther(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (message == WM_DASHER_EVENT)
		return OnDasherEvent( message, wParam, lParam, bHandled);
	else if (message == WM_DASHER_FOCUS)
		return OnDasherFocus(message, wParam, lParam, bHandled);
  else if (message == DASHER_SHOW_PREFS) {
    CPrefs Prefs(m_hWnd, m_pDasher, m_pAppSettings);
  }
	return 0;
}

void CDasherWindow::Layout() {
  int iStyle(m_pAppSettings->GetLongParameter(APP_LP_STYLE));
  
  // Set up the window properties

  if((iStyle == 1) || (iStyle == 2)) {
    SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_NOACTIVATE | WS_EX_APPWINDOW);
    SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }
  else {
    SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) & !WS_EX_NOACTIVATE);
    SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }

  // Now do the actual layout

  bool bHorizontal(iStyle == 1); 
  bool bShowEdit(iStyle != 2);

  RECT ClientRect;
  GetClientRect( &ClientRect);
  const int Width = ClientRect.right;
  const int Height = ClientRect.bottom;

  int ToolbarHeight;
  if((m_pToolbar != 0) && m_pAppSettings->GetBoolParameter(APP_BP_SHOW_TOOLBAR))
    ToolbarHeight = m_pToolbar->GetHeight() + 2;
  else
    ToolbarHeight = 0;
  int CurY = ToolbarHeight;

  int SlidebarHeight;
  if(m_pSlidebar != 0)
    SlidebarHeight = m_pSlidebar->GetHeight();
  else
    SlidebarHeight = 0;
  int MaxCanvas = Height - SlidebarHeight;

  if(m_pSplitter) {
    int SplitterPos = m_pSplitter->GetPos();
    int SplitterHeight = m_pSplitter->GetHeight();
    SplitterPos = max(CurY + 2 * SplitterHeight, SplitterPos);
    SplitterPos = min(SplitterPos, MaxCanvas - 3 * SplitterHeight);
    m_pSplitter->Move(SplitterPos, Width);

    if(bHorizontal) {
      if(m_pCanvas)
        m_pCanvas->Move(0, CurY, Width / 2, MaxCanvas - CurY);

      if(m_pEdit)
        m_pEdit->Move(Width / 2, CurY, Width / 2, MaxCanvas - CurY);
    }
    else {
      if(bShowEdit) {
        m_pEdit->ShowWindow(SW_SHOW);
        m_pSplitter->ShowWindow(SW_SHOW);

        if(m_pEdit)
          m_pEdit->Move(0, CurY, Width, SplitterPos - CurY);

        CurY = SplitterPos + SplitterHeight;
      }
      else {
        m_pEdit->ShowWindow(SW_HIDE);
        m_pSplitter->ShowWindow(SW_HIDE);
      }

      int CanvasHeight = Height - CurY - SlidebarHeight - GetSystemMetrics(SM_CYEDGE);
 
      if(m_pCanvas)
        m_pCanvas->Move(0, CurY, Width, CanvasHeight);
     }
  }
}

void CDasherWindow::PopulateSettings() {
  if(m_pDasher) {
    WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_NORMAL, false, m_pDasher->GetLongParameter(LP_DASHER_FONTSIZE)==1);
    WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_LARGE, false, m_pDasher->GetLongParameter(LP_DASHER_FONTSIZE)==2);
    WinMenu.SetStatus(ID_OPTIONS_FONTSIZE_VERYLARGE, false, m_pDasher->GetLongParameter(LP_DASHER_FONTSIZE)==4);
  }
}

#ifdef PJC_EXPERIMENTAL

VOID CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {

  if(hwnd == g_hWnd)
    return;

  if(!g_bInCallback) {
    g_bInCallback = true;
    VARIANT MyV;
    IAccessible *pMyAccessible;
    VariantInit(&MyV);

    if( AccessibleObjectFromEvent( hwnd, idObject, idChild, &pMyAccessible, &MyV) != S_OK) {
      // Do nothing
    }
    else {
      VARIANT MyV2;
      VariantInit(&MyV2);

      pMyAccessible->get_accRole(MyV, &MyV2);

//      if(MyV2.iVal == ROLE_SYSTEM_TEXT) {
        ITextStoreAnchor *pAnchor(NULL);
      HRESULT MyFoo = g_pMgr->GetFocused(IID_ITextStoreAnchor, (IUnknown **)(&pAnchor));
pAnchor->AddRef();
//        pMyAccessible->QueryInterface(IID_ITextStoreAnchor, (void **)(&pAnchor));
//        MessageBox(NULL, L"Got Text", L"Foo", MB_OK);
HRESULT MyBar;

if(pAnchor) {
  IAnchor *pStart;
  IAnchor *pEnd;


HRESULT MyLock;
HRESULT MyLock2;

IEnumUnknown *pMyEnum;

g_pMgr->GetDocuments(&pMyEnum);

if(pMyEnum) {
  pMyEnum->AddRef();
}

  MyLock = pAnchor->RequestLock(TS_LF_READWRITE, &MyLock2);

  MyBar = pAnchor->InsertTextAtSelection(TF_IAS_NOQUERY, L"Foo", 3, &pStart, &pEnd);
//MessageBox(NULL, L"Got Text", L"Foo", MB_OK);
    }

      VariantClear(&MyV2);
    }

    VariantClear(&MyV);
    g_bInCallback = false;
  }
};

#endif