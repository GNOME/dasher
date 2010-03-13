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

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

struct menuentry {
  int paramNum; // enum value in Parameters.h for setting store
  int idcNum;   // #define value in resource.h for dasher.rc
  bool bInvert;
};

// List of menu items that will be displayed in the General Preferences
static menuentry menutable[] = {
  {BP_PALETTE_CHANGE, IDC_COLOURSCHEME, true}
};

CAlphabetBox::CAlphabetBox(HWND Parent, CDasherInterfaceBase *DI, CAppSettings *pAppSettings)
: CPrefsPageBase(Parent, DI, pAppSettings), m_pDasherInterface(DI), m_CurrentAlphabet(DI->GetStringParameter(SP_ALPHABET_ID)),  Editing(false), Cloning(false), EditChar(false), CustomBox(0), CurrentGroup(0), CurrentChar(0) {
  m_hwnd = 0;
  m_hPropertySheet = 0;
}

void CAlphabetBox::PopulateList() {
#ifndef JAPANESE
    EnableWindow(GetDlgItem(m_hwnd, IDC_LM_JAPANESE), FALSE);
#endif

    slider = GetDlgItem(m_hwnd, IDC_UNIFORMSLIDER);
    SendMessage(slider, TBM_SETPAGESIZE, 0L, 20); // PgUp and PgDown change bitrate by reasonable amount
    SendMessage(slider, TBM_SETTICFREQ, 10, 0L);
    SendMessage(slider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(0, 1000));
  
    uniformbox = GetDlgItem(m_hwnd, IDC_UNIFORMVAL);  

    if(m_pAppSettings->GetBoolParameter(BP_LM_ADAPTIVE)) {
      SendMessage(GetDlgItem(m_hwnd, IDC_ADAPTIVE), BM_SETCHECK, BST_CHECKED, 0);
    }
    else  {
      SendMessage(GetDlgItem(m_hwnd, IDC_ADAPTIVE), BM_SETCHECK, BST_UNCHECKED, 0);
    }

    SendMessage(slider, TBM_SETPOS, TRUE, (LPARAM) m_pAppSettings->GetLongParameter(LP_UNIFORM));

    _sntprintf(m_tcBuffer, 100, TEXT("%0.1f"), m_pAppSettings->GetLongParameter(LP_UNIFORM) / 10.0);
    SendMessage(uniformbox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);

  switch(m_pAppSettings->GetLongParameter(LP_LANGUAGE_MODEL_ID)) {
    case 0:
      SendMessage(GetDlgItem(m_hwnd, IDC_LM_PPM), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case 2:
      SendMessage(GetDlgItem(m_hwnd, IDC_LM_WORD), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case 3:
      SendMessage(GetDlgItem(m_hwnd, IDC_LM_MIXTURE), BM_SETCHECK, BST_CHECKED, 0);
      break;
#ifdef JAPANESE
    case 4:
      SendMessage(GetDlgItem(m_hwnd, IDC_LM_JAPANESE), BM_SETCHECK, BST_CHECKED, 0);
      break;
#endif
   }
 // Populate the orientation selection:

  int iOrientation = m_pAppSettings->GetLongParameter(LP_ORIENTATION);

  if(iOrientation == Dasher::Opts::AlphabetDefault) {
    SendMessage(GetDlgItem(m_hwnd, IDC_OR_DEFAULT), BM_SETCHECK, BST_CHECKED, 0);

    EnableWindow(GetDlgItem(m_hwnd, IDC_OR_LR), FALSE);
    EnableWindow(GetDlgItem(m_hwnd, IDC_OR_RL), FALSE);
    EnableWindow(GetDlgItem(m_hwnd, IDC_OR_TB), FALSE);
    EnableWindow(GetDlgItem(m_hwnd, IDC_OR_BT), FALSE);
  }
  else {
    SendMessage(GetDlgItem(m_hwnd, IDC_OR_CUSTOM), BM_SETCHECK, BST_CHECKED, 0);

    switch(iOrientation) {
    case Dasher::Opts::LeftToRight:
      SendMessage(GetDlgItem(m_hwnd, IDC_OR_LR), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case Dasher::Opts::RightToLeft:
      SendMessage(GetDlgItem(m_hwnd, IDC_OR_RL), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case Dasher::Opts::TopToBottom:
      SendMessage(GetDlgItem(m_hwnd, IDC_OR_TB), BM_SETCHECK, BST_CHECKED, 0);
      break;
    case Dasher::Opts::BottomToTop:
      SendMessage(GetDlgItem(m_hwnd, IDC_OR_BT), BM_SETCHECK, BST_CHECKED, 0);
      break;
    }
  }

  HWND ListBox = GetDlgItem(m_hwnd, IDC_ALPHABETS);
  SendMessage(ListBox, LB_RESETCONTENT, 0, 0);

  m_CurrentAlphabet = m_pDasherInterface->GetStringParameter(SP_ALPHABET_ID);

  m_pDasherInterface->GetPermittedValues(SP_ALPHABET_ID, AlphabetList);

  int iDefaultIndex(-1);
  int iFallbackIndex(-1);

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

  iDefaultIndex =  SendMessage(ListBox, LB_FINDSTRING, -1, (LPARAM)L"English alphabet - limited punctuation");
  if(iDefaultIndex == LB_ERR) {
    iDefaultIndex =  SendMessage(ListBox, LB_FINDSTRING, -1, (LPARAM)L"Default");
    if(iDefaultIndex == LB_ERR)
      iDefaultIndex = 0;
  }

  SendMessage(ListBox, LB_SETCURSEL, iDefaultIndex, 0);
  LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, 0, 0);
  m_CurrentAlphabet = AlphabetList[CurrentIndex];
  }
  // Tell list box that we have set an item for it (so that delete and edit can be grayed if required)
  SendMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_ALPHABETS, LBN_SELCHANGE), 0);



// all the button checkboxes
  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    if(m_pDasherInterface->GetBoolParameter(menutable[ii].paramNum) != menutable[ii].bInvert) 
	    SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
    else  
	    SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_UNCHECKED, 0);
  }

}

