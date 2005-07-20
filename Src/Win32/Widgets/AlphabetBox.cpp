// AlphabetBox.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "AlphabetBox.h"
#include "../resource.h"

using namespace Dasher;
using namespace std;

CAlphabetBox::CAlphabetBox(HWND Parent, CDasherInterface *DI)
:m_pDasherInterface(DI), m_CurrentAlphabet(DI->GetStringParameter(SP_ALPHABET_ID)), Editing(false), Cloning(false), EditChar(false), CustomBox(0), CurrentGroup(0), CurrentChar(0) {
  m_hwnd = 0;
  DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_ALPHABET, Parent, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
}

void CAlphabetBox::PopulateList() {
  HWND ListBox = GetDlgItem(m_hwnd, IDC_ALPHABETS);
  SendMessage(ListBox, LB_RESETCONTENT, 0, 0);

  m_pDasherInterface->GetAlphabets(&AlphabetList);

  // Add each string to list box and index each one
  bool SelectionSet = false;
  for(unsigned int i = 0; i < AlphabetList.size(); i++) {
    Tstring Item;
    WinUTF8::UTF8string_to_wstring(AlphabetList[i], Item);
    LRESULT Index = SendMessage(ListBox, LB_ADDSTRING, 0, (LPARAM) Item.c_str());
    SendMessage(ListBox, LB_SETITEMDATA, Index, (LPARAM) i);
    if(AlphabetList[i] == m_CurrentAlphabet) {
      SendMessage(ListBox, LB_SETCURSEL, Index, 0);
      SelectionSet = true;
    }
  }
  if(SelectionSet == false) {
    SendMessage(ListBox, LB_SETCURSEL, 0, 0);
    LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, 0, 0);
    m_CurrentAlphabet = AlphabetList[CurrentIndex];
  }
  // Tell list box that we have set an item for it (so that delete and edit can be grayed if required)
  SendMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_ALPHABETS, LBN_SELCHANGE), 0);
}

void CAlphabetBox::InitCustomBox() {
  Editing = false;

  // Sort out "spin" or "up down" buttons so that I get messages from them.
  SendMessage(GetDlgItem(CustomBox, IDC_MOVE_GROUP), UDM_SETBUDDY, (WPARAM) GetDlgItem(CustomBox, IDC_GROUP_BUDDY), 0);
  SendMessage(GetDlgItem(CustomBox, IDC_MOVE_CHAR), UDM_SETBUDDY, (WPARAM) GetDlgItem(CustomBox, IDC_CHAR_BUDDY), 0);

  Tstring Data;
  WinUTF8::UTF8string_to_wstring(CurrentInfo.AlphID, Data);
  SendMessage(GetDlgItem(CustomBox, IDC_DESCRIPTION), WM_SETTEXT, 0, (LPARAM) Data.c_str());
  WinUTF8::UTF8string_to_wstring(CurrentInfo.TrainingFile, Data);
  SendMessage(GetDlgItem(CustomBox, IDC_TRAIN), WM_SETTEXT, 0, (LPARAM) Data.c_str());

  // Load encodings list box from resource strings
  vector < pair < int, int > >Encodings;
  Encodings.push_back(pair < int, int >(IDS_TYPE_None, Opts::MyNone));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Arabic, Opts::Arabic));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Baltic, Opts::Baltic));
  Encodings.push_back(pair < int, int >(IDS_TYPE_CentralEurope, Opts::CentralEurope));
  Encodings.push_back(pair < int, int >(IDS_TYPE_ChineseSimplified, Opts::ChineseSimplified));
  Encodings.push_back(pair < int, int >(IDS_TYPE_ChineseTraditional, Opts::ChineseTraditional));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Cyrillic, Opts::Cyrillic));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Greek, Opts::Greek));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Hebrew, Opts::Hebrew));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Japanese, Opts::Japanese));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Korean, Opts::Korean));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Thai, Opts::Thai));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Turkish, Opts::Turkish));
  Encodings.push_back(pair < int, int >(IDS_TYPE_VietNam, Opts::VietNam));
  Encodings.push_back(pair < int, int >(IDS_TYPE_Western, Opts::Western));
  Tstring ResourceString;
  const HWND Encoding = GetDlgItem(CustomBox, IDC_ENCODING);
  for(unsigned int i = 0; i < Encodings.size(); i++) {
    WinLocalisation::GetResourceString(Encodings[i].first, &ResourceString);
    LRESULT Index = SendMessage(Encoding, CB_ADDSTRING, 0, (LPARAM) ResourceString.c_str());
    SendMessage(Encoding, CB_SETITEMDATA, Index, (LPARAM) Encodings[i].second);
    if((CurrentInfo.Type == (Opts::AlphabetTypes) Encodings[i].second) || (i == 0))
      SendMessage(Encoding, CB_SETCURSEL, Index, 0);
  }

  // Orientations list box
  vector < pair < int, int > >Orientations;
  Orientations.push_back(pair < int, int >(IDS_ORIENT_LR, Opts::LeftToRight));
  Orientations.push_back(pair < int, int >(IDS_ORIENT_RL, Opts::RightToLeft));
  Orientations.push_back(pair < int, int >(IDS_ORIENT_TB, Opts::TopToBottom));
  Orientations.push_back(pair < int, int >(IDS_ORIENT_BT, Opts::BottomToTop));
  const HWND Orientation = GetDlgItem(CustomBox, IDC_RO);
  for(unsigned int j = 0; j < Orientations.size(); j++) {
    WinLocalisation::GetResourceString(Orientations[j].first, &ResourceString);
    LRESULT Index = SendMessage(Orientation, CB_ADDSTRING, 0, (LPARAM) ResourceString.c_str());
    SendMessage(Orientation, CB_SETITEMDATA, Index, (LPARAM) Orientations[j].second);
    if((CurrentInfo.Orientation == (Opts::ScreenOrientations) Orientations[j].second) || (j == 0))
      SendMessage(Orientation, CB_SETCURSEL, Index, 0);
  }

  // Show Groups
  CurrentGroup = 0;
  CurrentChar = 0;
  ShowGroups();

  // Show characters for first group
  SendMessage(GetDlgItem(CustomBox, IDC_CHARS), LB_SETCOLUMNWIDTH, 34, 0);

  // Set check mark for the "space character"
  if(CurrentInfo.SpaceCharacter.Display != std::string(""))
    SendMessage(GetDlgItem(CustomBox, IDC_SPACE), BM_SETCHECK, BST_CHECKED, 0);
  // Set check mark for the "paragraph character"
  if(CurrentInfo.ParagraphCharacter.Display != std::string(""))
    SendMessage(GetDlgItem(CustomBox, IDC_PARAGRAPH), BM_SETCHECK, BST_CHECKED, 0);
  // Set check mark for the "control character"
  if(CurrentInfo.ControlCharacter.Display != std::string(""))
    SendMessage(GetDlgItem(CustomBox, IDC_CONTROLCHAR), BM_SETCHECK, BST_CHECKED, 0);
}

