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

#include "../../DasherCore/DasherSettingsInterface.h"
#include "Canvas.h"

class CKeyBox : public CWinWrap
{
public:
	CKeyBox(HWND Parent, CCanvas* m_pCanvas, Dasher::CDasherSettingsInterface *Interface);
	void PopulateWidgets();
	std::string GetControlText(HWND Dialog, int ControlID);
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HWND CustomBox;
	HWND slider;
	HWND uniformbox;
	CCanvas* m_pCanvas;
	int keycoords[18];
	int ypixels;
	int mouseposdist;
	double NewUniform;

	// Some status flags:
	bool Editing;
	bool SelectionSet;

	TCHAR m_tcBuffer[1000];
	
	Dasher::CDasherSettingsInterface* m_pSettingsInterface;
};


#endif /* #ifndef __AlphabetBox_h__ */