//void CAlphabetBox::InitCustomBox() {
//  Editing = false;
//
//  // Sort out "spin" or "up down" buttons so that I get messages from them.
//  SendMessage(GetDlgItem(CustomBox, IDC_MOVE_GROUP), UDM_SETBUDDY, (WPARAM) GetDlgItem(CustomBox, IDC_GROUP_BUDDY), 0);
//  SendMessage(GetDlgItem(CustomBox, IDC_MOVE_CHAR), UDM_SETBUDDY, (WPARAM) GetDlgItem(CustomBox, IDC_CHAR_BUDDY), 0);
//
//  Tstring Data;
//  WinUTF8::UTF8string_to_wstring(CurrentInfo.AlphID, Data);
//  SendMessage(GetDlgItem(CustomBox, IDC_DESCRIPTION), WM_SETTEXT, 0, (LPARAM) Data.c_str());
//  WinUTF8::UTF8string_to_wstring(CurrentInfo.TrainingFile, Data);
//  SendMessage(GetDlgItem(CustomBox, IDC_TRAIN), WM_SETTEXT, 0, (LPARAM) Data.c_str());
//
//  // Load encodings list box from resource strings
//  vector < pair < int, int > >Encodings;
//  Encodings.push_back(pair < int, int >(IDS_TYPE_None, Opts::MyNone));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Arabic, Opts::Arabic));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Baltic, Opts::Baltic));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_CentralEurope, Opts::CentralEurope));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_ChineseSimplified, Opts::ChineseSimplified));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_ChineseTraditional, Opts::ChineseTraditional));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Cyrillic, Opts::Cyrillic));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Greek, Opts::Greek));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Hebrew, Opts::Hebrew));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Japanese, Opts::Japanese));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Korean, Opts::Korean));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Thai, Opts::Thai));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Turkish, Opts::Turkish));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_VietNam, Opts::VietNam));
//  Encodings.push_back(pair < int, int >(IDS_TYPE_Western, Opts::Western));
//  Tstring ResourceString;
//  const HWND Encoding = GetDlgItem(CustomBox, IDC_ENCODING);
//  for(unsigned int i = 0; i < Encodings.size(); i++) {
//    WinLocalisation::GetResourceString(Encodings[i].first, &ResourceString);
//    LRESULT Index = SendMessage(Encoding, CB_ADDSTRING, 0, (LPARAM) ResourceString.c_str());
//    SendMessage(Encoding, CB_SETITEMDATA, Index, (LPARAM) Encodings[i].second);
//    if((CurrentInfo.Type == (Opts::AlphabetTypes) Encodings[i].second) || (i == 0))
//      SendMessage(Encoding, CB_SETCURSEL, Index, 0);
//  }
//
//  // Orientations list box
//  vector < pair < int, int > >Orientations;
//  Orientations.push_back(pair < int, int >(IDS_ORIENT_LR, Opts::LeftToRight));
//  Orientations.push_back(pair < int, int >(IDS_ORIENT_RL, Opts::RightToLeft));
//  Orientations.push_back(pair < int, int >(IDS_ORIENT_TB, Opts::TopToBottom));
//  Orientations.push_back(pair < int, int >(IDS_ORIENT_BT, Opts::BottomToTop));
//  const HWND Orientation = GetDlgItem(CustomBox, IDC_RO);
//  for(unsigned int j = 0; j < Orientations.size(); j++) {
//    WinLocalisation::GetResourceString(Orientations[j].first, &ResourceString);
//    LRESULT Index = SendMessage(Orientation, CB_ADDSTRING, 0, (LPARAM) ResourceString.c_str());
//    SendMessage(Orientation, CB_SETITEMDATA, Index, (LPARAM) Orientations[j].second);
//    if((CurrentInfo.Orientation == (Opts::ScreenOrientations) Orientations[j].second) || (j == 0))
//      SendMessage(Orientation, CB_SETCURSEL, Index, 0);
//  }
//
//  // Show Groups
//  CurrentGroup = 0;
//  CurrentChar = 0;
//  ShowGroups();
//
//  // Show characters for first group
//  SendMessage(GetDlgItem(CustomBox, IDC_CHARS), LB_SETCOLUMNWIDTH, 34, 0);
//
//  // Set check mark for the "space character"
//  if(CurrentInfo.SpaceCharacter.Display != std::string(""))
//    SendMessage(GetDlgItem(CustomBox, IDC_SPACE), BM_SETCHECK, BST_CHECKED, 0);
//  // Set check mark for the "paragraph character"
//  if(CurrentInfo.ParagraphCharacter.Display != std::string(""))
//    SendMessage(GetDlgItem(CustomBox, IDC_PARAGRAPH), BM_SETCHECK, BST_CHECKED, 0);
//  // Set check mark for the "control character"
//  if(CurrentInfo.ControlCharacter.Display != std::string(""))
//    SendMessage(GetDlgItem(CustomBox, IDC_CONTROLCHAR), BM_SETCHECK, BST_CHECKED, 0);
//}
//
//std::string CAlphabetBox::GetControlText(HWND Dialog, int ControlID) {
//  HWND Control = GetDlgItem(Dialog, ControlID);
//  LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1;      // +1 to allow for terminator
//  TCHAR *Buffer = new TCHAR[BufferLength];
//  SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM) Buffer);
//  string ItemName;
//  WinUTF8::wstring_to_UTF8string(Buffer, ItemName);
//  delete[]Buffer;
//  return ItemName;
//}
//
//void CAlphabetBox::NewGroup(std::string NewGroup) {
//  HWND ListBox = GetDlgItem(CustomBox, IDC_GROUPS);
//  Tstring Group;
//  WinUTF8::UTF8string_to_wstring(NewGroup, Group);
//  SendMessage(ListBox, LB_ADDSTRING, 0, (LPARAM) Group.c_str());
//
//  CAlphIO::AlphInfo::group TmpGroup;
//  TmpGroup.Description = NewGroup;
//  CurrentInfo.Groups.push_back(TmpGroup);
//}
//
//void CAlphabetBox::ShowGroups() {
//  Tstring Data;
//  const HWND Groups = GetDlgItem(CustomBox, IDC_GROUPS);
//  SendMessage(Groups, LB_RESETCONTENT, 0, 0);
//
//  if(CurrentInfo.Groups.size() == 0) {
//    CurrentGroup = 0;
//    ShowGroupChars();
//    return;
//  }
//
//  for(unsigned int k = 0; k < CurrentInfo.Groups.size(); k++) {
//    WinUTF8::UTF8string_to_wstring(CurrentInfo.Groups[k].Description, Data);
//    SendMessage(Groups, LB_ADDSTRING, 0, (LPARAM) Data.c_str());
//  }
//
//  if(CurrentGroup >= CurrentInfo.Groups.size())
//    CurrentGroup = CurrentInfo.Groups.size() - 1;
//
//  SendMessage(Groups, LB_SETCURSEL, CurrentGroup, 0);
//  ShowGroupChars();
//}
//
//void CAlphabetBox::ShowGroupChars() {
//  const HWND Chars = GetDlgItem(CustomBox, IDC_CHARS);
//  SendMessage(Chars, LB_RESETCONTENT, 0, 0);
//
//  if(CurrentInfo.Groups.size() == 0) {  // If no groups are defined, don't try
//    CurrentChar = 0;
//    return;
//  }
//
//  Tstring Data;
//  for(unsigned int j = 0; j < CurrentInfo.Groups[CurrentGroup].Characters.size(); j++) {
//    WinUTF8::UTF8string_to_wstring(CurrentInfo.Groups[CurrentGroup].Characters[j].Display, Data);
//    SendMessage(Chars, LB_ADDSTRING, 0, (LPARAM) Data.c_str());
//  }
//  SendMessage(Chars, LB_SETCURSEL, CurrentChar, 0);
//}
//
//void CAlphabetBox::CustomCharacter(std::string Display, std::string Text, int Colour) {
//  vector < CAlphIO::AlphInfo::character > &Chars = CurrentInfo.Groups[CurrentGroup].Characters;
//
//  if(Chars.size() == 0)         // Cannot edit a character if none exist
//    EditChar = false;
//
//  if(!EditChar) {               // if need to add a character
//    CAlphIO::AlphInfo::character NewChar;
//    if(CurrentChar == Chars.size())
//      Chars.push_back(NewChar);
//    else {
//      Chars.insert(Chars.begin() + CurrentChar + 1, NewChar);
//      ++CurrentChar;
//    }
//  }
//  else
//    EditChar = false;           // just resetting flag
//
//  Chars[CurrentChar].Text = Text;
//  Chars[CurrentChar].Display = Display;
//  Chars[CurrentChar].Colour = Colour;
//
//  ShowGroupChars();             // lazy, don't really need to update whole list
//}
//
//bool CAlphabetBox::UpdateInfo() {
//  CurrentInfo.AlphID = GetControlText(CustomBox, IDC_DESCRIPTION);
//
//  // Check description is unique and not blank
//  bool Unique = true;
//  for(unsigned int i = 0; i < AlphabetList.size(); i++) {
//    if((AlphabetList[i] == CurrentInfo.AlphID) && (AlphabetList[i] != m_CurrentAlphabet)) {
//      Unique = false;
//      break;
//    }
//  }
//  if((CurrentInfo.AlphID == "") || (Unique == false)) {
//    Tstring ErrMessage, AppTitle;
//    WinLocalisation::GetResourceString(IDS_ERR_ALPH_DESC, &ErrMessage);
//    WinLocalisation::GetResourceString(IDS_APP_TITLE, &AppTitle);
//    MessageBox(CustomBox, ErrMessage.c_str(), AppTitle.c_str(), MB_ICONERROR);
//    HWND Desc = GetDlgItem(CustomBox, IDC_DESCRIPTION);
//    SetFocus(Desc);
//    SendMessage(Desc, EM_SETSEL, 0, -1);
//    return false;
//  }
//
//  HWND Encoding = GetDlgItem(CustomBox, IDC_ENCODING);
//  LRESULT CurrentItem = SendMessage(Encoding, CB_GETCURSEL, 0, 0);
//  CurrentInfo.Type = (Opts::AlphabetTypes) SendMessage(Encoding, CB_GETITEMDATA, CurrentItem, 0);
//
//  CurrentInfo.Mutable = true;
//
//  HWND Orientation = GetDlgItem(CustomBox, IDC_RO);
//  CurrentItem = SendMessage(Orientation, CB_GETCURSEL, 0, 0);
//  CurrentInfo.Orientation = (Opts::ScreenOrientations) SendMessage(Orientation, CB_GETITEMDATA, CurrentItem, 0);
//
//  if(SendMessage(GetDlgItem(CustomBox, IDC_SPACE), BM_GETCHECK, 0, 0) == BST_CHECKED) {
//    CurrentInfo.SpaceCharacter.Text = " ";
//    CurrentInfo.SpaceCharacter.Display = "_";
//    CurrentInfo.SpaceCharacter.Colour = 9;
//  }
//  else {
//    CurrentInfo.SpaceCharacter.Text = "";
//    CurrentInfo.SpaceCharacter.Display = "";
//    CurrentInfo.SpaceCharacter.Colour = -1;
//  }
//
//  if(SendMessage(GetDlgItem(CustomBox, IDC_PARAGRAPH), BM_GETCHECK, 0, 0) == BST_CHECKED) {
//    CurrentInfo.ParagraphCharacter.Text = "\r\n";
//    CurrentInfo.ParagraphCharacter.Display = "¶";
//    CurrentInfo.ParagraphCharacter.Colour = 9;
//  }
//  else {
//    CurrentInfo.ParagraphCharacter.Text = "";
//    CurrentInfo.ParagraphCharacter.Display = "";
//    CurrentInfo.ParagraphCharacter.Colour = -1;
//  }
//
//  if(SendMessage(GetDlgItem(CustomBox, IDC_CONTROLCHAR), BM_GETCHECK, 0, 0) == BST_CHECKED) {
//    CurrentInfo.ControlCharacter.Text = "Control";
//    CurrentInfo.ControlCharacter.Display = "Control";
//    CurrentInfo.ControlCharacter.Colour = 8;
//  }
//  else {
//    CurrentInfo.ControlCharacter.Text = "";
//    CurrentInfo.ControlCharacter.Display = "";
//    CurrentInfo.ControlCharacter.Colour = -1;
//  }
//
//  CurrentInfo.TrainingFile = GetControlText(CustomBox, IDC_TRAIN);
//
//  return true;
//}


