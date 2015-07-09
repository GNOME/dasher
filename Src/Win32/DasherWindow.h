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

#include "../Common/WinOptions.h"

class CToolbar;
namespace Dasher {
  class CDasher;
};

// Abstract interfaces to the Dasher engine
#include "../../DasherCore/DasherInterfaceBase.h"

class CDasherWindow : 
	public ATL::CWindowImpl<CDasherWindow>, 
	public CSplitterOwner 
{
public:
	CDasherWindow();
	~CDasherWindow();

	DECLARE_WND_CLASS(_T("DASHER"))

	BEGIN_MSG_MAP( CDasherWindow )
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_GETMINMAXINFO,OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_INITMENUPOPUP,OnInitMenuPopup)
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus)
		MESSAGE_RANGE_HANDLER(0xC000,0xFFFF,OnOther)
	END_MSG_MAP()

	LRESULT OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitMenuPopup(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetMinMaxInfo(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOther(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDasherFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	// Create window (and children)
	// failure returns NULL
	HWND Create();

	void Show(int nCmdShow);

  void HandleWinEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, 
    LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

  ///ACL making these public so can be called directly from CDasher,
  /// rather than sending a windows message.
  void HandleParameterChange(int iParameter);
  
private:
 
	void SaveWindowState() const;
	bool LoadWindowState();

  bool m_bFullyCreated;
	Dasher::CDasher *m_pDasher;

	HACCEL hAccelTable;

	// Widgets:
	CToolbar *m_pToolbar;
	CEdit *m_pEdit;
	//CCanvas *m_pCanvas;
	CSplitter *m_pSplitter;
	CStatusControl *m_pSpeedAlphabetBar;
	CMenu WinMenu;
	//CSplash *Splash;

	CAppSettings *m_pAppSettings;

	HICON m_hIconSm;

	HMENU m_hMenu;

	// Misc window handling
	void Layout();
};

#endif /* #ifdef __MainWindow_h__ */
