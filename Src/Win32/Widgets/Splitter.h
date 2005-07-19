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

class CSplitterOwner {
public:
  virtual void Layout() = 0;
};

class CSplitter:public CWinWrap {
public:
  CSplitter(HWND Parent, int Pos, CSplitterOwner * NewOwner, bool Visible = true);
  void Move(int Pos, int Width);
  int GetHeight() {
    return m_Height;
  } int GetPos() {
    return m_Pos;
  }
  void SetVisible(bool Value);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  bool Visible;
  CSplitterOwner *Owner;
  int m_Height, m_Pos;
  HWND m_hwnd, Parent;
  Tstring CreateMyClass();
  enum SplitStatuses { None, Sizing } SplitStatus;
};

#endif  /* #ifndef __Splitter_h__ */