std::string CAlphabetBox::GetControlText(HWND Dialog, int ControlID) {
  HWND Control = GetDlgItem(Dialog, ControlID);
  LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1;      // +1 to allow for terminator
  TCHAR *Buffer = new TCHAR[BufferLength];
  SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM) Buffer);
  string ItemName;
  WinUTF8::wstring_to_UTF8string(Buffer, ItemName);
  delete[]Buffer;
  return ItemName;
}

void CAlphabetBox::NewGroup(std::string NewGroup) {
  HWND ListBox = GetDlgItem(CustomBox, IDC_GROUPS);
  Tstring Group;
  WinUTF8::UTF8string_to_wstring(NewGroup, Group);
  SendMessage(ListBox, LB_ADDSTRING, 0, (LPARAM) Group.c_str());

  CAlphIO::AlphInfo::group TmpGroup;
  TmpGroup.Description = NewGroup;
  CurrentInfo.Groups.push_back(TmpGroup);
}

void CAlphabetBox::ShowGroups() {
  Tstring Data;
  const HWND Groups = GetDlgItem(CustomBox, IDC_GROUPS);
  SendMessage(Groups, LB_RESETCONTENT, 0, 0);

  if(CurrentInfo.Groups.size() == 0) {
    CurrentGroup = 0;
    ShowGroupChars();
    return;
  }

  for(unsigned int k = 0; k < CurrentInfo.Groups.size(); k++) {
    WinUTF8::UTF8string_to_wstring(CurrentInfo.Groups[k].Description, Data);
    SendMessage(Groups, LB_ADDSTRING, 0, (LPARAM) Data.c_str());
  }

  if(CurrentGroup >= CurrentInfo.Groups.size())
    CurrentGroup = CurrentInfo.Groups.size() - 1;

  SendMessage(Groups, LB_SETCURSEL, CurrentGroup, 0);
  ShowGroupChars();
}

