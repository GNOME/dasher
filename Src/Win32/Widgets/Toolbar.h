// Toolbar.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Toolbar_h__
#define __Toolbar_h__

#include "../../Common/NoClones.h"
#include "../../DasherCore/DasherInterfaceBase.h"

class CToolbar : private NoClones {
public:
  CToolbar(HWND hParent, bool bVisible);

  // Tell the rebar to fill its space
  void Resize();

  // Show/hide the toolbar
  void ShowToolbar(bool bValue);

  // get the toolbar height (used to lay out the rest of the window)
  int GetHeight();

private:

  // Create the toolbar window
  void CreateToolbar();

  // Destroy the toolbar window (used when it's not visible - shouldn't we just hide it?)
  void DestroyToolbar();

  Dasher::CDasherInterfaceBase *m_pDasher;
  HWND m_hwnd;
  HWND m_hParent;
  HWND m_hRebar;
};

#endif  /* #ifdef __Toolbar_h__ */
