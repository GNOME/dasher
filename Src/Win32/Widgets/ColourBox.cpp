// AlphabetBox.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "ColourBox.h"
#include "../resource.h"
#include "../WinUTF8.h"
#include "../WinLocalisation.h"

#include <utility> // for std::pair

using namespace Dasher;
using namespace std;


CColourBox::CColourBox(HWND Parent, CDasherAppInterface* AI,
	CDasherSettingsInterface* SI, const string& CurrentColour)
	: m_AppInterface(AI), m_SettingsInterface(SI),
	  m_CurrentColours(CurrentColour)
{
	m_hwnd=0;
	DialogBoxParam(WinHelper::hInstApp, (LPCTSTR)IDD_COLOUR, Parent, (DLGPROC)WinWrapMap::WndProc, (LPARAM)this);
}


void CColourBox::PopulateList()
{
	HWND ListBox = GetDlgItem(m_hwnd, IDC_COLOURS);
	SendMessage(ListBox, LB_RESETCONTENT, 0, 0);
	
	m_AppInterface->GetColours(&ColourList);
	
	// Add each string to list box and index each one
	bool SelectionSet=false;
	for (unsigned int i=0; i<ColourList.size(); i++) {
		Tstring Item;
		WinUTF8::UTF8string_to_Tstring(ColourList[i], &Item, GetACP());
		LRESULT Index = SendMessage(ListBox, LB_ADDSTRING, 0, (LPARAM) Item.c_str());
		SendMessage(ListBox, LB_SETITEMDATA, Index, (LPARAM) i);
		if (ColourList[i]==m_CurrentColours) {
			SendMessage(ListBox, LB_SETCURSEL, Index, 0);
			SelectionSet=true;
		}
	}
	if (SelectionSet==false) {
		SendMessage(ListBox, LB_SETCURSEL, 0, 0);
		LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, 0, 0);
		m_CurrentColours = ColourList[CurrentIndex];
	}
	// Tell list box that we have set an item for it (so that delete and edit can be grayed if required)
	SendMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_COLOURS, LBN_SELCHANGE), 0);
}


void CColourBox::InitCustomBox()
{	
	// Sort out "spin" or "up down" buttons so that I get messages from them.
	SendMessage(GetDlgItem(CustomBox, IDC_MOVE_GROUP), UDM_SETBUDDY, (WPARAM)GetDlgItem(CustomBox, IDC_GROUP_BUDDY), 0);
	SendMessage(GetDlgItem(CustomBox, IDC_MOVE_CHAR), UDM_SETBUDDY, (WPARAM)GetDlgItem(CustomBox, IDC_CHAR_BUDDY), 0);
}


LRESULT CColourBox::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		if (!m_hwnd) { // If this is the initial dialog for the first time
			m_hwnd = Window;
			PopulateList();
		} 
		return TRUE;
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case (IDC_DISPLAY):
			if (HIWORD(wParam)==EN_CHANGE) {
				HWND Control = GetDlgItem(Window, IDC_DISPLAY);
				LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1; // +1 to allow for terminator
				TCHAR* Buffer = new TCHAR[BufferLength];
				SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM)Buffer);
				string ItemName;
				SendMessage(GetDlgItem(Window, IDC_TEXT), WM_SETTEXT, 0, (LPARAM)Buffer);
				delete[] Buffer;
			}
			break;
		case (IDC_COLOURS):
			if (HIWORD(wParam)==LBN_SELCHANGE) {
				HWND ListBox = GetDlgItem(m_hwnd, IDC_COLOURS);
				LRESULT CurrentItem = SendMessage(ListBox, LB_GETCURSEL, 0, 0);
				LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, CurrentItem, 0);
				m_CurrentColours = ColourList[CurrentIndex];
			}
			return TRUE;
			break;
		case (IDOK):
			if (m_CurrentColours!= std::string("") ) {
				m_SettingsInterface->ChangeColours(m_CurrentColours);
			}
			// deliberate fall through
		case (IDCANCEL):
		{
			EndDialog(Window, LOWORD(wParam));
			return TRUE;
		}
		break;
		}
	}
	return FALSE;
}
