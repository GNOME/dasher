// Canvas.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Canvas_h__
#define __Canvas_h__

#include "../../DasherCore/Win32/DasherInterface.h"
#include "../../DasherCore/DasherTypes.h"

#include "../DasherMouseInput.h"

#include "Screen.h"

class CEdit;

class CCanvas:public CWinWrap {
public:
  CCanvas(HWND Parent, CDasherInterface * DI);
  ~CCanvas();
  void Move(int x, int y, int Width, int Height);
  void Paint();

  void OnDestroy();

  HWND getwindow() {
    return m_hwnd;
  } 
  void StartOnLeftClick(bool Value) {
    startonleft = Value;
  }


  void MousePosStart(bool Value);
  void setkeycoords(int coords[18]) {
    for(int i = 0; i < 18; i++) {
      keycoords[i] = coords[i];
    };
  }
  int *getkeycoords() {
    return keycoords;
  }

  void setforward(bool value) {
    forward = value;
  }
  void setbackward(bool value) {
    backward = value;
  }
  void setselect(bool value) {
    select = value;
  }
  bool getforward() {
    return forward;
  }
  bool getbackward() {
    return backward;
  }
  bool getselect() {
    return select;
  }
  void startspace();
  void centrecursor();
  void StartStop();

  void Pause() {
    running = 0;
  }
  bool Running() {
    return running;
  }
  void SetScreenInterface(CDasherInterface * dasherinterface) {
    m_pScreen->SetInterface(dasherinterface);
  }
  int OnTimer();

  bool GetCanvasSize(int* pTop, int* pLeft, int* pBottom, int* pRight);

protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:

  HWND Parent;
  HDC m_hdc;
  int keycoords[18], buttonnum, yscaling;
  bool forward, backward, select;
  CScreen *m_pScreen;
  CDasherInterface * m_pDasherInterface;

  // Mouse Input Abstraction

  Dasher::CDasherMouseInput * m_pInput;

  Dasher::screenint imousex, imousey;
  UINT MY_TIMER;
  bool startonleft;
  bool direction;
  bool running;
  bool firstwindow;
  bool secondwindow;
  bool lbuttonheld;
  bool inturbo;
  bool enabletime;
  DWORD startturbo;
  DWORD endturbo;
  DWORD lastlbutton;
  DWORD mousepostime;
  DWORD previoustime;
  RECT coords;

  // Indicates that a button is depressed, so we dont stop on idle
  bool m_bButtonDown;
  // Ticks as last event, for stop on idle
  DWORD m_dwTicksLastEvent;

};

#endif /* #ifndef __Canvas_h__ */
