// Prefs.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////



#ifndef __PrefsBox_h__
#define __PrefsBox_h__

#include "../WinWrap.h"
#include "../resource.h"
#include "../../DasherCore/DasherTypes.h"
#include "../../DasherCore/DasherSettingsInterface.h"
#include "Canvas.h"
#include "../DasherWindow.h"

class CPrefs : public CWinWrap
{
public:
	CPrefs(HWND Parent, CCanvas* m_pCanvas, CDasherWindow* m_pWindow, Dasher::CDasherSettingsInterface* m_pSettings, Dasher::CDasherWidgetInterface* m_pWidget);
	void PopulateWidgets();
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HWND m_hwnd;
	HWND CustomBox;
	CCanvas* m_pCanvas;
	CDasherWindow* m_pWindow;
	Dasher::CDasherSettingsInterface* m_pSettings;
	Dasher::CDasherWidgetInterface* m_pWidget;
};


#endif /* #ifndef __PrefsBox_h__ */