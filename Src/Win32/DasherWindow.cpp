// DasherWindow.cpp
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "Common/WinMenus.h"
#include "Dasher.h"
// TODO: Put this in DasherInterfaceBase header?
#include "../DasherCore/ControlManager.h"
#include "DasherWindow.h"
#include "Widgets/AboutBox.h"
#ifndef _WIN32_WCE
#include "Widgets/Prefs.h"
#endif

#include "Widgets/Slidebar.h"
#include "Widgets/Toolbar.h"
#include "WinCommon.h"

#ifndef _WIN32_WCE
#include <Htmlhelp.h>
#include <Oleacc.h>
//#include <guiddef.h>
#endif

using namespace Dasher;
using namespace std;

#define IDT_TIMER1 200

// NOTE: There were previously various bits and pieces in this class from 
// text services framework stuff, which were never really finished. If
// required, look in version control history (prior to May 2007).

CDasherWindow::CDasherWindow() {
  m_pToolbar = 0;
  m_pEdit = 0;
  m_pSlidebar = 0;
  m_pSplitter = 0;
  m_pDasher = 0;

  m_hIconSm = (HICON) LoadImage(WinHelper::hInstApp, (LPCTSTR) IDI_DASHER, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
  
  ATL::CWndClassInfo& wc = CDasherWindow::GetWndClassInfo();
  wc.m_wc.hIcon = LoadIcon(WinHelper::hInstApp, (LPCTSTR) IDI_DASHER);
  wc.m_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.m_wc.hbrBackground = (HBRUSH) (COLOR_WINDOW); 
 #ifndef _WIN32_WCE
  wc.m_wc.lpszMenuName = (LPCTSTR) IDC_DASHER;
  wc.m_wc.hIconSm = m_hIconSm;
#endif
}

HWND CDasherWindow::Create() {
  hAccelTable = LoadAccelerators(WinHelper::hInstApp, (LPCTSTR) IDC_DASHER);

  // Get window title from resource script
  Tstring WindowTitle;
  WinLocalisation::GetResourceString(IDS_APP_TITLE, &WindowTitle);

  m_pAppSettings = new CAppSettings(0, 0);
  int iStyle(m_pAppSettings->GetLongParameter(APP_LP_STYLE));

  HWND hWnd;

#ifndef _WIN32_WCE
  if((iStyle == 1) || (iStyle == 2))
    hWnd = CWindowImpl<CDasherWindow >::Create(NULL, NULL, WindowTitle.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,  WS_EX_NOACTIVATE | WS_EX_APPWINDOW | WS_EX_TOPMOST);
  else
    hWnd = CWindowImpl<CDasherWindow >::Create(NULL, NULL, WindowTitle.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN);
#else
  hWnd = CWindowImpl<CDasherWindow, CWindow, CWinTraits<WS_CLIPCHILDREN | WS_CLIPSIBLINGS> >::Create(NULL);
#endif

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

#ifdef _WIN32_WCE
  m_pToolbar->ShowToolbar(false);
#endif

  // FIXME - the edit box really shouldn't need access to the interface, 
  // but at the moment it does, for training, blanking the display etc

  m_pEdit->SetInterface(m_pDasher);

  m_pSlidebar = new CStatusControl(m_pDasher);
  m_pSlidebar->Create(hWnd);


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
  delete m_pToolbar;
  delete m_pEdit;
  delete m_pSplitter;
  delete m_pDasher;
  delete m_pAppSettings;

  DestroyIcon(m_hIconSm);
}



void CDasherWindow::Main() {
  // TODO: Sort this sort ofthing out, figure out how it fits into ATL etc.

#ifndef _WIN32_WCE
	DASHER_ASSERT_VALIDPTR_RW(m_pDasher);
#endif

	m_pDasher->Main();
	Sleep(50); // limits framerate to 50fps
}

int CDasherWindow::MessageLoop() {
  // See previous function's comments
	MSG msg;
		
  while(GetMessage(&msg, NULL, 0, 0)) {
  	if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

  m_pDasher->StartShutdown();

  return msg.wParam;
}

void CDasherWindow::Show(int nCmdShow) {
  InvalidateRect(NULL, FALSE);
  
  if(!LoadWindowState())
    ShowWindow(nCmdShow);
}

void CDasherWindow::SaveWindowState() const {
#ifndef DASHER_WINCE
  WINDOWPLACEMENT wp;
  wp.length = sizeof(WINDOWPLACEMENT);
  
  if(GetWindowPlacement(&wp)) {//function call succeeds
    m_pAppSettings->SaveSetting("WindowState", &wp);
  }
#endif
}

bool CDasherWindow::LoadWindowState() {
#ifndef DASHER_WINCE
  WINDOWPLACEMENT wp;
  
  if(m_pAppSettings->LoadSetting("WindowState", &wp)) {
	  if(SetWindowPlacement(&wp))
      return true;
  }
#endif
  return false;
}

void CDasherWindow::HandleParameterChange(int iParameter) {
  switch(iParameter) {
#ifndef _WIN32_WCE
   case APP_BP_SHOW_TOOLBAR:
     m_pToolbar->ShowToolbar(m_pAppSettings->GetBoolParameter(APP_BP_SHOW_TOOLBAR));
     break;
#endif
   case APP_LP_STYLE:
     // TODO: No longer handled after startup?
     Layout();
     break;
   case APP_BP_TIME_STAMP:
     // TODO: reimplement
     // m_pEdit->TimeStampNewFiles(m_pAppSettings->GetBoolParameter(APP_BP_TIME_STAMP));
     break;
  case LP_MAX_BITRATE:
    // TODO: reimplement
    break;
  default:
    break;
  }
}

void CDasherWindow::HandleControlEvent(int iID) {
  // TODO: Some kind of automatic translation here? Or at least a table

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

LRESULT CDasherWindow::OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  const int wmId = LOWORD(wParam);
  const int wmEvent = HIWORD(wParam);
  
  // Tell edit box if it has changed. It should know itself really, but this is easier
  // This shouldn't be here - it should be in the edit box class
  if( m_pEdit && ((HWND) lParam == m_pEdit->GetHwnd()) && (HIWORD(wParam) == EN_CHANGE)) {
    m_pEdit->SetDirty();
    return 0;
  }
  
  // Parse the menu selections:
  // TODO: Put these into separate functions
  switch (wmId) {
  case IDM_ABOUT: {
    CAboutBox Aboutbox;
    Aboutbox.DoModal(m_hWnd);
    return 0;
  }
#ifndef _WIN32_WCE
  case ID_OPTIONS_PREFS: {
    CPrefs Prefs(m_hWnd, m_pDasher, m_pAppSettings);
    return 0;
  }
#endif
#ifndef _WIN32_WCE
  case ID_HELP_CONTENTS:
    HtmlHelp(m_hWnd, L"Dasher.chm", HH_DISPLAY_INDEX, NULL);
    return 0;
#endif
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
      CUserLogBase* pUserLog = m_pDasher->GetUserLogPtr();
      if (pUserLog != NULL)
	pUserLog->NewTrial();
      
      m_pDasher->SetBuffer(0);
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
	  // Game temp comment for compiling
    //m_pDasher->TrainFile(m_pEdit->Import(),0,0);
    return 0;
  default:
    return DefWindowProc(message, wParam, lParam);
  }
 
  Layout();
  return 0;
}

LRESULT CDasherWindow::OnDasherEvent(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  // Apparently putting the typecast directly in the switch doesn't work
  CEvent *pEvent( (CEvent *)lParam );
  
  // TODO: return if handled?
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
  PostQuitMessage(0);
	return 0;
}

#ifndef _WIN32_WCE
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
#endif

LRESULT CDasherWindow::OnInitMenuPopup(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
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
//  m_pSlidebar->Resize();
  
  Layout();
  
  return 0;
}


LRESULT CDasherWindow::OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  m_pDasher->TakeFocus();
  return 0;
}

LRESULT CDasherWindow::OnOther(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  if (message == WM_DASHER_EVENT)
    return OnDasherEvent( message, wParam, lParam, bHandled);
  else if (message == WM_DASHER_FOCUS)
    return OnDasherFocus(message, wParam, lParam, bHandled);
  else if (message == DASHER_SHOW_PREFS) {
#ifndef _WIN32_WCE
    CPrefs Prefs(m_hWnd, m_pDasher, m_pAppSettings);
#endif
  }
  return 0;
}

void CDasherWindow::Layout() {
  int iStyle(m_pAppSettings->GetLongParameter(APP_LP_STYLE));
  
  // Set up the window properties
#ifndef _WIN32_WCE
  if((iStyle == 1) || (iStyle == 2)) {
    SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_NOACTIVATE | WS_EX_APPWINDOW);
    SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }
  else {
    SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) & !WS_EX_NOACTIVATE);
    SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }
#endif

  // Now do the actual layout

  bool bHorizontal(iStyle == 1); 
  bool bShowEdit(iStyle != 2);

  RECT ClientRect;
  GetClientRect( &ClientRect);
  const int Width = ClientRect.right;
  const int Height = ClientRect.bottom;

  int ToolbarHeight;
  if(m_pToolbar && m_pAppSettings->GetBoolParameter(APP_BP_SHOW_TOOLBAR))
    ToolbarHeight = m_pToolbar->GetHeight();
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
      if(m_pDasher)
        m_pDasher->Move(0, CurY, Width / 2, MaxCanvas - CurY);

      if(m_pEdit)
        m_pEdit->Move(Width / 2, CurY, Width / 2, MaxCanvas - CurY);
    }
    else {
      if(bShowEdit) {
        m_pEdit->ShowWindow(SW_SHOW);
        m_pSplitter->ShowWindow(SW_SHOW);

        if(m_pEdit)
          m_pEdit->Move(2, CurY + 2, Width - 4, SplitterPos - CurY - 2);

        CurY = SplitterPos + SplitterHeight;
      }
      else {
        m_pEdit->ShowWindow(SW_HIDE);
        m_pSplitter->ShowWindow(SW_HIDE);
      }

      int CanvasHeight = Height - CurY - SlidebarHeight - GetSystemMetrics(SM_CYEDGE);
 
      if(m_pDasher)
        m_pDasher->Move(2, CurY+2, Width-4, CanvasHeight-2);

      if(m_pSlidebar)
        m_pSlidebar->MoveWindow(2, Height - SlidebarHeight, Width-4, SlidebarHeight);
     }


  }
}

#ifndef _WIN32_WCE
void CDasherWindow::HandleWinEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
  // Ignore events if not in direct mode
  if(m_pAppSettings && (m_pAppSettings->GetLongParameter(APP_LP_STYLE) != 2))
    return;

  // For now assume all events are focus changes, so reset the buffer
  if(m_pDasher)
    m_pDasher->SetBuffer(0);
}
#endif