void CAlphabetBox::ShowGroupChars() {
  const HWND Chars = GetDlgItem(CustomBox, IDC_CHARS);
  SendMessage(Chars, LB_RESETCONTENT, 0, 0);

  if(CurrentInfo.Groups.size() == 0) {  // If no groups are defined, don't try
    CurrentChar = 0;
    return;
  }

  Tstring Data;
  for(unsigned int j = 0; j < CurrentInfo.Groups[CurrentGroup].Characters.size(); j++) {
    WinUTF8::UTF8string_to_wstring(CurrentInfo.Groups[CurrentGroup].Characters[j].Display, Data);
    SendMessage(Chars, LB_ADDSTRING, 0, (LPARAM) Data.c_str());
  }
  SendMessage(Chars, LB_SETCURSEL, CurrentChar, 0);
}

void CAlphabetBox::CustomCharacter(std::string Display, std::string Text, int Colour) {
  vector < CAlphIO::AlphInfo::character > &Chars = CurrentInfo.Groups[CurrentGroup].Characters;

  if(Chars.size() == 0)         // Cannot edit a character if none exist
    EditChar = false;

  if(!EditChar) {               // if need to add a character
    CAlphIO::AlphInfo::character NewChar;
    if(CurrentChar == Chars.size())
      Chars.push_back(NewChar);
    else {
      Chars.insert(Chars.begin() + CurrentChar + 1, NewChar);
      ++CurrentChar;
    }
  }
  else
    EditChar = false;           // just resetting flag

  Chars[CurrentChar].Text = Text;
  Chars[CurrentChar].Display = Display;
  Chars[CurrentChar].Colour = Colour;

  ShowGroupChars();             // lazy, don't really need to update whole list
}

bool CAlphabetBox::UpdateInfo() {
  CurrentInfo.AlphID = GetControlText(CustomBox, IDC_DESCRIPTION);

  // Check description is unique and not blank
  bool Unique = true;
  for(unsigned int i = 0; i < AlphabetList.size(); i++) {
    if((AlphabetList[i] == CurrentInfo.AlphID) && (AlphabetList[i] != m_CurrentAlphabet)) {
      Unique = false;
      break;
    }
  }
  if((CurrentInfo.AlphID == "") || (Unique == false)) {
    Tstring ErrMessage, AppTitle;
    WinLocalisation::GetResourceString(IDS_ERR_ALPH_DESC, &ErrMessage);
    WinLocalisation::GetResourceString(IDS_APP_TITLE, &AppTitle);
    MessageBox(CustomBox, ErrMessage.c_str(), AppTitle.c_str(), MB_ICONERROR);
    HWND Desc = GetDlgItem(CustomBox, IDC_DESCRIPTION);
    SetFocus(Desc);
    SendMessage(Desc, EM_SETSEL, 0, -1);
    return false;
  }

  HWND Encoding = GetDlgItem(CustomBox, IDC_ENCODING);
  LRESULT CurrentItem = SendMessage(Encoding, CB_GETCURSEL, 0, 0);
  CurrentInfo.Type = (Opts::AlphabetTypes) SendMessage(Encoding, CB_GETITEMDATA, CurrentItem, 0);

  CurrentInfo.Mutable = true;

  HWND Orientation = GetDlgItem(CustomBox, IDC_RO);
  CurrentItem = SendMessage(Orientation, CB_GETCURSEL, 0, 0);
  CurrentInfo.Orientation = (Opts::ScreenOrientations) SendMessage(Orientation, CB_GETITEMDATA, CurrentItem, 0);

  if(SendMessage(GetDlgItem(CustomBox, IDC_SPACE), BM_GETCHECK, 0, 0) == BST_CHECKED) {
    CurrentInfo.SpaceCharacter.Text = " ";
    CurrentInfo.SpaceCharacter.Display = "_";
    CurrentInfo.SpaceCharacter.Colour = 9;
  }
  else {
    CurrentInfo.SpaceCharacter.Text = "";
    CurrentInfo.SpaceCharacter.Display = "";
    CurrentInfo.SpaceCharacter.Colour = -1;
  }

  if(SendMessage(GetDlgItem(CustomBox, IDC_PARAGRAPH), BM_GETCHECK, 0, 0) == BST_CHECKED) {
    CurrentInfo.ParagraphCharacter.Text = "\r\n";
    CurrentInfo.ParagraphCharacter.Display = "¶";
    CurrentInfo.ParagraphCharacter.Colour = 9;
  }
  else {
    CurrentInfo.ParagraphCharacter.Text = "";
    CurrentInfo.ParagraphCharacter.Display = "";
    CurrentInfo.ParagraphCharacter.Colour = -1;
  }

  if(SendMessage(GetDlgItem(CustomBox, IDC_CONTROLCHAR), BM_GETCHECK, 0, 0) == BST_CHECKED) {
    CurrentInfo.ControlCharacter.Text = "Control";
    CurrentInfo.ControlCharacter.Display = "Control";
    CurrentInfo.ControlCharacter.Colour = 8;
  }
  else {
    CurrentInfo.ControlCharacter.Text = "";
    CurrentInfo.ControlCharacter.Display = "";
    CurrentInfo.ControlCharacter.Colour = -1;
  }

  CurrentInfo.TrainingFile = GetControlText(CustomBox, IDC_TRAIN);

  return true;
}

