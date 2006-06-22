// Slidebar.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

/*
	IAM 08/2002
	TODO YUCK. Sorry I do appreciate that this is a *total* mess.
*/

#include "..\Common\WinCommon.h"

#include "Slidebar.h"
#include "../resource.h"

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

void CSlidebar::CreateEdit() {
  // The edit box
  SB_edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | ES_READONLY, 0, 0, 0, CW_USEDEFAULT, m_hwnd, NULL, WinHelper::hInstApp, NULL);
  // ES_NUMBER does not allow decimal point
  // ES_READONLY (at least for now) so don't have to handle typed input)
  //SendMessage(SB_edit, WM_SETFONT, (WPARAM) 4, (LPARAM) 0);
  SendMessage(SB_edit, EM_LIMITTEXT, (WPARAM) 4, (LPARAM) 0);
  edit_width = 50;

  // Get information about the system font
  HDC hdc = GetDC(SB_edit);
  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);
  TEXTMETRIC tmGui;
  GetTextMetrics(hdc, &tmGui);
  SendMessage(SB_edit, WM_SETFONT, (WPARAM) hGuiFont, true);
  DeleteObject(hGuiFont);
  ReleaseDC(SB_edit, hdc);

  //calculate the new height for the edit control
  edit_height = tmGui.tmHeight + (GetSystemMetrics(SM_CYEDGE) * 2);
  //edit_width = tmGui.tmMaxCharWidth*4; // This is too wide, but only way to do safely.
  edit_width = tmGui.tmAveCharWidth * 5;        // Stuff it, going to be unsafe
  // Try putting in 4 W's and you'll see why.
  // Someone /might/ have really wide numbers in their system font.

  // This shouldn't be here. TODO: rejig.
  //m_NormalHeight = (tmGui.tmHeight + GetSystemMetrics(SM_CYEDGE))*2;
  m_NormalHeight = tmGui.tmHeight * 2 - tmGui.tmInternalLeading;

  // Create a spin control for the edit box

  m_hUpDown = CreateWindowEx(WS_EX_CLIENTEDGE, UPDOWN_CLASS, TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | ES_READONLY, 0, 0, 0, CW_USEDEFAULT, m_hwnd, NULL, WinHelper::hInstApp, NULL);
//WinWrapMap::add(SB_slider, this);
  SendMessage(m_hUpDown, UDM_SETBUDDY, (WPARAM)SB_edit, 0);
  SendMessage(m_hUpDown, UDM_SETRANGE, 0, (LPARAM) MAKELONG(1, 8));

  m_hCombo = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("COMBOBOX"), TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | ES_READONLY, 0, 0, 128, 128, m_hwnd, NULL, WinHelper::hInstApp, NULL);
  WinWrapMap::add(m_hCombo, this);
  PopulateCombo();
  return;
}