bool CAlphabetBox::Apply() {

  if(SendMessage(GetDlgItem(m_hwnd, IDC_OR_DEFAULT), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::AlphabetDefault);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_OR_LR), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::LeftToRight);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_OR_RL), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::RightToLeft);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_OR_TB), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::TopToBottom);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_OR_BT), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_ORIENTATION, Dasher::Opts::BottomToTop);

  m_pAppSettings->SetBoolParameter( BP_LM_ADAPTIVE, SendMessage(GetDlgItem(m_hwnd, IDC_ADAPTIVE), BM_GETCHECK, 0, 0)!=0 );
  
  long NewUniform;
  NewUniform = SendMessage(slider, TBM_GETPOS, 0, 0); // 32-bit return value
  // XXX PRLW: Rather than fix the crash caused when LP_UNIFORM=0, put in
  // the same workaround found in Gtk2/Preferences.cpp
  if (NewUniform < 50) NewUniform = 50;
  m_pAppSettings->SetLongParameter( LP_UNIFORM, NewUniform);

  if(SendMessage(GetDlgItem(m_hwnd, IDC_LM_PPM), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_LANGUAGE_MODEL_ID, 0);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_LM_WORD), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_LANGUAGE_MODEL_ID, 2);
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_LM_MIXTURE), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_LANGUAGE_MODEL_ID, 3);
#ifdef JAPANESE
  else if(SendMessage(GetDlgItem(m_hwnd, IDC_LM_JAPANESE), BM_GETCHECK, 0, 0))
    m_pAppSettings->SetLongParameter(LP_LANGUAGE_MODEL_ID, 4);
