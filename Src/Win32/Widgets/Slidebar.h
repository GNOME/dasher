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

#include "Canvas.h"
#include "../../DasherCore/DasherInterfaceBase.h"
#include "StatusControl.h"

class CSlidebar {
public:
	CSlidebar(HWND ParentWindow, Dasher::CDasherInterfaceBase * DasherInterface);

  // Called when the parent window gets resized, makes the rebar position itself correctly
  void Resize();

  // Return the height, used to layout the rest of the window
  int GetHeight();

private:
  HWND m_hRebar; 
  CStatusControl *m_pStatusControl;
};

#endif  /* #ifndef __Slidebar_h__ */