CSlidebar::CSlidebar(HWND ParentWindow, CDasherInterface *NewDasherInterface, double StartValue, CCanvas *NewDasherCanvas) {
  

  m_iControlHeight = MulDiv(10, HIWORD(GetDialogBaseUnits()), 8) + 1;
  m_Height = m_iControlHeight + 2;

  m_pDasherInterface = NewDasherInterface;
  m_pDasherCanvas = NewDasherCanvas;

  // A nicer font than SYSTEM. Generally useful to have around.
  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);

  m_hwnd = ParentWindow;

  std::wstring strSpeedLabel(L"Speed:");
  std::wstring strAlphabetLabel(L"Alphabet:");

  m_hSpeedLabel = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), strSpeedLabel.c_str(), 
      SS_SUNKEN | SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, ParentWindow, NULL, WinHelper::hInstApp, NULL);

  m_hAlphabetLabel = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), strAlphabetLabel.c_str(), 
      SS_SUNKEN | SS_CENTER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, ParentWindow, NULL, WinHelper::hInstApp, NULL);

  SendMessage(m_hSpeedLabel, WM_SETFONT, (WPARAM) hGuiFont, true);
  SendMessage(m_hAlphabetLabel, WM_SETFONT, (WPARAM) hGuiFont, true);

  SIZE sSize;

  HDC hSpeedDC(GetDC(m_hSpeedLabel));
  SelectObject(hSpeedDC,hGuiFont);
  GetTextExtentPoint32(hSpeedDC, strSpeedLabel.c_str(), strSpeedLabel.size(), &sSize);
  MoveWindow(m_hSpeedLabel, 0, 0, sSize.cx + 4, m_iControlHeight, false);
  
  HDC hAlphabetDC(GetDC(m_hAlphabetLabel));
  SelectObject(hAlphabetDC,hGuiFont);
  GetTextExtentPoint32(hAlphabetDC, strAlphabetLabel.c_str(), strAlphabetLabel.size(), &sSize);
  MoveWindow(m_hAlphabetLabel, 0, 0, sSize.cx + 4, m_iControlHeight, false);
  


  // The static control, which acts as a container
  /*Tstring MaxSpeed;
  WinLocalisation::GetResourceString(IDS_MAX_SPEED, &MaxSpeed);
  m_hwnd = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), MaxSpeed.c_str(), WS_CHILD | WS_TABSTOP | SS_OWNERDRAW, 0, 0, 0, 0, ParentWindow, NULL, WinHelper::hInstApp, NULL);

  HDC hDC(GetDC(m_hwnd));
  SelectObject(hDC,hGuiFont);

  SIZE sSize;
  GetTextExtentPoint32(hDC, MaxSpeed.c_str(), MaxSpeed.size(), &sSize);

  m_iLabelWidth = sSize.cx;
  m_iLabelHeight = sSize.cy;*/

  m_iBorderTop = 8;

  //WinWrapMap::add(m_hwnd, this);
  //SB_WndFunc = (WNDPROC) SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);
  //SendMessage(m_hwnd, WM_SETFONT, (WPARAM) hGuiFont, true);
  //InvalidateRect(m_hwnd, NULL, FALSE);
  //ShowWindow(m_hwnd, SW_SHOW);
  //UpdateWindow(m_hwnd);
  //static_width = 90;            // TODO: Shouldn't do this.

  // The box next to the slider that displays the bitrate
  CreateEdit();

  SendMessage(m_hCombo, WM_SETFONT, (WPARAM) hGuiFont, true);

  // The Slider
  WinHelper::InitCommonControlLib();
  //SB_slider = CreateWindowEx(0L, TRACKBAR_CLASS, TEXT(""), WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | WS_TABSTOP, 0, 0, 0, 0, m_hwnd, NULL, WinHelper::hInstApp, NULL);

  //// Slider format: bit rate range of 0-8 with control precision of 0.01
  //SendMessage(SB_slider, TBM_SETPAGESIZE, 0L, 20);      // PgUp and PgDown change bitrate by reasonable amount
  //SendMessage(SB_slider, TBM_SETTICFREQ, 100, 0L);
  //SendMessage(SB_slider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(10, 800));
  //SendMessage(SB_slider, TBM_SETPOS, TRUE, 201);        // TRUE means can show it now.
  //WinWrapMap::add(SB_slider, this);
  //SL_WndFunc = (WNDPROC) SetWindowLong(SB_slider, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);

  SetValue(StartValue);
}

//void CSlidebar::Redraw(LPDRAWITEMSTRUCT pDrawItem) {
//  Tstring MaxSpeed;
//  WinLocalisation::GetResourceString(IDS_MAX_SPEED, &MaxSpeed);
//
//  HGDIOBJ hGuiFont;
//  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);
//  SelectObject(pDrawItem->hDC,hGuiFont); // FIXME - I think we're supposed to store the old value
//
//  RECT sRect;
//
//  sRect.left = 0;
//  sRect.top = 0;
//  sRect.right = m_Width;
//  sRect.bottom = m_Height;
//
//  FillRect(pDrawItem->hDC, &sRect, GetSysColorBrush(COLOR_3DFACE));
//
//  sRect.left = 4;
//  sRect.top = 4 + m_iLabelHeight / 2;
//  sRect.right = m_Width - 4;
//  sRect.bottom = m_Height - 4;
//
//  DrawEdge(pDrawItem->hDC, &sRect, EDGE_ETCHED, BF_RECT);
//
//  sRect.left = 11;
//  sRect.top = 4;
//  sRect.right = 11 + m_iLabelWidth;
//  sRect.bottom = 4 + m_iLabelHeight;
//
//  DrawText(pDrawItem->hDC,MaxSpeed.c_str(),MaxSpeed.size(),&sRect,DT_LEFT);
//};

