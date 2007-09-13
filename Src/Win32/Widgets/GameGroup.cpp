#include "GameGroup.h"
#include "WinCommon.h"

#include <string>

// TODO: Make this a notify?

CGameGroup::CGameGroup(CDasherInterfaceBase *pDasherInterface) {
  m_pDasherInterface = pDasherInterface;
  m_pDemoButton=NULL;
  bVisible=false;
}

// TODO: ATL has more sophisticated handlers for conrol and notify messages - consider using them instead
LRESULT CGameGroup::OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  switch(HIWORD(wParam)) {
    case CBN_SELCHANGE:
//      SelectAlphabet();
      break;
    default:
      bHandled = false;
      break;
  }

  return 0;
}

LRESULT CGameGroup::OnNotify(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  NMHDR *pNotify((LPNMHDR)lParam);

  switch(pNotify->code) {
    case UDN_DELTAPOS:
      UpdateSpeed(((LPNMUPDOWN) lParam)->iPos, ((LPNMUPDOWN) lParam)->iDelta);
      break;
    default:
      bHandled = false;
      break;
  }

  return 0;
}

LRESULT CGameGroup::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  LayoutChildrenUpdate();
  return 0;
}

LRESULT CGameGroup::OnCreate(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  CreateChildren();
  LayoutChildrenInitial();
  return 0;
}

LRESULT CGameGroup::OnShow(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  
  bVisible = wParam;
  return 0;
}

void CGameGroup::Message(int message,const void* messagedata)
{ 
  /*const std::string* pStr;
  std::string strText ="<span background=\"purple\" foreground=\"white\">";
  switch(message) {
  case GAME_MESSAGE_SET_TARGET_STRING:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    *(pPrivate->pstrTarget) = (*pStr);
    gtk_misc_set_alignment(GTK_MISC(pPrivate->pGameInfoLabel), 0.0, 0.0);
    gtk_label_set_justify(pPrivate->pGameInfoLabel, GTK_JUSTIFY_CENTER);
    gtk_label_set_text(pPrivate->pGameInfoLabel, pPrivate->pstrTarget->c_str());
    break;
  case GAME_MESSAGE_DISPLAY_TEXT:
    gtk_misc_set_alignment(GTK_MISC(pPrivate->pGameInfoLabel), 0.5, 0.0);
    gtk_label_set_use_markup(pPrivate->pGameInfoLabel, true);
    gtk_label_set_justify(pPrivate->pGameInfoLabel, GTK_JUSTIFY_CENTER);
    //    std::string strText ="<span background=\"purple\">";
    strText+=(reinterpret_cast<const char*>(messagedata));
    strText+="</span>";
    gtk_label_set_markup(pPrivate->pGameInfoLabel, strText.c_str());
    break;
  case GAME_MESSAGE_EDIT:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    *(pPrivate->pstrOutput) += (*pStr);
    game_mode_helper_update_target_label(pSelf);
    break;
  case GAME_MESSAGE_EDIT_DELETE:
    *(pPrivate->pstrOutput) = pPrivate->pstrOutput->substr( 0, pPrivate->pstrOutput->size() -
							    *reinterpret_cast<const int *>(messagedata));
    game_mode_helper_update_target_label(pSelf);
    break;
  case GAME_MESSAGE_SET_SCORE:
    gtk_entry_set_text(pPrivate->pScore, reinterpret_cast<const char*>(messagedata));
    break;
  case GAME_MESSAGE_SET_LEVEL:
    gtk_entry_set_text(pPrivate->pLevel, reinterpret_cast<const char*>(messagedata));
    break;
  case GAME_MESSAGE_CLEAR_BUFFER:
    dasher_editor_internal_cleartext(pPrivate->pEditor);
    pPrivate->pstrOutput->clear();
    break;
  case GAME_MESSAGE_HELP_MESSAGE:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    game_mode_helper_dialog_box(pSelf, pStr->c_str());
    break;
  }*/
}

HWND CGameGroup::Create(HWND hParent) {
  CWindowImpl<CGameGroup>::Create(hParent,0,0,WS_CLIPCHILDREN | WS_CHILD );

  // Children are created by the OnCreate handler
  //PopulateCombo();
  //PopulateSpeed();
  m_iLabelHeight=25;
  m_iHeight = 100;
  m_iWidth=100;
  m_pDemoButton->ShowWindow(SW_SHOW);
  m_pNextButton->ShowWindow(SW_SHOW);

  //SendMessage(m_pDemoButton->m_hWnd, BM_SETCHECK,(WPARAM)BST_CHECKED, 0 );
  return m_hWnd;
}

void CGameGroup::CreateChildren() {
  // First create the two static labels.
  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);

  std::wstring strScoreLabel(L"Speed:");
  std::wstring strLevelLabel(L"Alphabet:");
