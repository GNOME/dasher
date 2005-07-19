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

#include "WinCommon.h"

#include "Slidebar.h"
#include "../resource.h"

using namespace Dasher;

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

CSlidebar::CSlidebar(HWND ParentWindow, CDasherSettingsInterface *NewDasherInterface, double StartValue, bool Visible, CCanvas *NewDasherCanvas)
:Visible(Visible) {
  m_Height = 200;

  DasherInterface = NewDasherInterface;
  DasherCanvas = NewDasherCanvas;

  // A nicer font than SYSTEM. Generally useful to have around.
  HGDIOBJ hGuiFont;
  hGuiFont = GetStockObject(DEFAULT_GUI_FONT);

  // The static control, which acts as a container
  Tstring MaxSpeed;
  WinLocalisation::GetResourceString(IDS_MAX_SPEED, &MaxSpeed);
  m_hwnd = CreateWindowEx(WS_EX_CONTROLPARENT, TEXT("STATIC"), MaxSpeed.c_str(), WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, ParentWindow, NULL, WinHelper::hInstApp, NULL);
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
  SendMessage(SB_slider, TBM_SETRANGE, FALSE, (LPARAM) MAKELONG(0, 800));
  SendMessage(SB_slider, TBM_SETPOS, TRUE, 201);        // TRUE means can show it now.
  WinWrapMap::add(SB_slider, this);
  SL_WndFunc = (WNDPROC) SetWindowLong(SB_slider, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);

  SetValue(StartValue);
}

int CSlidebar::Resize(int Width, int Base) {
  if(Visible)
    m_Height = m_NormalHeight;
  else
    m_Height = 0;

  m_Width = Width;

  // The whole bar
  MoveWindow(m_hwnd, 0, Base - m_Height, m_Width, m_Height, TRUE);

  // The edit control
  MoveWindow(SB_edit, static_width,     // x
             (m_Height - edit_height) / 2,      // y
             edit_width, edit_height, TRUE);

  // The slider
  int pos2 = static_width + edit_width;
  MoveWindow(SB_slider, pos2, 0, Width - pos2, m_Height, TRUE);

  return m_Height;
}

LRESULT CSlidebar::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  // Allows us to process messages sent to our slide bar
  LRESULT result;

  if(message == WM_KEYDOWN) {
    switch (wParam) {
    case VK_SPACE:
      DasherCanvas->startspace();
      return 0;
      break;
    case VK_F12:
      DasherCanvas->centrecursor();
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
      DasherInterface->SetLongParameter(LP_MAX_BITRATE, NewBitrate);
    break;
  }
  return result;
}

void CSlidebar::SetValue(double NewSlideVal)  {
  SlideVal = NewSlideVal;
  SetSlideBar(SlideVal);
  SetEditBox(SlideVal);
} double CSlidebar::GetValue()  {

  return SlideVal;
}
void CSlidebar::SetSlideBar(double value)  {
  SendMessage(SB_slider, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) (value * 100));
}

void CSlidebar::SetEditBox(double value) {
  //Tstring s = double2string(value); // see below for this travesty
  TCHAR *Buffer = new TCHAR[10];
  _stprintf(Buffer, TEXT("%0.2f"), value);

  SendMessage(SB_edit, WM_SETTEXT, 0, (LPARAM) (LPCSTR) Buffer);

  delete[]Buffer;
}

/*
// Just for humor value, this is what happened when I tried to
// use C++ string manipulation instead of "evil" C buffers. You
// would need all of these libaries too:
//#include <iostream.h>
//#include <sstream>
//#include <string>
//                                                  IAM 08/2002
Tstring CSlidebar::double2string(double number)
{
	typedef std::basic_ostringstream<TCHAR> Tostringstream;
	Tostringstream output;
#ifdef DECENT_COMPILER
	// VC++ 6.0 on my computer doesn't seem to follow the standard.
	// Seems to be very confused about fixed and scientific.
	// Annoying as it is ugly when the display jumps from 1 to 1.01
	output.setf(ios::fixed, ios::floatfield);
	output.precision(2);
#endif
	
	if (output << number) {
		return output.str();
	} else {
		return TEXT("0.00");
	}
}
*/

void CSlidebar::SetVisible(bool Value) {
  Visible = Value;
}
