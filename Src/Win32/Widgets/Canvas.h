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
#include "Edit.h"

struct ThreadParams
{
  HWND hw;
  UINT cb;
};

class CCanvas : public CWinWrap
{
public:
	CCanvas(HWND Parent, Dasher::CDasherWidgetInterface* WI, Dasher::CDasherAppInterface* AI, CEdit* EB);
	~CCanvas();
	void Move(int x, int y, int Width, int Height);
	void Paint();
	HWND getwindow() {return m_hwnd;}
	void StartOnLeftClick(bool Value) {startonleft = Value;}
	void StartOnSpace(bool Value) {startonspace = Value;}
	void KeyControl(bool Value) {keycontrol = Value;}
	void WindowPause(bool Value) {windowpause = Value;}
	void MousePosStart(bool Value);
	void SpeakOnStop(bool Value) {speakonstop = Value;}
	void setkeycoords(int coords[18]) {for (int i=0; i<18; i++) {keycoords[i]=coords[i];};}
	int* getkeycoords() {return keycoords;}
	void setyscale(int y) {yscaling=y;}
	int getyscale() {return yscaling;}
	void setforward(bool value) {forward=value;}
	void setbackward(bool value) {backward=value;}
	void setselect(bool value) {select=value;}
	bool getforward() {return forward;}
	bool getbackward() {return backward;}
	bool getselect() {return select;}
	void startspace();
	void centrecursor();
	void StartStop();
	void DrawOutlines(bool Value) {Screen->DrawOutlines(Value);}
	void Pause() {running=0;}
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HWND Parent;
	int keycoords[18],buttonnum,yscaling;
	bool forward,backward,select;
	CScreen* Screen;
	Dasher::CDasherWidgetInterface* m_DasherWidgetInterface;
	Dasher::CDasherAppInterface* m_DasherAppInterface;
	CEdit* m_DasherEditBox;
	HANDLE hThreadl;
	DWORD dwThreadID;
	unsigned int imousex,imousey;
	UINT MY_TIMER;
	bool running;
	bool startonleft;
	bool startonspace;
	bool keycontrol;
	bool windowpause;
	bool mouseposstart;
	bool firstwindow;
	bool secondwindow;
	bool speakonstop;
	DWORD mousepostime;
	RECT coords;
	ThreadParams *tp;
};


#endif /* #ifndef __Canvas_h__ */
