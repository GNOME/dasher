// Canvas.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __Canvas_h__
#define __Canvas_h__

#include "../WinWrap.h"
#include "../../DasherCore/DasherWidgetInterface.h"
#include "../../DasherCore/DasherAppInterface.h"
#include "Screen.h"

struct ThreadParams
{
  HWND hw;
  UINT cb;
};

class CCanvas : public CWinWrap
{
public:
	CCanvas(HWND Parent, Dasher::CDasherWidgetInterface* WI, Dasher::CDasherAppInterface* AI);
	~CCanvas();
	void Move(int x, int y, int Width, int Height);
	void Paint();
	HWND getwindow() {return m_hwnd;}
	void StartOnLeftClick(bool Value) {startonleft = Value;}
	void StartOnSpace(bool Value) {startonspace = Value;}
	void KeyControl(bool Value) {keycontrol = Value;}
	void WindowPause(bool Value) {windowpause = Value;}
	void setkeycoords(int coords[8]) {for (int i=0; i<8; i++) {keycoords[i]=coords[i];};}
	int* getkeycoords() {return keycoords;}
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HWND Parent;
	int keycoords[8];
	CScreen* Screen;
	Dasher::CDasherWidgetInterface* m_DasherWidgetInterface;
	Dasher::CDasherAppInterface* m_DasherAppInterface;
	HANDLE hThreadl;
	DWORD dwThreadID;
	unsigned int imousex,imousey;
	UINT MY_TIMER;
	bool running;
	bool startonleft;
	bool startonspace;
	bool keycontrol;
	bool windowpause;
	RECT coords;
	ThreadParams *tp;
};


#endif /* #ifndef __Canvas_h__ */
