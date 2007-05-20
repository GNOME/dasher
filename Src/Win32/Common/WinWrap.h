// WinMrap.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

/*
TODO: explain the WindowClass stuff

Win32 Windows are hard to wrap into objects. We must provide an
LRESULT CALLBACK WndProc function to process messages. However,
this may only be a member of an object if it is defined static,
which makes the function unable to alter the object.

A rather inelegant solution is to define a WndProc procedure and
a map of window handles to objects externally. The universal
WndProc function is passed a window handle from which it can
obtain an object pointer and pass on the message. In a bid to
maintain some elegance, these external parts are separated into
the WinWrapMap namespace.

In this software windows that require non-static message
processing functions derive themselves from the CWinWrap abstract
class. They must define the WndProc function.

After generating a window handle they call:
	WinWrapMap::add(m_hwnd, this);
Then (important: and only then) will all messages received by
WinWrapMap::WndProc be passed onto the local WndProc function.
Controls can be subclassed with SetWindowLong. Similarly, if a
custom window class wants anything other than the default window
processor during the CreateWindow() call, it should specify a
temporary processor at first and use 
SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG) WinWrapMap::WndProc);
after adding itself to WinWrapMap.

Dialogs should be created using DialogBoxParam and passing a
pointer to the CWinWrap object in the lparam parameter.

The virtual destructor attempts to destroy the window and remove
it from the map. Obviously you'll need to override this if it is
not what you want.

Moving to a pre-rolled C++ GUI library seems like a very good idea.

Iain Murray August 2002
*/

#ifndef __WinWrap_h__
#define __WinWrap_h__

#include "WinCommon.h"

#include <map>

namespace WinWrapMap {
  LRESULT CALLBACK WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

  // You need to use a slightly different version for property sheets

  LRESULT CALLBACK PSWndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam); 
} 

class CWinWrap:private NoClones {
public:
  virtual ~ CWinWrap();
protected:
  virtual LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
    return 0;
  };

  HWND m_hwnd;
private:
  friend LRESULT CALLBACK WinWrapMap::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
  friend LRESULT CALLBACK WinWrapMap::PSWndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
};

namespace WinWrapMap {
  void add(HWND Win, CWinWrap * WinObject);
  void remove(HWND Win);
  bool Register(TCHAR * WndClassName);
}
#endif                          /* #ifndef __WinWrap_h__ */
