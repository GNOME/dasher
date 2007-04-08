// Menu.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Menu_h__
#define __Menu_h__

#include <map>


class CMenu {
public:
  void SetStatus(UINT Command, bool Grayed, bool Checked);
  bool GetCheck(UINT Command);

  void SortOut(HMENU Menu);
private:
  struct MenuOptions {
    bool Grayed;
    bool Checked;
  };
    std::map < UINT, MenuOptions > m_Items;
};

#endif  /* #ifndef __Menu_h__ */
