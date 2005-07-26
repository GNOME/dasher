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
#include "../../DasherCore/Win32/DasherInterface.h"

class CToolbar:private NoClones {
public:
  CToolbar(HWND ParentWindow, CDasherInterface *DI);
  int Resize();
  void ShowToolbar(bool bValue);
private:
  CDasherInterface *m_pDasher;
  HWND m_hwnd, ParentWindow;

  void CreateToolbar();
  void DestroyToolbar();
};

#endif  /* #ifdef __Toolbar_h__ */
