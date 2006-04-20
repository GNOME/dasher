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

  return;
}

CSlidebar::CSlidebar(HWND ParentWindow, CDasherInterface *NewDasherInterface, double StartValue, CCanvas *NewDasherCanvas) {
  m_Height = 200;

  m_pDasherInterface = NewDasherInterface;
  m_pDasherCanvas = NewDasherCanvas;

  // A nicer font than SYSTEM. Generally useful to have around.
  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);

  // The static control, which acts as a container
  Tstring MaxSpeed;
  WinLocalisation::GetResourceString(IDS_MAX_SPEED, &MaxSpeed);
  m_hwnd = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), MaxSpeed.c_str(), WS_CHILD | WS_TABSTOP | SS_OWNERDRAW, 0, 0, 0, 0, ParentWindow, NULL, WinHelper::hInstApp, NULL);

  HDC hDC(GetDC(m_hwnd));
  SelectObject(hDC,hGuiFont);

  SIZE sSize;
  GetTextExtentPoint32(hDC, MaxSpeed.c_str(), MaxSpeed.size(), &sSize);

  m_iLabelWidth = sSize.cx;
  m_iLabelHeight = sSize.cy;

  m_iBorderTop = sSize.cy + 8;

  WinWrapMap::add(m_hwnd, this);
  SB_WndFunc = (WNDPROC) SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);
  SendMessage(m_hwnd, WM_SETFONT, (WPARAM) hGuiFont, true);
  InvalidateRect(m_hwnd, NULL, FALSE);
  ShowWindow(m_hwnd, SW_SHOW);
  UpdateWindow(m_hwnd);
  static_width = 90;            // TODO: Shouldn't do this.

  // The box next to the slider that displays the bitrate
  CreateEdit();

  // The Slider
  WinHelper::InitCommonControlLib();
  SB_slider = CreateWindowEx(0L, TRACKBAR_CLASS, TEXT(""), WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | WS_TABSTOP, 0, 0, 0, 0, m_hwnd, NULL, WinHelper::hInstApp, NULL);

  // Slider format: bit rate range of 0-8 with control precision of 0.01
  SendMessage(SB_slider, TBM_SETPAGESIZE, 0L, 20);      // PgUp and PgDown change bitrate by reasonable amount
  SendMessage(SB_slider, TBM_SETTICFREQ, 100, 0L);
  SendMessage(SB_slider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(10, 800));
  SendMessage(SB_slider, TBM_SETPOS, TRUE, 201);        // TRUE means can show it now.
  WinWrapMap::add(SB_slider, this);
  SL_WndFunc = (WNDPROC) SetWindowLong(SB_slider, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);

  SetValue(StartValue);
}

void CSlidebar::Redraw(LPDRAWITEMSTRUCT pDrawItem) {
  Tstring MaxSpeed;
  WinLocalisation::GetResourceString(IDS_MAX_SPEED, &MaxSpeed);

  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);
  SelectObject(pDrawItem->hDC,hGuiFont); // FIXME - I think we're supposed to store the old value

  RECT sRect;

  sRect.left = 0;
  sRect.top = 0;
  sRect.right = m_Width;
  sRect.bottom = m_Height;

  FillRect(pDrawItem->hDC, &sRect, GetSysColorBrush(COLOR_3DFACE));

  sRect.left = 4;
  sRect.top = 4 + m_iLabelHeight / 2;
  sRect.right = m_Width - 4;
  sRect.bottom = m_Height - 4;

  DrawEdge(pDrawItem->hDC, &sRect, EDGE_ETCHED, BF_RECT);

  sRect.left = 11;
  sRect.top = 4;
  sRect.right = 11 + m_iLabelWidth;
  sRect.bottom = 4 + m_iLabelHeight;

  DrawText(pDrawItem->hDC,MaxSpeed.c_str(),MaxSpeed.size(),&sRect,DT_LEFT);
};

int CSlidebar::Resize(int Width, int Base) {
  if(m_pDasherInterface->GetBoolParameter(BP_SHOW_SLIDER))
   // m_Height = m_NormalHeight;
   m_Height = edit_height + m_iBorderTop + 11;
  else
    m_Height = 0;

  m_Width = Width;

  // The whole bar
  MoveWindow(m_hwnd, 0, Base - m_Height, m_Width, m_Height, TRUE);

  // The edit control
  MoveWindow(SB_edit, 11,     // x
             m_iBorderTop,      // y
             edit_width, edit_height, TRUE);

  // The slider
  int pos2 = 15 + edit_width;
  MoveWindow(SB_slider, pos2, m_iBorderTop, Width - pos2 - 11, edit_height, TRUE);

  RECT sRect;

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

  if(Window == m_hwnd) {
    result = CallWindowProc(SB_WndFunc, Window, message, wParam, lParam);
  }
  else {
    result = CallWindowProc(SL_WndFunc, Window, message, wParam, lParam);
  }

  switch (message) {
  case WM_HSCROLL:
    double NewBitrate;
    if((LOWORD(wParam) == SB_THUMBPOSITION) | (LOWORD(wParam) == SB_THUMBTRACK)) {
      // Some messages give the new postion
      NewBitrate = HIWORD(wParam) / 100.0;
    }
    else {
      // Otherwise we have to ask for it
      long Pos = SendMessage(SB_slider, TBM_GETPOS, 0, 0);
      NewBitrate = Pos / 100.0;
    }
    SetEditBox(NewBitrate);
    if(LOWORD(wParam) == TB_ENDTRACK)
      m_pDasherInterface->SetLongParameter(LP_MAX_BITRATE, long(round(NewBitrate*100.0)));
    break;
  }
  return result;
}

void CSlidebar::SetValue(double NewSlideVal) {  SlideVal = NewSlideVal;  SetSlideBar(SlideVal);  SetEditBox(SlideVal);} double CSlidebar::GetValue() {
  return SlideVal;}
void CSlidebar::SetSlideBar(double value) {  SendMessage(SB_slider, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) round(value * 100));}

void CSlidebar::SetEditBox(double value) {
  //Tstring s = double2string(value); // see below for this travesty
  TCHAR *Buffer = new TCHAR[10];
  _stprintf(Buffer, TEXT("%0.2f"), value);

  SendMessage(SB_edit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);

  delete[]Buffer;
}
