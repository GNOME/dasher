// Canvas.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __Canvas_h__
#define __Canvas_h__

#include "../../DasherCore/DasherWidgetInterface.h"
#include "../../DasherCore/DasherAppInterface.h"
#include "../../DasherCore/DasherTypes.h"

#include "../DasherMouseInput.h"

#include "Screen.h"

class CEdit;

class CCanvas : public CWinWrap
{
public:
	CCanvas(HWND Parent, Dasher::CDasherWidgetInterface* WI, Dasher::CDasherAppInterface* AI, CEdit* EB);
	~CCanvas();
	void Move(int x, int y, int Width, int Height);
	void Paint();

	void OnDestroy();

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
	void setmouseposdist(int y) {m_iMousePosDist=y;}
	int getmouseposdist() {return m_iMousePosDist;}
	void setuniform(int y) {uniform=y;}
	int getuniform() {return uniform;}
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
	void SetScreenInterface(Dasher::CDasherWidgetInterface* widgetinterface){m_pScreen->SetInterface(widgetinterface);}	
	int	OnTimer();

protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:

	HWND Parent;
	HDC m_hdc;
	int keycoords[18],buttonnum,yscaling;
	int m_iMousePosDist;
	int uniform;
	bool forward,backward,select;
	CScreen* m_pScreen;
	Dasher::CDasherWidgetInterface* m_DasherWidgetInterface;
	Dasher::CDasherAppInterface* m_DasherAppInterface;

	// Mouse Input Abstraction

	Dasher::CDasherMouseInput* m_pInput;

	CEdit* m_DasherEditBox;
	Dasher::screenint imousex,imousey;
	UINT MY_TIMER;
	bool running;
	bool startonleft;

	bool startonspace;

	bool keycontrol;

	bool windowpause;

	bool m_MousePosStart;

	bool firstwindow;

	bool secondwindow;

	bool speakonstop;

	bool oned;

	bool direction;

	bool lbuttonheld;

	bool inturbo;

	bool enabletime;

	DWORD startturbo;

	DWORD endturbo;

	DWORD lastlbutton;

	DWORD mousepostime;

	DWORD previoustime;

	RECT coords;
};


#endif /* #ifndef __Canvas_h__ */