LRESULT CAlphabetBox::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_INITDIALOG:
    if(!m_hwnd) {               // If this is the initial dialog for the first time
      m_hwnd = Window;
      PopulateList();
    }
    else if(Editing) {
      CustomBox = Window;
      InitCustomBox();
    }
    else if(EditChar) {
      char colour[100];
      sprintf(colour, "%d", CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar].Colour);
      SendMessage(GetDlgItem(Window, IDC_DISPLAY), WM_SETTEXT, 0, (LPARAM) (LPCSTR) CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar].Display.c_str());
      SendMessage(GetDlgItem(Window, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) (LPCSTR) CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar].Text.c_str());
      SendMessage(GetDlgItem(Window, IDC_COLOUR), WM_SETTEXT, 0, (LPARAM) (LPCSTR) colour);
    }
    return TRUE;
    break;
  case WM_NOTIFY:{
      // Moving stuff in here
      if(CurrentInfo.Groups.size() < 1) {
        return TRUE;
        break;
      }
      NMUPDOWN *Data = (NMUPDOWN *) lParam;
      if(Data->hdr.idFrom == IDC_MOVE_GROUP) {
        CAlphIO::AlphInfo::group Tmp = CurrentInfo.Groups[CurrentGroup];
        if(Data->iDelta > 0) {
          if(CurrentGroup + 1 < CurrentInfo.Groups.size()) {
            CurrentInfo.Groups[CurrentGroup] = CurrentInfo.Groups[CurrentGroup + 1];
            CurrentInfo.Groups[CurrentGroup + 1] = Tmp;
            ++CurrentGroup;
          }
        }
        else {
          if(CurrentGroup > 0) {
            CurrentInfo.Groups[CurrentGroup] = CurrentInfo.Groups[CurrentGroup - 1];
            CurrentInfo.Groups[CurrentGroup - 1] = Tmp;
            --CurrentGroup;
          }
        }
        ShowGroups();
      }
      if(Data->hdr.idFrom == IDC_MOVE_CHAR) {
        if(CurrentInfo.Groups[CurrentGroup].Characters.size() < 1) {
          return TRUE;
          break;
        }
        CAlphIO::AlphInfo::character Tmp = CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar];
        if(Data->iDelta > 0) {
          if(CurrentChar + 1 < CurrentInfo.Groups[CurrentGroup].Characters.size()) {
            CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar] = CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar + 1];
            CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar + 1] = Tmp;
            ++CurrentChar;
          }
        }
        else {
          if(CurrentChar > 0) {
            CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar] = CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar - 1];
            CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar - 1] = Tmp;
            --CurrentChar;
          }
        }
        ShowGroupChars();
      }
      return TRUE;
      break;
    }
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
    case (IDC_ALPHABETS):
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        HWND ListBox = GetDlgItem(m_hwnd, IDC_ALPHABETS);
        LRESULT CurrentItem = SendMessage(ListBox, LB_GETCURSEL, 0, 0);
        LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, CurrentItem, 0);
        m_CurrentAlphabet = AlphabetList[CurrentIndex];
        CurrentInfo = m_pDasherInterface->GetInfo(m_CurrentAlphabet);
        if(CurrentInfo.Mutable) {
          EnableWindow(GetDlgItem(m_hwnd, IDC_DEL_ALPH), TRUE);
          EnableWindow(GetDlgItem(m_hwnd, IDC_EDIT), TRUE);
        }
        else {
          EnableWindow(GetDlgItem(m_hwnd, IDC_DEL_ALPH), FALSE);
          EnableWindow(GetDlgItem(m_hwnd, IDC_EDIT), FALSE);
        }
      }
      return TRUE;
      break;
    case (IDC_GROUPS):
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        HWND GroupList = GetDlgItem(Window, IDC_GROUPS);
        CurrentGroup = SendMessage(GroupList, LB_GETCURSEL, 0, 0);
        CurrentChar = 0;
        ShowGroupChars();
      }
      return TRUE;
      break;
    case (IDC_CHARS):
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        HWND CharList = GetDlgItem(Window, IDC_CHARS);
        CurrentChar = SendMessage(CharList, LB_GETCURSEL, 0, 0);
      }
      return TRUE;
      break;
    case (IDC_EDIT_CHAR):
      if(CurrentInfo.Groups.size() < 1) // if no groups, can't edit character
        break;
      EditChar = true;
      DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_EDITLETTER, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
      break;
    case (IDC_ADD_CHAR):
      if(CurrentInfo.Groups.size() < 1) // if no groups, can't add character
        break;
      DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_ADDLETTERS, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
      break;
    case (IDOK_ADDCHAR):{
        string Display = GetControlText(Window, IDC_DISPLAY);
        string Text = GetControlText(Window, IDC_TEXT);
        int Colour = atoi(GetControlText(Window, IDC_COLOUR).c_str());
        CustomCharacter(Display, Text, Colour);
        TCHAR Terminator = '\0';
        SendMessage(GetDlgItem(Window, IDC_DISPLAY), WM_SETTEXT, 0, (LPARAM) & Terminator);
        SetFocus(GetDlgItem(Window, IDC_DISPLAY));
        break;
      }
    case (IDOK_CHAR):{
        string Display = GetControlText(Window, IDC_DISPLAY);
        string Text = GetControlText(Window, IDC_TEXT);
        int Colour = atoi(GetControlText(Window, IDC_COLOUR).c_str());
        CustomCharacter(Display, Text, Colour);
        EndDialog(Window, LOWORD(wParam));
        EditChar = false;
        break;
      }
    case (IDC_ADD_GROUP):
      DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_NEWGROUP, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
      break;
    case (IDOK_NEWGROUP):{
        NewGroup(GetControlText(Window, IDC_GROUP_TEXT));
        EndDialog(Window, LOWORD(wParam));
        break;
      }
    case (IDC_DELGROUP):
      if(CurrentInfo.Groups.size() > CurrentGroup)
        CurrentInfo.Groups.erase(CurrentInfo.Groups.begin() + CurrentGroup);
      ShowGroups();
      break;
    case (IDC_DEL_CHAR):
      if(CurrentInfo.Groups.size() > CurrentGroup) {
        vector < CAlphIO::AlphInfo::character > &Tmp = CurrentInfo.Groups[CurrentGroup].Characters;
        if(Tmp.size() > CurrentChar) {
          Tmp.erase(Tmp.begin() + CurrentChar);
          CurrentChar = min(Tmp.size() - 1, CurrentChar);
          ShowGroupChars();
        }
      }
      break;
    case (IDOK_CUSTOMIZE):
      if(UpdateInfo()) {
        m_pDasherInterface->SetInfo(CurrentInfo);
        EndDialog(Window, LOWORD(wParam));
        PopulateList();
      }
      break;
    case (IDC_DEL_ALPH):{
        HWND ListBox = GetDlgItem(m_hwnd, IDC_ALPHABETS);
        LRESULT CurrentItem = SendMessage(ListBox, LB_GETCURSEL, 0, 0);
        LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, CurrentItem, 0);
        if(CurrentIndex >= 0 && (unsigned int)CurrentIndex < AlphabetList.size()) {
          m_pDasherInterface->DeleteAlphabet(AlphabetList[CurrentIndex]);
          PopulateList();
        }
        break;
      }
    case (IDC_CLONE):
      Cloning = true;
      // deliberate fall through
    case (IDC_ADD_ALPH):
      DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_NEWALPHABET, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
      break;
    case (IDOK_NEWALPH):{
        m_CurrentAlphabet = ""; // Prevent over-writing another alphabet in rare circumstances.
        string NewAlph = GetControlText(Window, IDC_NEW_ALPH_STRING);
        EndDialog(Window, LOWORD(wParam));
        if(Cloning) {
          Cloning = false;
        }
        else {
          CAlphIO::AlphInfo Tmp;
          CurrentInfo = Tmp;
        }
        CurrentInfo.AlphID = NewAlph;
      }                         // deliberate fall through
    case (IDC_EDIT):
      Editing = true;
      DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_CUSTOMALPHABET, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
      break;
    case (IDOK):
      if(m_CurrentAlphabet != std::string("")) {
        m_pDasherInterface->SetStringParameter(SP_ALPHABET_ID, m_CurrentAlphabet);
      }
      // deliberate fall through
    case (IDCANCEL):
      {
        EndDialog(Window, LOWORD(wParam));
        return TRUE;
      }
    case ID_CUSTOM_CANCEL:
      PopulateList();           // Need to reget settings for current selection. Cheaty way to do it.
      EndDialog(Window, LOWORD(wParam));
      return TRUE;
      break;
    }
  }
  return FALSE;
}
