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
  CToolbar(HWND ParentWindow, CDasherInterface *DI, bool Visible = true);

  enum action { SetFalse, SetTrue, Toggle, Query };
  bool SetVisible(action Cmd);
  bool ShowText(action Cmd);
  bool SetLargeIcons(action Cmd);
  int Resize();
private:
  CDasherInterface *m_pDasher;
  bool Visible;
  HWND m_hwnd, ParentWindow;

  bool DoAction(bool * Property, action Cmd);
  void CreateToolbar();
  void DestroyToolbar();
};

#endif  /* #ifdef __Toolbar_h__ */
