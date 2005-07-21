// Splitter.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Splitter_h__
#define __Splitter_h__

#define MY_LAYOUT WM_USER+2

#include "../../DasherCore/Win32/DasherInterface.h"

class CSplitterOwner {
public:
  virtual void Layout() = 0;
};

class CSplitter:public CWinWrap {
public:
  CSplitter(HWND Parent, CDasherInterface *DI, int Pos, CSplitterOwner * NewOwner);
  void Move(int Pos, int Width);
  int GetHeight() {
    return GetSystemMetrics(SM_CYSIZEFRAME);
  } 
  int GetPos() {
    return m_Pos;
  }

protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  CDasherInterface *m_pDasherInterface;
  CSplitterOwner *Owner;
  int m_Pos;
  HWND m_hwnd, Parent;
  Tstring CreateMyClass();
  enum SplitStatuses { None, Sizing } SplitStatus;
};

#endif  /* #ifndef __Splitter_h__ */
