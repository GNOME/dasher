// Toolbar.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

// This is a very much hard-wired toolbar.
// Code would need more logic and less copy and pasting to be more generally useful.

#include "Toolbar.h"
#include "../resource.h"
#include "../WinLocalisation.h"

CToolbar::CToolbar(HWND ParentWindow, bool Text, bool LargeIcons, bool Visible)
: Text(Text), Visible(Visible), LargeIcons(LargeIcons), m_hwnd(0), ParentWindow(ParentWindow)
{
	if (Visible)
		CreateToolbar();
}


bool CToolbar::SetVisible(action Cmd)
{
	switch (Cmd)
	{
	case SetFalse:
		Visible = false;
		break;
	case SetTrue:
		Visible = true;
		break;
	case Toggle:
		Visible = !(Visible);
		break;
	case Query:
		// deliberate fall through
	default:
		return Visible;
	}
	
	if (Visible)
		this->CreateToolbar();
	else
		this->DestroyToolbar();
	
	return Visible;
}



bool CToolbar::ShowText(action Cmd) { return DoAction(&Text, Cmd); }

bool CToolbar::SetLargeIcons(action Cmd) { return DoAction(&LargeIcons, Cmd); }

bool CToolbar::DoAction(bool* Property, action Cmd)
{
	switch (Cmd)
	{
	case SetFalse:
		*Property = false;
		break;
	case SetTrue:
		*Property = true;
		break;
	case Toggle:
		*Property = !(*Property);
		break;
	case Query:
		// deliberate fall through
	default:
		return *Property;
	}
	
	if (Visible) {
		this->DestroyToolbar();
		this->CreateToolbar();
	}
	
	return *Property;
}


int CToolbar::Resize()
{
	// Makes the toolbar fit the buttons and fill the width of the window
	// Return height of toolbar for information
	
	if (Visible) {
		SendMessage(m_hwnd, TB_AUTOSIZE, 0, 0);
		RECT TB_rect;
		GetWindowRect(m_hwnd, &TB_rect);
		return(TB_rect.bottom - TB_rect.top);
	} else {
		return 0;
	}
}


