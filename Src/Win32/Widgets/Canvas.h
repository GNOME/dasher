// Canvas.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Canvas_h__
#define __Canvas_h__

#include "../TabletPC/CursorInRange.h"
#include "../../DasherCore/DasherComponent.h"
#include "../../DasherCore/DasherTypes.h"

namespace Dasher
{
	class CDasherMouseInput;
	class CSocketInput;
}

class CDasherInterface;
class CEdit;
class CScreen;

#define WM_DASHER_TIMER WM_USER + 128   // FIXME - shouldn't define this twice


class CCanvas:	
	public ATL::CWindowImpl<CCanvas>, 
	public Dasher::CDasherComponent 
{
public:

	static ATL::CWndClassInfo& GetWndClassInfo() 
	{ 
		static ATL::CWndClassInfo wc = \
		{ \
			{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc, \
			  0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL, _T("CANVAS"), NULL }, \
			NULL, NULL, MAKEINTRESOURCE(IDC_CROSS), TRUE, 0, _T("") \
		}; \
	return wc;
	}

	BEGIN_MSG_MAP( CCanvas )
	    MESSAGE_HANDLER(WM_PAINT, OnPaint)
	    MESSAGE_HANDLER(WM_CREATE, OnCreate)
	    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
//	    MESSAGE_HANDLER(WM_DASHER_TIMER, OnTimer)
	    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	    MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
	    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	    MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
	    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	    MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	    MESSAGE_HANDLER(WM_CURSOR_IN_RANGE, OnCursorInRange)
	    MESSAGE_HANDLER(WM_CURSOR_OUT_OF_RANGE, OnCursorOutOfRange)
	    MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
	    MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
	    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	    MESSAGE_HANDLER(WM_SIZE, OnSize)

	END_MSG_MAP()

	CCanvas(CDasherInterface * DI);
	~CCanvas();

	HWND Create(HWND hParent);

	void DoFrame();

	LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCursorInRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCursorOutOfRange(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClk(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT OnTimer(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyUp(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void Move(int x, int y, int Width, int Height);
	void Paint();



  HWND getwindow() {
    return m_hWnd;
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
 
  void centrecursor();
  void StartStop();

  void Pause() {
    running = 0;
  }
  bool Running() {
    return running;
  }
  void SetScreenInterface(CDasherInterface * dasherinterface);
  
  int OnTimer();

  bool GetCanvasSize(int& pTop, int& pLeft, int& pBottom, int& pRight);

  void HandleEvent(Dasher::CEvent *pEvent);

private:

  HDC m_hdc;
  int keycoords[18], buttonnum, yscaling;
  bool forward, backward, select;
  CScreen *m_pScreen;
  CDasherInterface * m_pDasherInterface;

  // Input devices:

  Dasher::CDasherMouseInput * m_pMouseInput;
  Dasher::CSocketInput * m_pSocketInput;

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

  // Enables tablet pc events
  CCursorInRange m_CursorInRange;

};

#endif /* #ifndef __Canvas_h__ */