/*
  // TODO: Wrap windows here in CWindow classes.
#ifndef _WIN32_WCE
  // TODO: Is this really needed?
  m_hScoreLabel = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), strScoreLabel.c_str(), 
      SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, NULL, WinHelper::hInstApp, NULL);
#else
  m_hScoreLabel = CreateWindowEx(0, TEXT("STATIC"), strScoreLabel.c_str(), 
      SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, NULL, WinHelper::hInstApp, NULL);
#endif

#ifndef _WIN32_WCE
  m_hLevelLabel = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), strLevelLabel.c_str(), 
      SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, NULL, WinHelper::hInstApp, NULL);
#else
  m_hAlphabetLabel = CreateWindowEx(0, TEXT("STATIC"), strAlphabetLabel.c_str(), 
      SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, NULL, WinHelper::hInstApp, NULL);
#endif*/
  
  //SendMessage(m_hSpeedLabel, WM_SETFONT, (WPARAM) hGuiFont, true);
  //SendMessage(m_hAlphabetLabel, WM_SETFONT, (WPARAM) hGuiFont, true);
  m_pDemoButton = new CWindow;
  m_pDemoButton->Create(WC_BUTTON, m_hWnd,0,TEXT("Demo"),WS_CHILD | BS_AUTOCHECKBOX | BS_PUSHLIKE);
  m_pDemoButton->SetFont((HFONT)hGuiFont);
  HDC hDemoDC(m_pDemoButton->GetDC());
  SelectObject(hDemoDC, hGuiFont);
  GetTextExtentPoint32(hDemoDC,TEXT("Demo"),4,&m_sDemoSize);
  m_pDemoButton->ReleaseDC(hDemoDC);
  m_sDemoSize.cx += 5;
  m_sDemoSize.cy += 5;

  m_pNextButton = new CWindow;
  m_pNextButton->Create(WC_BUTTON, m_hWnd,0,TEXT("Next"),WS_CHILD);
  m_pNextButton->SetFont((HFONT)hGuiFont);
  HDC hNextDC(m_pNextButton->GetDC());
  SelectObject(hNextDC, hGuiFont);
  GetTextExtentPoint32(hNextDC,TEXT("Demo"),4,&m_sNextSize);
  m_pNextButton->ReleaseDC(hNextDC);
  m_sNextSize.cx += 5;
  m_sNextSize.cy += 5;
/*  SIZE sSize;

  HDC hSpeedDC(::GetDC(m_hSpeedLabel));
  SelectObject(hSpeedDC, hGuiFont);
  GetTextExtentPoint32(hSpeedDC, strSpeedLabel.c_str(), strSpeedLabel.size(), &sSize);
  ::MoveWindow(m_hSpeedLabel, 0, 0, sSize.cx + 4, sSize.cy, false);
  ::ReleaseDC(m_hSpeedLabel, hSpeedDC);

  HDC hAlphabetDC(::GetDC(m_hAlphabetLabel));
  SelectObject(hAlphabetDC, hGuiFont);
  GetTextExtentPoint32(hAlphabetDC, strAlphabetLabel.c_str(), strAlphabetLabel.size(), &sSize);
  ::MoveWindow(m_hAlphabetLabel, 0, 0, sSize.cx + 4, sSize.cy, false);
  ::ReleaseDC(m_hAlphabetLabel, hAlphabetDC);

  // Next the edit box and up/down control
  m_hScoreEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_GROUP, 0, 0, 0, CW_USEDEFAULT, m_hWnd, NULL, WinHelper::hInstApp, NULL);
  SendMessage(m_hEdit, EM_LIMITTEXT, (WPARAM) 4, (LPARAM) 0);

  HDC hdc = ::GetDC(m_hScoreEdit);
  TEXTMETRIC tmGui;
  GetTextMetrics(hdc, &tmGui);
  SendMessage(m_hEdit, WM_SETFONT, (WPARAM) hGuiFont, true);
  ::ReleaseDC(m_hEdit, hdc);

  m_iEditHeight = tmGui.tmHeight + (GetSystemMetrics(SM_CYEDGE) * 2);
  m_iEditWidth = tmGui.tmAveCharWidth * 7;
  ::MoveWindow(m_hEdit, 0, 0, m_iEditWidth, m_iEditHeight, false);
 
  m_hUpDown = CreateWindowEx(WS_EX_CLIENTEDGE, UPDOWN_CLASS, TEXT(""), UDS_ALIGNRIGHT | WS_CHILD  | WS_TABSTOP |WS_VISIBLE |  WS_GROUP, 0, 0, 16, 16, m_hWnd, NULL, WinHelper::hInstApp, NULL);
  SendMessage(m_hUpDown, UDM_SETRANGE, 0, (LPARAM) MAKELONG(800, 1));

  // And finally the combo box
  m_hCombo = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("COMBOBOX"), TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VISIBLE |WS_GROUP | ES_READONLY, 0, 0, 128, 128, m_hWnd, NULL, WinHelper::hInstApp, NULL);
  SendMessage(m_hCombo, WM_SETFONT, (WPARAM) hGuiFont, true);

  //DeleteObject(hGuiFont);
*/
  // Set the height to what is finally required. The extra pixel is needed to get everything to line up nicely
  //MoveWindow(0, 0, 0, m_iEditHeight+1, false);
}

