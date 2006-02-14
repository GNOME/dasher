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
#include "AppSettings.h"
#include "KeyboardOutput.h"

#include "../Common/WinOptions.h"

class CToolbar;
class CSlidebar;
namespace Dasher {
  class CDasher;
};

// Abstract interfaces to the Dasher engine
#include "../DasherInterface.h"

class CDasherWindow : 
  public ATL::CWindowImpl<CDasherWindow>, 
	public CSplitterOwner 
{
public:
	CDasherWindow();
	~CDasherWindow();

	DECLARE_WND_CLASS(_T("DASHER") )

	BEGIN_MSG_MAP( CDasherWindow )
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_GETMINMAXINFO,OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_INITMENUPOPUP,OnInitMenuPopup)
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus)
		MESSAGE_HANDLER(WM_DRAWITEM,OnDrawItem)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_RANGE_HANDLER(0xC000,0xFFFF,OnOther)
	END_MSG_MAP()

	LRESULT OnDrawItem(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitMenuPopup(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetMinMaxInfo(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOther(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDasherEvent(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDasherFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnTimer(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	// Create window (and children)
	// failure returns NULL
	HWND Create();

	void Show(int nCmdShow);

	int MessageLoop();

private:
 
	// Main processing function, called by MessageLoop
	void Main();

	void SaveWindowState() const;
	bool LoadWindowState();

	Dasher::CDasher *m_pDasher;

	HACCEL hAccelTable;

	// Method to set values of all settings in the menu
	void PopulateSettings();

	/// 
	/// Handle control events
	///

	void HandleControlEvent(int iID);

	void HandleParameterChange(int iParameter);

	// Widgets:
	CToolbar *m_pToolbar;
	CEdit *m_pEdit;
	CCanvas *m_pCanvas;
	CSplitter *m_pSplitter;
	CSlidebar *m_pSlidebar;
	CMenu WinMenu;
	CSplash *Splash;

	CAppSettings *m_pAppSettings;

	HICON m_hIconSm;

	LPCWSTR AutoOffset;
	LPCWSTR DialogCaption;
	char tmpAutoOffset[25];

	// Misc window handling
	void Layout();

  HWND m_hTargetWindow;
  bool m_bHaveTarget;

  CKeyboardOutput *m_pKeyboardOutput;

};

#endif /* #ifdef __MainWindow_h__ */
