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
	void setmouseposdist(int y) {m_pScreen->SetMousePosDist(y);mouseposdist=y;}
	int getmouseposdist() {return m_pScreen->GetMousePosDist();}
	void onedimensional(bool value) {oned=value;}
	void setforward(bool value) {forward=value;}
	void setbackward(bool value) {backward=value;}
	void setselect(bool value) {select=value;}
	bool getforward() {return forward;}
	bool getbackward() {return backward;}
	bool getselect() {return select;}
	void startspace();
	void centrecursor();
	void StartStop();
	void DrawOutlines(bool Value) {m_pScreen->DrawOutlines(Value);}
	void Pause() {running=0;}
	bool Running() {return running;}
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HWND Parent;
	HDC m_hdc;
	int keycoords[18],buttonnum,yscaling,mouseposdist;
	bool forward,backward,select;
	CScreen* m_pScreen;
	Dasher::CDasherWidgetInterface* m_DasherWidgetInterface;
	Dasher::CDasherAppInterface* m_DasherAppInterface;
	CEdit* m_DasherEditBox;
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
	bool oned;
	DWORD mousepostime;
	DWORD previoustime;
	RECT coords;
};


#endif /* #ifndef __Canvas_h__ */