void CGameGroup::LayoutChildrenInitial() {
  RECT rc;
  GetWindowRect(&rc);

  // And how much is needed for the various bits and pieces
  RECT sRect;
/*  
  ::GetWindowRect(m_hScoreLabel, &sRect);
  int iScoreLabelWidth(sRect.right - sRect.left);  
  int iScoreLabelHeight(sRect.bottom - sRect.top);
  
  ::GetWindowRect(m_hLevelLabel, &sRect);
  int iLevelLabelWidth(sRect.right - sRect.left);  
  int iLevelLabelHeight(sRect.bottom - sRect.top);
  
  ::GetWindowRect(m_hLevelEdit, &sRect);
  int iLevelEditWidth(sRect.right - sRect.left);
  int iLevelEditHeight(sRect.bottom - sRect.top);
  
  ::GetWindowRect(m_hScoreEdit, &sRect);
  int iScoreEditWidth(sRect.right - sRect.left);  
  int iScoreEditHeight(sRect.bottom - sRect.top);
*/
  // Finally, do the layout
  // TODO: This isn't necessary - the only thing which is going to change is the width of the combo
  /*
  int iPosition(0);

  ::MoveWindow(m_hSpeedLabel, iPosition, (iEditHeight - iSpeedLabelHeight) / 2, iSpeedLabelWidth, iSpeedLabelHeight, TRUE);
  iPosition += iSpeedLabelWidth + 2;

  ::MoveWindow(m_hEdit, iPosition, 0, m_iEditWidth, iEditHeight, TRUE);
  ::SendMessage(m_hUpDown, UDM_SETBUDDY, (WPARAM)m_hEdit, 0);
  iPosition += m_iEditWidth + 2;

  ::MoveWindow(m_hAlphabetLabel, iPosition, (iEditHeight - iAlphabetLabelHeight) / 2, iAlphabetLabelWidth, iAlphabetLabelHeight, TRUE);
  iPosition += iAlphabetLabelWidth + 2;

  ::MoveWindow(m_hCombo, iPosition, 0, rc.right - rc.left - iPosition, iComboHeight, TRUE);
  */
 GetWindowRect(&sRect);
 int iFullWidth = sRect.right-sRect.left;
 int iFullHeight = sRect.bottom - sRect.top;
 HRGN hWindowRgn = CreateRectRgn(0,0,iFullWidth,iFullHeight);
 HRGN hRgn=CreateRectRgn(0,m_iLabelHeight,
                         iFullWidth - m_iWidth,iFullHeight);

 HRGN hResult=CreateRectRgn(0,0,0,0);
 CombineRgn(hResult,hWindowRgn,hRgn,RGN_XOR);
 SetWindowRgn(hResult,true);
 int iButtonX = max(m_sNextSize.cx,m_sDemoSize.cx);
int iButtonY = max(m_sNextSize.cy,m_sDemoSize.cy);
 
 m_pNextButton->MoveWindow(iFullWidth - m_iWidth+4,4,iButtonX,iButtonY);
 m_pDemoButton->MoveWindow(iFullWidth - m_iWidth+4,3*iButtonY/2,iButtonX,iButtonY);
 
}

void CGameGroup::LayoutChildrenUpdate() {
  // For now don't do anything special here
  LayoutChildrenInitial();
}

void CGameGroup::PopulateSpeed() {
  int iValue(m_pDasherInterface->GetLongParameter(LP_MAX_BITRATE));

  TCHAR *Buffer = new TCHAR[10];
  _stprintf(Buffer, TEXT("%0.2f"), iValue / 100.0);
  SendMessage(m_hScoreEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);
  delete[]Buffer;
}

void CGameGroup::UpdateSpeed(int iPos, int iDelta) {
  int iValue(iPos + iDelta);

  if(iValue > 800)
    iValue = 800;

  if(iValue < 1)
    iValue = 1;

  TCHAR *Buffer = new TCHAR[10];
  _stprintf(Buffer, TEXT("%0.2f"), iValue / 100.0);
  SendMessage(m_hScoreEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);
  delete[]Buffer;

  m_pDasherInterface->SetLongParameter(LP_MAX_BITRATE, iValue);
}