void CToolbar::CreateToolbar()
{
	Visible = true;
	
	WinHelper::InitCommonControlLib();
	
	// Create Toolbar
#ifdef OriginalWin95
	m_hwnd = CreateWindow(TOOLBARCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
		ParentWindow, NULL, WinHelper::hInstApp, NULL);
#else
	//Unless I'm going for ultra-compatibility, display a nice flat-style toolbar
	m_hwnd = CreateWindow(TOOLBARCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT, 0, 0, 0, 0,
		ParentWindow, NULL, WinHelper::hInstApp, NULL);
#endif
	
	// Allows system to work with any version of common controls library
	SendMessage(m_hwnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	
	// Get Standard toolbar bitmaps.
	TBADDBITMAP bitmaps;
	bitmaps.hInst = HINST_COMMCTRL;
	if (LargeIcons==0) {
		bitmaps.nID = IDB_STD_SMALL_COLOR;
	} else {
		bitmaps.nID = IDB_STD_LARGE_COLOR;
	}
	SendMessage(m_hwnd, TB_ADDBITMAP, 0, (LPARAM)&bitmaps);
	
	// Get Non-standard Copy-All bitmap
	bitmaps.hInst = WinHelper::hInstApp;
	if (LargeIcons==0) {
		bitmaps.nID = IDB_COPY_ALL_SMALL_COLOR;
	} else {
		bitmaps.nID = IDB_COPY_ALL_LARGE_COLOR;
	}

	const int COPY_ALL_INDEX = SendMessage(m_hwnd, TB_ADDBITMAP, 0, (LPARAM)&bitmaps);

	
	// Create buttons. If you add more you MUST increase the size of the buttons array.
	// TODO: This could be more flexible. Menus and toolbars are not customisable for now.
	const int tbnum = 8;
	
	TBBUTTON buttons[tbnum];
	
	if (Text) {
		// Load strings from resource file (this could be more elegant)
		Tstring AllButtons;
		Tstring CurButton;
		WinLocalisation::GetResourceString(IDS_FILE_NEW, &CurButton);
		AllButtons += CurButton + TEXT('\0');
		WinLocalisation::GetResourceString(IDS_FILE_OPEN, &CurButton);
		AllButtons += CurButton + TEXT('\0');
		WinLocalisation::GetResourceString(IDS_FILE_SAVE, &CurButton);
		AllButtons += CurButton + TEXT('\0');
		WinLocalisation::GetResourceString(IDS_EDIT_CUT, &CurButton);
		AllButtons += CurButton + TEXT('\0');
		WinLocalisation::GetResourceString(IDS_EDIT_COPY, &CurButton);
		AllButtons += CurButton + TEXT('\0');
		WinLocalisation::GetResourceString(IDS_EDIT_COPY_ALL, &CurButton);
		AllButtons += CurButton + TEXT('\0');
		WinLocalisation::GetResourceString(IDS_EDIT_PASTE, &CurButton);
		AllButtons += CurButton + TEXT('\0') + TEXT('\0');
		const TCHAR* buttontext = AllButtons.c_str();

		// If we didn't use resource files, it would look like this:
		//TCHAR buttontext[]=TEXT("New\0Open\0Save\0Cut\0Copy\0Paste\0");
		
		SendMessage(m_hwnd, TB_ADDSTRING, 0, (DWORD)buttontext);
	}
	
	// TODO: Should do tooltips
	
	buttons[0].iBitmap = STD_FILENEW;
	buttons[0].idCommand = ID_FILE_NEW;
	buttons[0].fsState = TBSTATE_ENABLED;
	buttons[0].fsStyle = TBSTYLE_BUTTON;
	buttons[0].iString = 0;
	
	buttons[1].iBitmap = STD_FILEOPEN;
	buttons[1].idCommand = ID_FILE_OPEN;
	buttons[1].fsState = TBSTATE_ENABLED;
	buttons[1].fsStyle = TBSTYLE_BUTTON;
	buttons[1].iString = 1;
	
	buttons[2].iBitmap = STD_FILESAVE;
	buttons[2].idCommand = ID_FILE_SAVE;
	buttons[2].fsState = TBSTATE_ENABLED;
	buttons[2].fsStyle = TBSTYLE_BUTTON;
	buttons[2].iString = 2;
	
	buttons[3].iBitmap = -1;
	buttons[3].idCommand = 0;
	buttons[3].fsState = TBSTATE_INDETERMINATE;
	buttons[3].fsStyle = TBSTYLE_SEP;
	buttons[3].iString = -1;
	
	buttons[4].iBitmap = STD_CUT;
	buttons[4].idCommand = ID_EDIT_CUT;
	buttons[4].fsState = TBSTATE_ENABLED;
	buttons[4].fsStyle = TBSTYLE_BUTTON;
	buttons[4].iString = 3;
	
	buttons[5].iBitmap = STD_COPY;
	buttons[5].idCommand = ID_EDIT_COPY;
	buttons[5].fsState = TBSTATE_ENABLED;
	buttons[5].fsStyle = TBSTYLE_BUTTON;
	buttons[5].iString = 4;
	
	buttons[6].iBitmap = COPY_ALL_INDEX;
	buttons[6].idCommand = ID_EDIT_COPY_ALL;
	buttons[6].fsState = TBSTATE_ENABLED;
	buttons[6].fsStyle = TBSTYLE_BUTTON;
	buttons[6].iString = 5;

	buttons[7].iBitmap = STD_PASTE;
	buttons[7].idCommand = ID_EDIT_PASTE;
	buttons[7].fsState = TBSTATE_ENABLED;
	buttons[7].fsStyle = TBSTYLE_BUTTON;
	buttons[7].iString = 6;
	
	SendMessage(m_hwnd, TB_ADDBUTTONS, tbnum, (LPARAM)&buttons);
	
	Resize();
}


void CToolbar::DestroyToolbar()
{
	Visible = false; // Just to make sure
	
	DestroyWindow(m_hwnd);
}
