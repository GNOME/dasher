#include "WinCommon.h"
#include "GameGroup.h"
#include "Edit.h"
#include "../../DasherCore/DasherInterfaceBase.h"
#include "../../DasherCore/GameMessages.h"
#include <string>
#include <sstream>

// TODO: Make this a notify
//using namespace Dasher::GameMode;


CGameGroup::CGameGroup(Dasher::CDasherInterfaceBase *pDasherInterface, CEdit* pEdit) {
  m_pDasherInterface = pDasherInterface;
  m_pEdit = pEdit;
  m_pDemoButton=NULL;
  bVisible=false;
}

HWND CGameGroup::Create(HWND hParent) {
  CWindowImpl<CGameGroup>::Create(hParent,0,0,WS_CLIPCHILDREN | WS_CHILD );
  // During the Create call, the WM_CREATE message is sent. This is 
  // handled by OnCreate, where the child windows are created, and the initial
  // layout performed.
  
  m_iButtonX = max(m_sNextSize.cx,m_sDemoSize.cx);
  m_iButtonY = max(m_sNextSize.cy,m_sDemoSize.cy);
  m_iLabelHeight= m_iButtonY;
  m_iSpacing = m_iButtonX/4;

  m_iHeight = 3*m_iButtonY + 4*m_iSpacing;
  m_iWidth = 2*m_iButtonX + 3*m_iSpacing;
  
  // Make all our children visible.
  m_pDemoButton->ShowWindow(SW_SHOW);
  m_pNextButton->ShowWindow(SW_SHOW);
  m_pGameTextLabel->ShowWindow(SW_SHOW);
  m_pScoreEdit->ShowWindow(SW_SHOW);
  m_pLevelEdit->ShowWindow(SW_SHOW);
  m_pLevelLabel->ShowWindow(SW_SHOW);
  m_pScoreLabel->ShowWindow(SW_SHOW);

  // There is no this->ShowWindow() call because that is left up to
  // the parent DasherWindow.

  return m_hWnd;
}


LRESULT CGameGroup::OnCreate(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  // Handles WM_CREATE
  CreateChildren();
  LayoutChildrenInitial();
  return 0;
}


void CGameGroup::CreateChildren() {
  // First get the Default GUI font.
  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);

  //------------------------
  // Create the Demo button.
  //------------------------
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
  
  //------------------------
  // Create the Next button.
  //------------------------
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

  //---------------------------------------------------
  // Create the label which appears above the edit box.
  //---------------------------------------------------
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

  //----------------------------------------------
  // Create a control to show the current score...
  //----------------------------------------------
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

  //-----------------------
  // ...and a label for it.
  //-----------------------
  m_pScoreLabel = new CWindow;
  m_pScoreLabel->Create(WC_STATIC, m_hWnd,0,TEXT("Score"),
                       ES_NOHIDESEL | WS_CHILD | ES_READONLY | WS_VISIBLE);
  m_pScoreLabel->SetFont((HFONT)hGuiFont);
  hScoreDC = m_pScoreLabel->GetDC();
  SelectObject(hScoreDC, hGuiFont);
  m_pScoreLabel->ReleaseDC(hScoreDC);

  //----------------------------------------------
  // Create a control to show the current level...
  //----------------------------------------------
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

  //----------------------
  // ...and a label for it
  //----------------------
  m_pLevelLabel = new CWindow;
  m_pLevelLabel->Create(WC_STATIC, m_hWnd,0,TEXT("Level"),
                       ES_NOHIDESEL | WS_CHILD | ES_READONLY | WS_VISIBLE);
  m_pLevelLabel->SetFont((HFONT)hGuiFont);
  hLevelDC =  m_pLevelLabel->GetDC();
  SelectObject(hLevelDC, hGuiFont);
  GetTextExtentPoint32(hLevelDC,TEXT("'C^9A"),5,&sLevel);
  m_pLevelLabel->ReleaseDC(hLevelDC);

}

void CGameGroup::LayoutChildrenInitial() {
  RECT sRect;
  GetWindowRect(&sRect);

  int iFullWidth = sRect.right-sRect.left;
  int iFullHeight = sRect.bottom - sRect.top;

  HRGN hWindowRgn = CreateRectRgn(0,0,iFullWidth,iFullHeight);
  HRGN hRgn=CreateRectRgn(0,m_iLabelHeight,
                          iFullWidth - m_iWidth,iFullHeight);
  HRGN hResult=CreateRectRgn(0,0,0,0);
  CombineRgn(hResult,hWindowRgn,hRgn,RGN_XOR);
  SetWindowRgn(hResult,true);
  DeleteObject(hWindowRgn);
  DeleteObject(hRgn);
 
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


LRESULT CGameGroup::OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  // Handles WM_SIZE
  LayoutChildrenUpdate();
  return 0;
}

LRESULT CGameGroup::OnEditPaint(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  // Handles WM_CTLCOLOREDIT and WM_CTLCOLORSTATIC
  HDC hdcChild = (HDC)wParam;
  SetBkColor(hdcChild,GetSysColor(COLOR_MENUBAR));
  return (LRESULT)GetSysColorBrush(COLOR_MENUBAR);
}


LRESULT CGameGroup::OnShow(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  // Handles WM_SHOWWINDOW
  bVisible = wParam;
  return 0;
}

LRESULT CGameGroup::OnDemoClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{  
  int message = (m_pDemoButton->SendMessage(BM_GETCHECK)==BST_CHECKED)?
	  (Dasher::GameMode::GAME_MESSAGE_DEMO_ON):(Dasher::GameMode::GAME_MESSAGE_DEMO_OFF);
  m_pDasherInterface->GameMessageIn(message, NULL);
  return 0;
}

LRESULT CGameGroup::OnNextClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{  
	m_pDasherInterface->GameMessageIn(Dasher::GameMode::GAME_MESSAGE_NEXT, NULL);
  return 0;
}


void CGameGroup::Message(int message,const void* messagedata)
{ 
  using namespace Dasher::GameMode;
  const std::string* pStr;
  wstring strText;
  //std::stringstream s;
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
    /*
    s << m_pDasherInterface->GetFramerate()<<std::endl;
    WinUTF8::UTF8string_to_wstring(s.str(), strText);
    m_pScoreEdit->SetWindowTextW(strText.c_str());
    */
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

void CGameGroup::LayoutChildrenUpdate() {
  // For now don't do anything special here
  LayoutChildrenInitial();
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