#endif


  if(m_CurrentAlphabet != std::string("")) {
    if(m_CurrentAlphabet != m_pDasherInterface->GetStringParameter(SP_ALPHABET_ID))
      m_pDasherInterface->SetStringParameter(SP_ALPHABET_ID, m_CurrentAlphabet); 
  }

  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    m_pDasherInterface->SetBoolParameter(menutable[ii].paramNum, 
      (SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED) != menutable[ii].bInvert);
  }


  // Return false (and notify the user) if something is wrong.
  return TRUE;
}

LRESULT CAlphabetBox::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  //NMHDR *pNMHDR;
     double NewUniform;

  // most things we pass on to CPrefsPageBase, but we need to handle slider motion

  switch (message) {
  case WM_HSCROLL:
    if((LOWORD(wParam) == SB_THUMBPOSITION) | (LOWORD(wParam) == SB_THUMBTRACK)) {
      // Some messages give the new postion
      NewUniform = HIWORD(wParam);
    }
    else {
      // Otherwise we have to ask for it
      long Pos = SendMessage(slider, TBM_GETPOS, 0, 0);
      NewUniform = Pos;
    }
    {
      _sntprintf(m_tcBuffer, 100, TEXT("%0.1f"), NewUniform / 10);
      SendMessage(uniformbox, WM_SETTEXT, 0, (LPARAM) m_tcBuffer);
    }
    return TRUE;
    break;
  //case WM_INITDIALOG:
  //  if(!m_hwnd) {               // If this is the initial dialog for the first time
  //    m_hwnd = Window;
  //    PopulateList();
  //  }
  //  //else if(Editing) {
    //  CustomBox = Window;
    //  InitCustomBox();
    //}
    //else if(EditChar) {
    //  char colour[100];
    //  sprintf(colour, "%d", CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar].Colour);
    //  SendMessage(GetDlgItem(Window, IDC_DISPLAY), WM_SETTEXT, 0, (LPARAM) (LPCSTR) CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar].Display.c_str());
    //  SendMessage(GetDlgItem(Window, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) (LPCSTR) CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar].Text.c_str());
    //  SendMessage(GetDlgItem(Window, IDC_COLOUR), WM_SETTEXT, 0, (LPARAM) (LPCSTR) colour);
    //}
    //return TRUE;
    //break;
  //case WM_NOTIFY:{
  //  pNMHDR = (NMHDR*)lParam;
  //  if(m_hPropertySheet==0) {
  //    m_hPropertySheet = pNMHDR->hwndFrom;
  //  }
  //  switch (pNMHDR->code) {
  //  //case PSN_KILLACTIVE: // About to lose focus
  //  //  SetWindowLong( Window, DWL_MSGRESULT, !Validate());
  //  //  return TRUE;
  //  //  break;
  //  case PSN_APPLY: // User clicked OK/Apply - apply the changes
  //    if(Apply())
  //      SetWindowLong( Window, DWL_MSGRESULT, PSNRET_NOERROR);
  //    else
  //      SetWindowLong( Window, DWL_MSGRESULT, PSNRET_INVALID);
  //    return TRUE;
  //    break;
  //  //case UDN_DELTAPOS:
  //  //  // Moving stuff in here
  //  //  if(CurrentInfo.Groups.size() < 1) {
  //  //    return TRUE;
  //  //    break;
  //  //  }
  //  //  NMUPDOWN *Data = (NMUPDOWN *) lParam;
  //  //  if(Data->hdr.idFrom == IDC_MOVE_GROUP) {
  //  //    CAlphIO::AlphInfo::group Tmp = CurrentInfo.Groups[CurrentGroup];
  //  //    if(Data->iDelta > 0) {
  //  //      if(CurrentGroup + 1 < CurrentInfo.Groups.size()) {
  //  //        CurrentInfo.Groups[CurrentGroup] = CurrentInfo.Groups[CurrentGroup + 1];
  //  //        CurrentInfo.Groups[CurrentGroup + 1] = Tmp;
  //  //        ++CurrentGroup;
  //  //      }
  //  //    }
  //  //    else {
  //  //      if(CurrentGroup > 0) {
  //  //        CurrentInfo.Groups[CurrentGroup] = CurrentInfo.Groups[CurrentGroup - 1];
  //  //        CurrentInfo.Groups[CurrentGroup - 1] = Tmp;
  //  //        --CurrentGroup;
  //  //      }
  //  //    }
  //  //    ShowGroups();
  //  //  }
  //  //  if(Data->hdr.idFrom == IDC_MOVE_CHAR) {
  //  //    if(CurrentInfo.Groups[CurrentGroup].Characters.size() < 1) {
  //  //      return TRUE;
  //  //      break;
  //  //    }
  //  //    CAlphIO::AlphInfo::character Tmp = CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar];
  //  //    if(Data->iDelta > 0) {
  //  //      if(CurrentChar + 1 < CurrentInfo.Groups[CurrentGroup].Characters.size()) {
  //  //        CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar] = CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar + 1];
  //  //        CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar + 1] = Tmp;
  //  //        ++CurrentChar;
  //  //      }
  //  //    }
  //  //    else {
  //  //      if(CurrentChar > 0) {
  //  //        CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar] = CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar - 1];
  //  //        CurrentInfo.Groups[CurrentGroup].Characters[CurrentChar - 1] = Tmp;
  //  //        --CurrentChar;
  //  //      }
  //  //    }
  //  //    ShowGroupChars();
  //  //  }
  //  //  return TRUE;
  //  //  break; // End UDN_DELTAPOS
  //  }
  //}
  case WM_COMMAND:
    
    if(HIWORD(wParam)==BN_CLICKED || HIWORD(wParam)==LBN_SELCHANGE) {
      if(LOWORD(wParam) != 0 && m_hPropertySheet != 0 && m_hwnd != 0) {
          PropSheet_Changed(m_hPropertySheet, m_hwnd); // enables the 'Apply' button
          // Behaviour isn't *perfect* since it activates the Apply button even if you, say,
          // click 'new' alphabet then click Cancel when asked for a name.
      }
    }
    switch (LOWORD(wParam)) {
    //case (IDC_DISPLAY):
    //  if(HIWORD(wParam) == EN_CHANGE) {
    //    HWND Control = GetDlgItem(Window, IDC_DISPLAY);
    //    LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1;        // +1 to allow for terminator
    //    TCHAR *Buffer = new TCHAR[BufferLength];
    //    SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM) Buffer);
    //    string ItemName;
    //    SendMessage(GetDlgItem(Window, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) Buffer);
    //    delete[]Buffer;
    //  }
    //  break;
    case (IDC_ALPHABETS):
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        HWND ListBox = GetDlgItem(m_hwnd, IDC_ALPHABETS);
        LRESULT CurrentItem = SendMessage(ListBox, LB_GETCURSEL, 0, 0);
        LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, CurrentItem, 0);
        m_CurrentAlphabet = AlphabetList[CurrentIndex];
 /*       CurrentInfo = m_pDasherInterface->GetInfo(m_CurrentAlphabet);
        if(CurrentInfo.Mutable) {
          EnableWindow(GetDlgItem(m_hwnd, IDC_DEL_ALPH), TRUE);
          EnableWindow(GetDlgItem(m_hwnd, IDC_EDIT), TRUE);
        }
        else {
          EnableWindow(GetDlgItem(m_hwnd, IDC_DEL_ALPH), FALSE);
          EnableWindow(GetDlgItem(m_hwnd, IDC_EDIT), FALSE);
        }*/
      }
      return TRUE;
      break;
    case IDC_OR_DEFAULT:
      if(SendMessage(GetDlgItem(m_hwnd, IDC_OR_DEFAULT), BM_GETCHECK, 0, 0) == BST_CHECKED) {
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_LR), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_RL), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_TB), FALSE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_BT), FALSE);

        SendMessage(GetDlgItem(m_hwnd, IDC_OR_LR), BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessage(GetDlgItem(m_hwnd, IDC_OR_RL), BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessage(GetDlgItem(m_hwnd, IDC_OR_TB), BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessage(GetDlgItem(m_hwnd, IDC_OR_BT), BM_SETCHECK, BST_UNCHECKED, 0);
      }
      break;
    case IDC_OR_CUSTOM:
      if(SendMessage(GetDlgItem(m_hwnd, IDC_OR_CUSTOM), BM_GETCHECK, 0, 0 == BST_CHECKED)) {
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_LR), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_RL), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_TB), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_BT), TRUE);

        // TODO: Should really sort this out, but no way to look up default orientation until 
        // changes have been applied
        SendMessage(GetDlgItem(m_hwnd, IDC_OR_LR), BM_SETCHECK, BST_CHECKED, 0);
      }
      break;
     
 /*   case (IDC_GROUPS):
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        HWND GroupList = GetDlgItem(Window, IDC_GROUPS);
        CurrentGroup = SendMessage(GroupList, LB_GETCURSEL, 0, 0);
        CurrentChar = 0;
        ShowGroupChars();
      }
      return TRUE;
      break;*/
    //case (IDC_CHARS):
    //  if(HIWORD(wParam) == LBN_SELCHANGE) {
    //    HWND CharList = GetDlgItem(Window, IDC_CHARS);
    //    CurrentChar = SendMessage(CharList, LB_GETCURSEL, 0, 0);
    //  }
    //  return TRUE;
    //  break;
    //case (IDC_EDIT_CHAR):
    //  if(CurrentInfo.Groups.size() < 1) // if no groups, can't edit character
    //    break;
    //  EditChar = true;
    //  DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_EDITLETTER, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
    //  break;
    //case (IDC_ADD_CHAR):
    //  if(CurrentInfo.Groups.size() < 1) // if no groups, can't add character
    //    break;
    //  DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_ADDLETTERS, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
    //  break;
    //case (IDOK_ADDCHAR):{
    //    string Display = GetControlText(Window, IDC_DISPLAY);
    //    string Text = GetControlText(Window, IDC_TEXT);
    //    int Colour = atoi(GetControlText(Window, IDC_COLOUR).c_str());
    //    CustomCharacter(Display, Text, Colour);
    //    TCHAR Terminator = '\0';
    //    SendMessage(GetDlgItem(Window, IDC_DISPLAY), WM_SETTEXT, 0, (LPARAM) & Terminator);
    //    SetFocus(GetDlgItem(Window, IDC_DISPLAY));
    //    break;
    //  }
    //case (IDOK_CHAR):{
    //    string Display = GetControlText(Window, IDC_DISPLAY);
    //    string Text = GetControlText(Window, IDC_TEXT);
    //    int Colour = atoi(GetControlText(Window, IDC_COLOUR).c_str());
    //    CustomCharacter(Display, Text, Colour);
    //    EndDialog(Window, LOWORD(wParam));
    //    EditChar = false;
    //    break;
    //  }
    //case (IDC_ADD_GROUP):
    //  DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_NEWGROUP, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
    //  break;
    //case (IDOK_NEWGROUP):{
    //    NewGroup(GetControlText(Window, IDC_GROUP_TEXT));
    //    EndDialog(Window, LOWORD(wParam));
    //    break;
    //  }
    //case (IDC_DELGROUP):
    //  if(CurrentInfo.Groups.size() > CurrentGroup)
    //    CurrentInfo.Groups.erase(CurrentInfo.Groups.begin() + CurrentGroup);
    //  ShowGroups();
    //  break;
    //case (IDC_DEL_CHAR):
    //  if(CurrentInfo.Groups.size() > CurrentGroup) {
    //    vector < CAlphIO::AlphInfo::character > &Tmp = CurrentInfo.Groups[CurrentGroup].Characters;
    //    if(Tmp.size() > CurrentChar) {
    //      Tmp.erase(Tmp.begin() + CurrentChar);
    //      CurrentChar = min(Tmp.size() - 1, CurrentChar);
    //      ShowGroupChars();
    //    }
    //  }
    //  break;
    //case (IDOK_CUSTOMIZE):
    //  if(UpdateInfo()) {
    //    m_pDasherInterface->SetInfo(CurrentInfo);
    //    EndDialog(Window, LOWORD(wParam));
    //    PopulateList();
    //  }
    //  break;
    //case (IDC_DEL_ALPH):{
    //    HWND ListBox = GetDlgItem(m_hwnd, IDC_ALPHABETS);
    //    LRESULT CurrentItem = SendMessage(ListBox, LB_GETCURSEL, 0, 0);
    //    LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, CurrentItem, 0);
    //    if(CurrentIndex >= 0 && (unsigned int)CurrentIndex < AlphabetList.size()) {
    //      m_pDasherInterface->DeleteAlphabet(AlphabetList[CurrentIndex]);
    //      PopulateList();
    //    }
    //    break;
    //  }
    //case (IDC_CLONE):
    //  Cloning = true;
    //  // deliberate fall through
    //case (IDC_ADD_ALPH):
    //  DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_NEWALPHABET, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
    //  break;
    //case (IDOK_NEWALPH):{
    //    m_CurrentAlphabet = ""; // Prevent over-writing another alphabet in rare circumstances.
    //    string NewAlph = GetControlText(Window, IDC_NEW_ALPH_STRING);
    //    EndDialog(Window, LOWORD(wParam));
    //    if(Cloning) {
    //      Cloning = false;
    //    }
    //    else {
    //      CAlphIO::AlphInfo Tmp;
    //      CurrentInfo = Tmp;
    //    }
    //    CurrentInfo.AlphID = NewAlph;
    //  }                         // deliberate fall through
    //case (IDC_EDIT):
    //  Editing = true;
    //  DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_CUSTOMALPHABET, Window, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
    //  break;
    //case (IDOK):
    //  if(m_CurrentAlphabet != std::string("")) {
    //    m_pDasherInterface->SetStringParameter(SP_ALPHABET_ID, m_CurrentAlphabet);
    //  }
    //  // deliberate fall through
    //case (IDCANCEL):
    //  {
    //    EndDialog(Window, LOWORD(wParam));
    //    return TRUE;
    //  }
    //case ID_CUSTOM_CANCEL:
    //  PopulateList();           // Need to reget settings for current selection. Cheaty way to do it.
    //  EndDialog(Window, LOWORD(wParam));
    //  return TRUE;
    //  break;
    }
  break;
  default:
   return CPrefsPageBase::WndProc(Window, message, wParam, lParam);
  }

  // TODO: Sort out return values here
  return FALSE;
}
