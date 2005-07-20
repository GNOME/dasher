// Slidebar.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

// Interface for a bar with a slider, an edit box and a slider.
// Used for explicit speed control.

// Usage:
// TODO.

#ifndef __Slidebar_h__
#define __Slidebar_h__

#include "../../DasherCore/Win32/DasherInterface.h"
#include "Canvas.h"

class CSlidebar:public CWinWrap {
public:
  CSlidebar(HWND ParentWindow, CDasherInterface * DasherInterface, double StartValue = 2.51, bool Visible = true, CCanvas * NewDasherCanvas = 0);

  // Base is the desired position of the bottom of the slidebar.
  // The height of the slide bar is returned.
  int Resize(int Width, int Base);
  int GetHeight() {
    return m_Height;
  } void SetValue(double NewSlideVal);
  double GetValue();

  void SetVisible(bool Value);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
  WNDPROC sliderWndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  double SlideVal;

  // Message handler stuff
  WNDPROC SB_WndFunc;
  WNDPROC SL_WndFunc;

  // Used to keep edit box and slide bar in synch.
  void SetEditBox(double value);
  void SetSlideBar(double value);

  // The child windows
  void CreateEdit();
  HWND m_hwnd;;                 // Container window for the bar
  HWND SB_static;               // The bar label
  HWND SB_edit;                 // To show (or perhaps later type in the speed)
  HWND SB_slider;

  // These used to help window positioning
  int static_width;
  int static_height;            // TODO
  int edit_width;
  int edit_height;

  int m_Height;
  int m_Width;

  bool Visible;
  int m_NormalHeight;

  // The model to poke
  CDasherInterface * m_pDasherInterface;
  CCanvas *m_pDasherCanvas;
};

#endif  /* #ifndef __Slidebar_h__ */
