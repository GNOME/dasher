#include "WinCommon.h"
#include "GameGroup.h"
#include "Edit.h"
#include "../../DasherCore/DasherInterfaceBase.h"
#include "../../DasherCore/GameMessages.h"
#include <string>

// TODO: Make this a notify?

using Dasher::CDasherInterfaceBase;

CGameGroup::CGameGroup(CDasherInterfaceBase *pDasherInterface, CEdit* pEdit) {
  m_pDasherInterface = pDasherInterface;
  m_pEdit = pEdit;
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

LRESULT CGameGroup::OnEditPaint(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  HDC hdcChild = (HDC)wParam;
  SetBkColor(hdcChild,GetSysColor(COLOR_MENUBAR));
  //bHandled=true;
  return (LRESULT)GetSysColorBrush(COLOR_MENUBAR);
}


LRESULT CGameGroup::OnShow(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  
  bVisible = wParam;
  return 0;
}

LRESULT CGameGroup::OnDemoClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{  
  int message = (m_pDemoButton->SendMessage(BM_GETCHECK)==BST_CHECKED)?GAME_MESSAGE_DEMO_ON:GAME_MESSAGE_DEMO_OFF;
  m_pDasherInterface->GameMessageIn(message, NULL);
  return 0;
}

LRESULT CGameGroup::OnNextClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{  
  m_pDasherInterface->GameMessageIn(GAME_MESSAGE_NEXT, NULL);
  return 0;
}


void CGameGroup::Message(int message,const void* messagedata)
{ 
  const std::string* pStr;
  wstring strText;
    
  switch(message) {
  case GAME_MESSAGE_SET_TARGET_STRING:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    m_strTarget = (*pStr);

    WinUTF8::UTF8string_to_wstring(m_strTarget, strText);
    m_pGameTextLabel->SetWindowTextW(strText.c_str());
    break;
  case GAME_MESSAGE_DISPLAY_TEXT:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    WinUTF8::UTF8string_to_wstring(*pStr, strText);
    m_pGameTextLabel->SetWindowTextW(strText.c_str());
    break;
  case GAME_MESSAGE_EDIT:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    m_strOutput += (*pStr);
    UpdateTargetLabel();
    break;
  case GAME_MESSAGE_EDIT_DELETE:
    m_strOutput = m_strOutput.substr( 0, m_strOutput.size() -
							    *reinterpret_cast<const int *>(messagedata));
    UpdateTargetLabel();
    break;
  case GAME_MESSAGE_SET_SCORE:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    WinUTF8::UTF8string_to_wstring(*pStr, strText);
    m_pScoreEdit->SetWindowTextW(strText.c_str());
    break;

  case GAME_MESSAGE_SET_LEVEL:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    WinUTF8::UTF8string_to_wstring(*pStr, strText);
    m_pLevelEdit->SetWindowTextW(strText.c_str());
    break;

  case GAME_MESSAGE_CLEAR_BUFFER:
    m_pEdit->Clear();
    m_strOutput.clear();
    break;
  case GAME_MESSAGE_HELP_MESSAGE:
    pStr = reinterpret_cast<const std::string *>(messagedata);
    WinUTF8::UTF8string_to_wstring(*pStr, strText); 
    ::MessageBox(GetParent(), strText.c_str(), TEXT("Information"), MB_OK);
    break;
  }
}

void CGameGroup::UpdateTargetLabel()
{
}

HWND CGameGroup::Create(HWND hParent) {
  CWindowImpl<CGameGroup>::Create(hParent,0,0,WS_CLIPCHILDREN | WS_CHILD );

  // Children are created by the OnCreate handler

  //PopulateCombo();
  //PopulateSpeed();
  m_iButtonX = max(m_sNextSize.cx,m_sDemoSize.cx);
  m_iButtonY = max(m_sNextSize.cy,m_sDemoSize.cy);
  m_iLabelHeight= m_iButtonY;
  m_iSpacing = m_iButtonX/4;

  m_iHeight = 3*m_iButtonY + 4*m_iSpacing;
  m_iWidth = 2*m_iButtonX + 3*m_iSpacing;
  
  m_pDemoButton->ShowWindow(SW_SHOW);
  m_pNextButton->ShowWindow(SW_SHOW);
  m_pGameTextLabel->ShowWindow(SW_SHOW);
  m_pScoreEdit->ShowWindow(SW_SHOW);
  m_pLevelEdit->ShowWindow(SW_SHOW);
  m_pLevelLabel->ShowWindow(SW_SHOW);
  m_pScoreLabel->ShowWindow(SW_SHOW);
  

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
  m_pDemoButton->Create(WC_BUTTON, m_hWnd,0,TEXT("Demo"),
                        WS_CHILD | BS_AUTOCHECKBOX | BS_PUSHLIKE,
                        0,ID_DEMOBUTTON);
  m_pDemoButton->SetFont((HFONT)hGuiFont);
  HDC hDemoDC(m_pDemoButton->GetDC());
  SelectObject(hDemoDC, hGuiFont);
  GetTextExtentPoint32(hDemoDC,TEXT("Demo"),4,&m_sDemoSize);
  m_pDemoButton->ReleaseDC(hDemoDC);
  m_sDemoSize.cx += 5;
  m_sDemoSize.cy += 5;
  
  m_pNextButton = new CWindow;
  m_pNextButton->Create(WC_BUTTON, m_hWnd,0,TEXT("Next"),
                        WS_CHILD,
                        0,ID_NEXTBUTTON);
  m_pNextButton->SetFont((HFONT)hGuiFont);
  HDC hNextDC(m_pNextButton->GetDC());
  SelectObject(hNextDC, hGuiFont);
  GetTextExtentPoint32(hNextDC,TEXT("Demo"),4,&m_sNextSize);
  m_pNextButton->ReleaseDC(hNextDC);
  m_sNextSize.cx += 5;
  m_sNextSize.cy += 5;

  m_pGameTextLabel = new CWindow;
  m_pGameTextLabel->Create(WC_EDIT, m_hWnd,0,TEXT("Next"),
                           ES_NOHIDESEL | WS_CHILD | ES_READONLY | WS_VISIBLE,
                           WS_EX_CLIENTEDGE, ID_GAMELABEL);
  m_pGameTextLabel->SetFont((HFONT)hGuiFont);
  HDC hLabelDC(m_pGameTextLabel->GetDC());
  SelectObject(hLabelDC, hGuiFont);
  SIZE sLabel;
  GetTextExtentPoint32(hLabelDC,TEXT("'C^9A"),5,&sLabel);
  m_pGameTextLabel->ReleaseDC(hLabelDC);
  m_iLabelHeight = sLabel.cy+5;

  m_pScoreEdit = new CWindow;
  m_pScoreEdit->Create(WC_EDIT, m_hWnd,0,TEXT(""),
                       ES_NOHIDESEL | WS_CHILD | ES_READONLY | WS_VISIBLE,
                       WS_EX_CLIENTEDGE, ID_SCOREEDIT);
  m_pScoreEdit->SetFont((HFONT)hGuiFont);
  HDC hScoreDC(m_pScoreEdit->GetDC());
  SelectObject(hScoreDC, hGuiFont);
  SIZE sScore;
  GetTextExtentPoint32(hScoreDC,TEXT("'C^9A"),5,&sScore);
  m_pScoreEdit->ReleaseDC(hScoreDC);
  //m_iBoxHeight = sScore.cy+5;

  m_pLevelEdit = new CWindow;
  m_pLevelEdit->Create(WC_EDIT, m_hWnd,0,TEXT(""),
                       ES_NOHIDESEL | WS_CHILD | ES_READONLY | WS_VISIBLE,
                       WS_EX_CLIENTEDGE, ID_LEVELEDIT);
  m_pLevelEdit->SetFont((HFONT)hGuiFont);
  HDC hLevelDC(m_pLevelEdit->GetDC());
  SelectObject(hLevelDC, hGuiFont);
  SIZE sLevel;
  GetTextExtentPoint32(hLevelDC,TEXT("'C^9A"),5,&sLevel);
  m_pLevelEdit->ReleaseDC(hLevelDC);

  m_pLevelLabel = new CWindow;
  m_pLevelLabel->Create(WC_STATIC, m_hWnd,0,TEXT("Level"),
                       ES_NOHIDESEL | WS_CHILD | ES_READONLY | WS_VISIBLE,
                       WS_EX_CLIENTEDGE);
  m_pLevelLabel->SetFont((HFONT)hGuiFont);
  hLevelDC =  m_pLevelLabel->GetDC();
  SelectObject(hLevelDC, hGuiFont);
  GetTextExtentPoint32(hLevelDC,TEXT("'C^9A"),5,&sLevel);
  m_pLevelLabel->ReleaseDC(hLevelDC);

  m_pScoreLabel = new CWindow;
  m_pScoreLabel->Create(WC_STATIC, m_hWnd,0,TEXT("Score"),
                       ES_NOHIDESEL | WS_CHILD | ES_READONLY | WS_VISIBLE,
                       WS_EX_CLIENTEDGE);
  m_pScoreLabel->SetFont((HFONT)hGuiFont);
  hScoreDC = m_pScoreLabel->GetDC();
  SelectObject(hScoreDC, hGuiFont);
  m_pScoreLabel->ReleaseDC(hScoreDC);

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
 
 int iBoxX = iFullWidth-m_iWidth;
 
 m_pGameTextLabel->MoveWindow(0,0,iBoxX,m_iLabelHeight);
 
  int iCurrentY = m_iSpacing;
  m_pNextButton->MoveWindow(iBoxX+m_iSpacing,iCurrentY,m_iButtonX,m_iButtonY);
  m_pDemoButton->MoveWindow(iBoxX+m_iButtonX+2*m_iSpacing,iCurrentY,m_iButtonX,m_iButtonY);
  
  iCurrentY += m_iSpacing + m_iButtonY;
  m_pLevelLabel->MoveWindow(iBoxX+m_iSpacing,iCurrentY,m_iButtonX,m_iButtonY);
  m_pLevelEdit->MoveWindow(iBoxX+m_iButtonX+2*m_iSpacing,iCurrentY,m_iButtonX, m_iButtonY);

  iCurrentY += m_iSpacing + m_iButtonY;
  m_pScoreLabel->MoveWindow(iBoxX+m_iSpacing,iCurrentY,m_iButtonX,m_iButtonY);
  m_pScoreEdit->MoveWindow(iBoxX + m_iButtonX+2*m_iSpacing,iCurrentY,m_iButtonX, m_iButtonY);
}

void CGameGroup::LayoutChildrenUpdate() {
  // For now don't do anything special here
  LayoutChildrenInitial();
}

void CGameGroup::PopulateSpeed() {
  int iValue(m_pDasherInterface->GetLongParameter(LP_MAX_BITRATE));
/*
  TCHAR *Buffer = new TCHAR[10];
  _stprintf(Buffer, TEXT("%0.2f"), iValue / 100.0);
  SendMessage(m_hScoreEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);
  delete[]Buffer;*/
}

void CGameGroup::UpdateSpeed(int iPos, int iDelta) {
/*  int iValue(iPos + iDelta);

  if(iValue > 800)
    iValue = 800;

  if(iValue < 1)
    iValue = 1;

  TCHAR *Buffer = new TCHAR[10];
  _stprintf(Buffer, TEXT("%0.2f"), iValue / 100.0);
  SendMessage(m_hScoreEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);
  delete[]Buffer;

  m_pDasherInterface->SetLongParameter(LP_MAX_BITRATE, iValue);
  */
}

void CGameGroup::SetEditFont(std::string Name, long Size) {
  using namespace WinUTF8;
   using namespace WinLocalisation;
#ifndef _WIN32_WCE
  Tstring FontName;
  UTF8string_to_wstring(Name, FontName);

  if(Size == 0)
    Size = 14;

  HFONT Font;
  if(Name == "")
    Font = GetCodePageFont(GetUserCodePage(), -Size);
  else
    Font = CreateFont(-Size, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, FontName.c_str());    // DEFAULT_CHARSET => font made just from Size and FontName

  m_pGameTextLabel->SendMessage(WM_SETFONT, (WPARAM) Font, true);
#else
  // not implemented
#pragma message ( "CEdit::SetFot not implemented on WinCE")
  //DASHER_ASSERT(0);
#endif
}