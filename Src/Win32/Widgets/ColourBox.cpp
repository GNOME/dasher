// AlphabetBox.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "ColourBox.h"
#include "../resource.h"

#include <utility>              // for std::pair

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CColourBox::CColourBox(HWND Parent, CDasherInterfaceBase *DI)
:m_pDasherInterface(DI), m_CurrentColours(DI->GetStringParameter(SP_COLOUR_ID)) {
  m_hwnd = 0;
  //DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_COLOUR, Parent, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
}

void CColourBox::PopulateList() {
  HWND ListBox = GetDlgItem(m_hwnd, IDC_COLOURS);
  SendMessage(ListBox, LB_RESETCONTENT, 0, 0);

  m_pDasherInterface->GetPermittedValues(SP_COLOUR_ID, ColourList);

  // Add each string to list box and index each one
  bool SelectionSet = false;
  for(unsigned int i = 0; i < ColourList.size(); i++) {
    Tstring Item;
    WinUTF8::UTF8string_to_wstring(ColourList[i], Item);
    LRESULT Index = SendMessage(ListBox, LB_ADDSTRING, 0, (LPARAM) Item.c_str());
    SendMessage(ListBox, LB_SETITEMDATA, Index, (LPARAM) i);
    if(ColourList[i] == m_CurrentColours) {
      SendMessage(ListBox, LB_SETCURSEL, Index, 0);
      SelectionSet = true;
    }
  }
  if(SelectionSet == false) {
    SendMessage(ListBox, LB_SETCURSEL, 0, 0);
    LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, 0, 0);
    m_CurrentColours = ColourList[CurrentIndex];
  }
  // Tell list box that we have set an item for it (so that delete and edit can be grayed if required)
  SendMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_COLOURS, LBN_SELCHANGE), 0);
}

bool CColourBox::Validate() {
  // Return false if something is wrong to prevent user from clicking to a different page. Please also pop up a dialogue informing the user at this point.
  return TRUE;
}

bool CColourBox::Apply() {
  if(m_CurrentColours != std::string("")) {
    if(m_CurrentColours != m_pDasherInterface->GetStringParameter(SP_COLOUR_ID))
      m_pDasherInterface->SetStringParameter(SP_COLOUR_ID, m_CurrentColours);
  }

  // Return false (and notify the user) if something is wrong.
  return TRUE;
}

void CColourBox::InitCustomBox() {
  // Sort out "spin" or "up down" buttons so that I get messages from them.
  SendMessage(GetDlgItem(CustomBox, IDC_MOVE_GROUP), UDM_SETBUDDY, (WPARAM) GetDlgItem(CustomBox, IDC_GROUP_BUDDY), 0);
  SendMessage(GetDlgItem(CustomBox, IDC_MOVE_CHAR), UDM_SETBUDDY, (WPARAM) GetDlgItem(CustomBox, IDC_CHAR_BUDDY), 0);
}

LRESULT CColourBox::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  NMHDR *pNMHDR;

  switch (message) {
  case WM_INITDIALOG:
    if(!m_hwnd) {               // If this is the initial dialog for the first time
      m_hwnd = Window;
      PopulateList();
    }
    return TRUE;
    break;
  case WM_NOTIFY:
    pNMHDR = (NMHDR*)lParam;
    switch (pNMHDR->code) {
    case PSN_KILLACTIVE: // About to lose focus
      SetWindowLong( pNMHDR->hwndFrom, DWL_MSGRESULT, Validate());
      return TRUE;
      break;
    case PSN_APPLY: // User clicked OK/Apply - apply the changes
      if(Apply())
        SetWindowLong( pNMHDR->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLong( pNMHDR->hwndFrom, DWL_MSGRESULT, PSNRET_INVALID);
      return TRUE;
      break;
    }
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case (IDC_DISPLAY):
      if(HIWORD(wParam) == EN_CHANGE) {
        HWND Control = GetDlgItem(Window, IDC_DISPLAY);
        LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1;        // +1 to allow for terminator
        TCHAR *Buffer = new TCHAR[BufferLength];
        SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM) Buffer);
        string ItemName;
        SendMessage(GetDlgItem(Window, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) Buffer);
        delete[]Buffer;
      }
      break;
    case (IDC_COLOURS):
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        HWND ListBox = GetDlgItem(m_hwnd, IDC_COLOURS);
        LRESULT CurrentItem = SendMessage(ListBox, LB_GETCURSEL, 0, 0);
        LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, CurrentItem, 0);
        m_CurrentColours = ColourList[CurrentIndex];
      }
      return TRUE;
      break;
    case (IDOK):
      if(m_CurrentColours != std::string("")) {
        m_pDasherInterface->SetStringParameter(SP_COLOUR_ID, m_CurrentColours);
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
