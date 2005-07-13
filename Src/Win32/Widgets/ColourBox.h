// AlphabetBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////



#ifndef __ColourBox_h__
#define __ColourBox_h__


#include "../resource.h"

#include "../../DasherCore/DasherAppInterface.h"
#include "../../DasherCore/DasherSettingsInterface.h"
#include "../../DasherCore/ColourIO.h"


class CColourBox : public CWinWrap
{
public:
	CColourBox(HWND Parent, Dasher::CDasherAppInterface* AI,
		Dasher::CDasherSettingsInterface* SI);
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	Dasher::CDasherAppInterface* m_AppInterface;
	Dasher::CDasherSettingsInterface* m_SettingsInterface;
	
	HWND CustomBox;
	
	std::vector<std::string> ColourList;
	std::string m_CurrentColours;
	Dasher::CColourIO::ColourInfo CurrentInfo;
	// Some status flags:
	
	void PopulateList();
	void InitCustomBox();
	bool UpdateInfo();
};


#endif /* #ifndef __AlphabetBox_h__ */