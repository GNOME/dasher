// AlphabetBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////



#ifndef __AlphabetBox_h__
#define __AlphabetBox_h__

#include "../WinWrap.h"
#include "../resource.h"

#include "../../DasherCore/DasherAppInterface.h"
#include "../../DasherCore/DasherSettingsInterface.h"


class CAlphabetBox : public CWinWrap
{
public:
	CAlphabetBox(HWND Parent, Dasher::CDasherAppInterface* AI,
		Dasher::CDasherSettingsInterface* SI, const std::string& CurrentAlphabet);
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	Dasher::CDasherAppInterface* m_AppInterface;
	Dasher::CDasherSettingsInterface* m_SettingsInterface;
	
	HWND CustomBox;
	
	std::vector<std::string> AlphabetList;
	std::string m_CurrentAlphabet;
	Dasher::CAlphIO::AlphInfo CurrentInfo;
	// Some status flags:
	bool Editing;
	bool EditChar;
	bool Cloning;
	unsigned int CurrentGroup;
	unsigned int CurrentChar;
	
	void PopulateList();
	void InitCustomBox();
	std::string GetControlText(HWND Dialog, int ControlID);
	void NewGroup(std::string NewGroup);
	void ShowGroups();
	void ShowGroupChars();
	void CustomCharacter(std::string Display, std::string Text, int Colour);
	bool UpdateInfo();
};


#endif /* #ifndef __AlphabetBox_h__ */
