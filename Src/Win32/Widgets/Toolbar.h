// Toolbar.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Toolbar_h__
#define __Toolbar_h__

#include "../../Common/NoClones.h"
#include "../WinHelper.h"


class CToolbar : private NoClones
{
public:
	CToolbar(HWND ParentWindow, bool Text=false, bool LargeIcons=false, bool Visible=true);
	
	enum action { SetFalse, SetTrue, Toggle, Query };
	bool SetVisible(action Cmd);
	bool ShowText(action Cmd);
	bool SetLargeIcons(action Cmd);
	int Resize();
private:
	bool Visible, Text;
	bool LargeIcons;
	HWND m_hwnd, ParentWindow;
	
	bool DoAction(bool* Property, action Cmd);
	void CreateToolbar();
	void DestroyToolbar();
};


#endif /* #ifdef __Toolbar_h__ */