int CSlidebar::Resize(int Width, int Base) {
  if(m_pDasherInterface->GetBoolParameter(BP_SHOW_SLIDER))
   // m_Height = m_NormalHeight;
   m_Height = m_iControlHeight + 2;
  else
   m_Height = 0;

  m_Width = Width;

  // The whole bar
  //MoveWindow(m_hwnd, 0, Base - m_Height, m_Width, m_Height, TRUE);

//m_iBorderTop = Base - m_Height;

  int iLRPos(0);

  RECT sRect;
  
  GetWindowRect(m_hSpeedLabel, &sRect);
  MoveWindow(m_hSpeedLabel, iLRPos, Base - m_Height + 2, sRect.right-sRect.left, sRect.bottom-sRect.top, TRUE);
  iLRPos += sRect.right-sRect.left;
  
  // The edit control
  MoveWindow(SB_edit, iLRPos, Base - m_Height + 2, edit_width, edit_height, TRUE);
  GetWindowRect(SB_edit, &sRect);
  iLRPos += sRect.right-sRect.left;
  
  MoveWindow(m_hUpDown, iLRPos, Base - m_Height + 2, 16, edit_height, TRUE);
  GetWindowRect(m_hUpDown, &sRect);
  iLRPos += sRect.right-sRect.left;
  
  GetWindowRect(m_hAlphabetLabel, &sRect);
  MoveWindow(m_hAlphabetLabel, iLRPos, Base - m_Height + 2, sRect.right-sRect.left, sRect.bottom-sRect.top, TRUE);
  iLRPos += sRect.right-sRect.left;

  GetWindowRect(m_hCombo, &sRect);
  MoveWindow(m_hCombo, iLRPos, Base - m_Height + 2, m_Width - iLRPos , sRect.bottom - sRect.top, TRUE);

  sRect.left = 0;
  sRect.right = m_Width;
  sRect.top = 0;
  sRect.bottom = m_Height;

  InvalidateRect(m_hwnd, &sRect, false);


  return m_Height;
}

LRESULT CSlidebar::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  // Allows us to process messages sent to our slide bar
  LRESULT result;

  if(message == WM_KEYDOWN) {
    switch (wParam) {
    case VK_SPACE:
      m_pDasherInterface->KeyDown(GetTickCount(), 0);
      return 0;
      break;
    case VK_F12:
      m_pDasherCanvas->centrecursor();
      return 0;
      break;
    }
  }
  else if(message == WM_COMMAND) {
    switch(HIWORD(wParam)) {
      case CBN_SELCHANGE:
        AlphabetSelection();
        break;
    }
  }

  if(Window == m_hwnd) {
    result = CallWindowProc(SB_WndFunc, Window, message, wParam, lParam);
  }
  else {
    result = CallWindowProc(SL_WndFunc, Window, message, wParam, lParam);
  }

  return result;
}

void CSlidebar::SetValue(double NewSlideVal) {
  SlideVal = NewSlideVal;
  SetEditBox(SlideVal);
  PopulateCombo();
  return;
} 


double CSlidebar::GetValue() {
  return SlideVal;
}



//void CSlidebar::SetSlideBar(double value) {
//  //SendMessage(SB_slider, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) round(value * 100));
//  return;
//}

void CSlidebar::SetEditBox(double value) {
  //Tstring s = double2string(value); // see below for this travesty
  TCHAR *Buffer = new TCHAR[10];
  _stprintf(Buffer, TEXT("%0.2f"), value);

  SendMessage(SB_edit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);

  delete[]Buffer;
}

void CSlidebar::PopulateCombo() {
  int iCount(SendMessage(m_hCombo, CB_GETCOUNT, 0, 0));

  for(int i(0); i < iCount; ++i)
    SendMessage(m_hCombo, CB_DELETESTRING, 0, 0);

  std::wstring strEntry;
  
  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_ID), strEntry);
  SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_1), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_2), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_3), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  WinUTF8::UTF8string_to_wstring(m_pDasherInterface->GetStringParameter(SP_ALPHABET_4), strEntry);
  if(strEntry.size() > 0)
    SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)strEntry.c_str());

  SendMessage(m_hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)L"More Alphabets...");
  
  SendMessage(m_hCombo, CB_SETCURSEL, 0, 0);
}

void CSlidebar::AlphabetSelection() {
  int iIndex(SendMessage(m_hCombo, CB_GETCURSEL, 0, 0));
  int iLength(SendMessage(m_hCombo, CB_GETLBTEXTLEN, iIndex, 0));

  TCHAR *szSelection = new TCHAR[iLength + 1];
  SendMessage(m_hCombo, CB_GETLBTEXT, iIndex, (LPARAM)szSelection);

  std::string strNewValue;
  WinUTF8::wstring_to_UTF8string(szSelection, strNewValue);

  m_pDasherInterface->SetStringParameter(SP_ALPHABET_ID, strNewValue);

  delete[] szSelection;
}

void CSlidebar::HandleParameterChange(int iParameter) {
  switch(iParameter) {
    case SP_ALPHABET_ID:
      PopulateCombo();
  }
}