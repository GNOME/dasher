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
#include "Widgets/StatusControl.h"
#include "Widgets/Menu.h"
#include "Widgets/SplashScreen.h"
#include "Widgets/WindowSelect.h"
#include "DasherMouseInput.h"
#include "AppSettings.h"
#include "GameModeHelper.h"

#include "../Common/WinOptions.h"

class CToolbar;
class CGameGroup;
namespace Dasher {
  class CDasher;
};

// Abstract interfaces to the Dasher engine
#include "../../DasherCore/DasherInterfaceBase.h"

#ifdef _WIN32_WCE
class CDasherWindow : 
	public ATL::CWindowImpl<CDasherWindow, CWindow, CWinTraits< WS_CLIPCHILDREN | WS_CLIPSIBLINGS> >, 
	public CSplitterOwner 
{
#else
class CDasherWindow : 
	public ATL::CWindowImpl<CDasherWindow>, 
	public CSplitterOwner 
{
#endif
public:
//, CWindow, CFrameWinTraits>, 

	CDasherWindow();
	~CDasherWindow();

	DECLARE_WND_CLASS(_T("DASHER"))

	BEGIN_MSG_MAP( CDasherWindow )
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_GETMINMAXINFO,OnGetMinMaxInfo)
#endif
		MESSAGE_HANDLER(WM_INITMENUPOPUP,OnInitMenuPopup)
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus)
		MESSAGE_RANGE_HANDLER(0xC000,0xFFFF,OnOther)
	END_MSG_MAP()

	LRESULT OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitMenuPopup(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#ifndef _WIN32_WCE
	LRESULT OnGetMinMaxInfo(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif
	LRESULT OnOther(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDasherEvent(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGameMessage(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDasherFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	// Create window (and children)
	// failure returns NULL
	HWND Create();

	void Show(int nCmdShow);

	int MessageLoop();

#ifndef _WIN32_WCE
  void HandleWinEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, 
    LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
#endif

private:
 
	// Main processing function, called by MessageLoop
	void Main();

	void SaveWindowState() const;
	bool LoadWindowState();

	Dasher::CDasher *m_pDasher;

	HACCEL hAccelTable;

	/// 
	/// Handle control events
	///

	void HandleControlEvent(int iID);

	void HandleParameterChange(int iParameter);

	// Widgets:
	CToolbar *m_pToolbar;
	CEdit *m_pEdit;
	//CCanvas *m_pCanvas;
	CSplitter *m_pSplitter;
	CStatusControl *m_pSpeedAlphabetBar;
  CGameGroup* m_pGameGroup;
	CMenu WinMenu;
	//CSplash *Splash;

	CAppSettings *m_pAppSettings;

	HICON m_hIconSm;

	HMENU m_hMenu;

	LPCWSTR AutoOffset;
	LPCWSTR DialogCaption;
	char tmpAutoOffset[25];

	// Misc window handling
	void Layout();

  CGameModeHelper *m_pGameModeHelper;

};

#endif /* #ifdef __MainWindow_h__ */
