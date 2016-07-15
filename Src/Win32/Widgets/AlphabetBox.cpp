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
#include "../../DasherCore/DasherTypes.h"

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

CAlphabetBox::CAlphabetBox(HWND Parent, CAppSettings *pAppSettings)
: CPrefsPageBase(Parent, pAppSettings), 
  m_CurrentAlphabet(pAppSettings->GetStringParameter(SP_ALPHABET_ID)) {
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

  m_CurrentAlphabet = m_pAppSettings->GetStringParameter(SP_ALPHABET_ID);

  m_pAppSettings->GetPermittedValues(SP_ALPHABET_ID, AlphabetList);

  int iDefaultIndex(-1);

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
    if(m_pAppSettings->GetBoolParameter(menutable[ii].paramNum) != menutable[ii].bInvert) 
	    SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_CHECKED, 0);
    else  
	    SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_SETCHECK, BST_UNCHECKED, 0);
  }

}


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
    m_pAppSettings->SetStringParameter(SP_ALPHABET_ID, m_CurrentAlphabet); 
  }

  for(int ii = 0; ii<sizeof(menutable)/sizeof(menuentry); ii++)
  {
    m_pAppSettings->SetBoolParameter(menutable[ii].paramNum, 
      (SendMessage(GetDlgItem(m_hwnd, menutable[ii].idcNum), BM_GETCHECK, 0, 0) == BST_CHECKED) != menutable[ii].bInvert);
  }


  // Return false (and notify the user) if something is wrong.
  return TRUE;
}

LRESULT CAlphabetBox::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
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

  case WM_COMMAND:
    if(HIWORD(wParam)==BN_CLICKED || HIWORD(wParam)==LBN_SELCHANGE) {
      if(LOWORD(wParam) != 0 && m_hPropertySheet != 0 && m_hwnd != 0) {
          PropSheet_Changed(m_hPropertySheet, m_hwnd); // enables the 'Apply' button
          // Behaviour isn't *perfect* since it activates the Apply button even if you, say,
          // click 'new' alphabet then click Cancel when asked for a name.
      }
    }
    switch (LOWORD(wParam)) {
    case (IDC_ALPHABETS):
      if(HIWORD(wParam) == LBN_SELCHANGE) {
        HWND ListBox = GetDlgItem(m_hwnd, IDC_ALPHABETS);
        LRESULT CurrentItem = SendMessage(ListBox, LB_GETCURSEL, 0, 0);
        LRESULT CurrentIndex = SendMessage(ListBox, LB_GETITEMDATA, CurrentItem, 0);
		if (CurrentIndex != LB_ERR)
			m_CurrentAlphabet = AlphabetList[CurrentIndex];

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
      if(SendMessage(GetDlgItem(m_hwnd, IDC_OR_CUSTOM), BM_GETCHECK, 0, 0) == BST_CHECKED) {
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_LR), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_RL), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_TB), TRUE);
        EnableWindow(GetDlgItem(m_hwnd, IDC_OR_BT), TRUE);

        // TODO: Should really sort this out, but no way to look up default orientation until 
        // changes have been applied
        SendMessage(GetDlgItem(m_hwnd, IDC_OR_LR), BM_SETCHECK, BST_CHECKED, 0);
      }
      break;
    }
  break;
  default:
   return CPrefsPageBase::WndProc(Window, message, wParam, lParam);
  }

  // TODO: Sort out return values here
  return FALSE;
}
