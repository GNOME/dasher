// AlphabetBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////



#ifndef __KeyBox_h__
#define __KeyBox_h__

#include "../WinWrap.h"
#include "../resource.h"
#include "../../DasherCore/DasherTypes.h"
#include "Canvas.h"

class CKeyBox : public CWinWrap
{
public:
	CKeyBox(HWND Parent, CCanvas* m_pCanvas);
	void PopulateWidgets();
	std::string GetControlText(HWND Dialog, int ControlID);
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HWND m_hwnd;
	HWND CustomBox;
	CCanvas* m_pCanvas;
	int keycoords[8];

	// Some status flags:
	bool Editing;
	bool SelectionSet;
};


#endif /* #ifndef __AlphabetBox_h__ */